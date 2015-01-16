#include "sysutil.h"

int tcp_client(unsigned short port){
	int sockt;	
	if((sockt=socket(PF_INET,SOCK_STREAM,0))<0) ERR_EXIT("tcp_client");
		
	if(port>0){
		// 设置地址重复利用
		int on=1;
		int ret=setsockopt(sockt,SOL_SOCKET,SO_REUSEADDR,(const char*)&on,sizeof(on));
		if(ret<0) ERR_EXIT("setsockopt");	
		
		char ip[16]={0};
		getlocalip(ip);		// 获取本地ip
		
		struct sockaddr_in localaddr;
		memset(&localaddr,0,sizeof(localaddr));
		localaddr.sin_family=AF_INET;
		localaddr.sin_port=htons(port);
		localaddr.sin_addr.s_addr=inet_addr(ip);
		
		// 绑定
		if(bind(sockt,(struct sockaddr*)&localaddr,sizeof(localaddr))<0){
			ERR_EXIT("tcp_client->bind");		
		}
	}
	
	return sockt;
}

/**
* tcp_server - 启动tcp服务器
* @host:服务器IP地址或者服务器主机名
* @port:服务器端口
* 成功恢复监听套接字
*/
int tcp_server(const char *host,unsigned short port){
	int listenfd;
	if((listenfd=socket(PF_INET,SOCK_STREAM,0))<0) ERR_EXIT("tcp_server");
		
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	
	if(host!=NULL){
		// 判断是否是合法地址
		if(inet_aton(host,&servaddr.sin_addr)==0){
			struct hostent *hp;
			if((hp=gethostbyname(host))==NULL) ERR_EXIT("gethostbyname");
			servaddr.sin_addr=*(struct in_addr*)hp->h_addr;	
		}	
	}else{
		servaddr.sin_addr.s_addr=htonl(INADDR_ANY);	
	}
	servaddr.sin_port=htons(port);
	
	// 设置地址重复利用
	int on=1;
	int ret=setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const char*)&on,sizeof(on));
	if(ret<0) ERR_EXIT("gethostbyname");
	
	// 绑定
	ret=bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));	
	if(ret<0)	ERR_EXIT("tcp_server->bind");
		
	// 监听
	ret=listen(listenfd,SOMAXCONN);
	if(ret<0)ERR_EXIT("listen");
		
	return listenfd;
}

int getlocalip(char *ip){
	char host[100]={0};
	if(gethostname(host,sizeof(host))<0) return -1;
		
	struct hostent *hp;
	if((hp=gethostbyname(host))==NULL) return -1;
		
	strcpy(ip,inet_ntoa(*(struct in_addr*)hp->h_addr));
	return 0;
}

/* read_timeout - 读超时检测函数，不含读操作 
 * fd:文件描述符 
 * wait_seconds:等待超时秒数， 如果为0表示不检测超时； 
 * 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT 
 */ 
int  read_timeout( int  fd,  unsigned   int  wait_seconds) 
{ 
     int  ret =  0 ; 
     if  (wait_seconds >  0 ) { 
        fd_set read_fdset; 
         struct  timeval timeout; 

        FD_ZERO(&read_fdset); 
        FD_SET(fd, &read_fdset); 

        timeout.tv_sec = wait_seconds; 
        timeout.tv_usec =  0 ; 

         do 
        { 
            ret = select(fd +  1 , &read_fdset,  NULL ,  NULL , &timeout);  //select会阻塞直到检测到事件或者超时 
             // 如果select检测到可读事件发送，则此时调用read不会阻塞 
        } 
         while  (ret <  0  && errno == EINTR); 

         if  (ret ==  0 ) 
        { 
            ret = - 1 ; 
            errno = ETIMEDOUT; 
        } 
         else   if  (ret ==  1 ) 
             return   0 ; 
    } 
     return  ret; 
} 

/* write_timeout - 写超时检测函数，不含写操作 
 * fd:文件描述符 
 * wait_seconds:等待超时秒数， 如果为0表示不检测超时； 
 * 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT 
 */ 
int  write_timeout( int  fd,  unsigned   int  wait_seconds) 
{ 
     int  ret =  0 ; 
     if  (wait_seconds >  0 ) 
    { 

        fd_set write_fdset; 
         struct  timeval timeout; 

        FD_ZERO(&write_fdset); 
        FD_SET(fd, &write_fdset); 

        timeout.tv_sec = wait_seconds; 
        timeout.tv_usec =  0 ; 

         do 
        { 
            ret = select(fd +  1 ,  NULL , &write_fdset,  NULL , &timeout); 
        } 
         while  (ret <  0  && errno == EINTR); 

         if  (ret ==  0 ) 
        { 
            ret = - 1 ; 
            errno = ETIMEDOUT; 
        } 
         else   if  (ret ==  1 ) 
             return   0 ; 

    } 

     return  ret; 
} 

/* accept_timeout - 带超时的accept 
 * fd: 套接字 
 * addr: 输出参数，返回对方地址 
 * wait_seconds: 等待超时秒数，如果为0表示正常模式 
 * 成功（未超时）返回已连接套接字，失败返回-1，超时返回-1并且errno = ETIMEDOUT 
 */ 
int  accept_timeout( int  fd,  struct  sockaddr_in *addr,  unsigned   int  wait_seconds) 
{ 
    int  ret; 
    socklen_t addrlen =  sizeof ( struct  sockaddr_in); 

    if  (wait_seconds >  0 ) { 
        fd_set accept_fdset; 
        struct  timeval timeout; 
        FD_ZERO(&accept_fdset); 
        FD_SET(fd, &accept_fdset); 

        timeout.tv_sec = wait_seconds; 
        timeout.tv_usec =  0 ; 

         do 
        { 
            ret = select(fd +  1 , &accept_fdset,  NULL ,  NULL , &timeout); 
        } 
         while  (ret <  0  && errno == EINTR); 

         if  (ret == - 1 ) 
             return  - 1 ; 
         else   if  (ret ==  0 ) 
        { 
            errno = ETIMEDOUT; 
             return  - 1 ; 
        } 
    } 

     if  (addr !=  NULL ) 
        ret = accept(fd, ( struct  sockaddr *)addr, &addrlen); 
     else 
        ret = accept(fd,  NULL ,  NULL ); 
     if  (ret == - 1 ) 
        ERR_EXIT( "accpet error" ); 

     return  ret; 
} 

/* activate_nonblock - 设置IO为非阻塞模式 
 * fd: 文件描述符 
 */ 
void  activate_nonblock( int  fd) 
{ 
     int  ret; 
     int  flags = fcntl(fd, F_GETFL); 
     if  (flags == - 1 ) 
        ERR_EXIT( "fcntl error" ); 

    flags |= O_NONBLOCK; 
    ret = fcntl(fd, F_SETFL, flags); 
     if  (ret == - 1 ) 
        ERR_EXIT( "fcntl error" ); 
} 

/* deactivate_nonblock - 设置IO为阻塞模式 
 * fd: 文件描述符 
 */ 
void  deactivate_nonblock( int  fd) 
{ 
     int  ret; 
     int  flags = fcntl(fd, F_GETFL); 
     if  (flags == - 1 ) 
        ERR_EXIT( "fcntl error" ); 

    flags &= ~O_NONBLOCK; 
    ret = fcntl(fd, F_SETFL, flags); 
     if  (ret == - 1 ) 
        ERR_EXIT( "fcntl error" ); 
} 

/* connect_timeout - 带超时的connect 
 * fd: 套接字 
 * addr: 输出参数，返回对方地址 
 * wait_seconds: 等待超时秒数，如果为0表示正常模式 
 * 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT 
 */ 
int  connect_timeout( int  fd,  struct  sockaddr_in *addr,  unsigned   int  wait_seconds) 
{ 
     int  ret; 
    socklen_t addrlen =  sizeof ( struct  sockaddr_in); 

     if  (wait_seconds >  0 ) 
        activate_nonblock(fd); 

    ret = connect(fd, ( struct  sockaddr *)addr, addrlen); 
     if  (ret <  0  && errno == EINPROGRESS) 
    { 

        fd_set connect_fdset; 
         struct  timeval timeout; 
        FD_ZERO(&connect_fdset); 
        FD_SET(fd, &connect_fdset); 

        timeout.tv_sec = wait_seconds; 
        timeout.tv_usec =  0 ; 

         do 
        { 
             /* 一旦连接建立，套接字就可写 */ 
            ret = select(fd +  1 ,  NULL , &connect_fdset,  NULL , &timeout); 
        } 
         while  (ret <  0  && errno == EINTR); 

         if  (ret ==  0 ) 
        { 
            errno = ETIMEDOUT; 
             return  - 1 ; 
        } 
         else   if  (ret <  0 ) 
             return  - 1 ; 

         else   if  (ret ==  1 ) 
        { 
             /* ret返回为1，可能有两种情况，一种是连接建立成功，一种是套接字产生错误 
             * 此时错误信息不会保存至errno变量中（select没出错）,因此，需要调用 
             * getsockopt来获取 */ 
             int  err; 
            socklen_t socklen =  sizeof (err); 
             int  sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen); 
             if  (sockoptret == - 1 ) 
                 return  - 1 ; 
             if  (err ==  0 ) 
                ret =  0 ; 
             else 
            { 
                errno = err; 
                ret = - 1 ; 
            } 
        } 
    } 

     if  (wait_seconds >  0 ) 
        deactivate_nonblock(fd); 

     return  ret; 
}

void send_fd(int sock_fd, int send_fd)
{
    int ret;
    struct msghdr msg;
    struct cmsghdr *p_cmsg;
    struct iovec vec;
    char cmsgbuf[CMSG_SPACE(sizeof(send_fd))];
    int *p_fds;
    char sendchar = 0;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    p_cmsg = CMSG_FIRSTHDR(&msg);
    p_cmsg->cmsg_level = SOL_SOCKET;
    p_cmsg->cmsg_type = SCM_RIGHTS;
    p_cmsg->cmsg_len = CMSG_LEN(sizeof(send_fd));
    p_fds = (int *)CMSG_DATA(p_cmsg);
    *p_fds = send_fd; // 通过传递辅助数据的方式传递文件描述符
    
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1; //主要目的不是传递数据，故只传1个字符
    msg.msg_flags = 0;
    
    vec.iov_base = &sendchar;
    vec.iov_len = sizeof(sendchar);
    ret = sendmsg(sock_fd, &msg, 0);
    if (ret != 1)
        ERR_EXIT("sendmsg");
}
    
int recv_fd(const int sock_fd)
{
    int ret;
    struct msghdr msg;
    char recvchar;
    struct iovec vec;
    int recv_fd;
    char cmsgbuf[CMSG_SPACE(sizeof(recv_fd))];
    struct cmsghdr *p_cmsg;
    int *p_fd;
    vec.iov_base = &recvchar;
    vec.iov_len = sizeof(recvchar);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    msg.msg_flags = 0;
    
    p_fd = (int *)CMSG_DATA(CMSG_FIRSTHDR(&msg));
    *p_fd = -1;
    ret = recvmsg(sock_fd, &msg, 0);
    if (ret != 1)
        ERR_EXIT("recvmsg");
    
    p_cmsg = CMSG_FIRSTHDR(&msg);
    if (p_cmsg == NULL)
        ERR_EXIT("no passed fd");
    
    
    p_fd = (int *)CMSG_DATA(p_cmsg);
    recv_fd = *p_fd;
    if (recv_fd == -1)
        ERR_EXIT("no passed fd");
    
    return recv_fd;
}


ssize_t writen (int fd, const void *buf, size_t num){
	ssize_t res;
	size_t n;
	const char *ptr;
	n = num;
	ptr = buf;
	while (n > 0) {
     if ((res = write (fd, ptr, n)) <= 0) {
      if (errno == EINTR)
       res = 0;
      else
       return (-1);
     }
     ptr += res;/* 从剩下的地方继续写     */ 
     n -= res;
	}
	
	
	return (num);
}

ssize_t readn (int fd, void *buf, size_t num){
	ssize_t res;
	size_t n;
	char *ptr;
	
	n = num;
	ptr = buf;
	while (n > 0) {
	     if ((res = read (fd, ptr, n)) == -1) {
	      if (errno == EINTR)
	       res = 0;
	      else
	       return (-1);
	     }
	     else if (res == 0)
	      break;
	
	
	     ptr += res;
	     n -= res;
	}
	return (num - n);
}

static int  read_cnt;//刚开始可以置为一个负值（我的理解）  
static char *read_ptr;  
static char read_buf[MAX_COMMAND_LINE];  
  
static ssize_t  
my_read(int fd, char *ptr)//每次最多读取MAXLINE个字符，调用一次，每次只返回一个字符  
{  
  
    if (read_cnt <= 0) {  
again:  
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {//如果读取成功，返回read_cnt=读取的字符           if (errno == EINTR)  
                goto again;  
            return(-1);  
        } else if (read_cnt == 0)  
            return(0);  
        read_ptr = read_buf;  
    }  
  
    read_cnt--;//每次递减1，直到<0读完，才执行上面if的命令。  
    *ptr = *read_ptr++;//每次读取一个字符，转移一个字符  
    return(1);  
}  


ssize_t readline(int fd, void *vptr, size_t maxlen)  
{  
    ssize_t n, rc;  
    char    c, *ptr;  
  
    ptr = vptr;  
    for (n = 1; n < maxlen; n++) {  
        if ( (rc = my_read(fd, &c)) == 1) {  
            *ptr++ = c;  
            if (c == '\n')  
                break;  /* newline is stored, like fgets() */  
        } else if (rc == 0) {  
            *ptr = 0;  
            return(n - 1);  /* EOF, n - 1 bytes were read */  
        } else  
            return(-1);     /* error, errno set by read() */  
    }  
  
    *ptr = 0;   /* null terminate like fgets() */  
    return(n);  
}  
