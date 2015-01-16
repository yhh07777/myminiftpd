#include "sysutil.h"

int tcp_client(unsigned short port){
	int sockt;	
	if((sockt=socket(PF_INET,SOCK_STREAM,0))<0) ERR_EXIT("tcp_client");
		
	if(port>0){
		// ���õ�ַ�ظ�����
		int on=1;
		int ret=setsockopt(sockt,SOL_SOCKET,SO_REUSEADDR,(const char*)&on,sizeof(on));
		if(ret<0) ERR_EXIT("setsockopt");	
		
		char ip[16]={0};
		getlocalip(ip);		// ��ȡ����ip
		
		struct sockaddr_in localaddr;
		memset(&localaddr,0,sizeof(localaddr));
		localaddr.sin_family=AF_INET;
		localaddr.sin_port=htons(port);
		localaddr.sin_addr.s_addr=inet_addr(ip);
		
		// ��
		if(bind(sockt,(struct sockaddr*)&localaddr,sizeof(localaddr))<0){
			ERR_EXIT("tcp_client->bind");		
		}
	}
	
	return sockt;
}

/**
* tcp_server - ����tcp������
* @host:������IP��ַ���߷�����������
* @port:�������˿�
* �ɹ��ָ������׽���
*/
int tcp_server(const char *host,unsigned short port){
	int listenfd;
	if((listenfd=socket(PF_INET,SOCK_STREAM,0))<0) ERR_EXIT("tcp_server");
		
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	
	if(host!=NULL){
		// �ж��Ƿ��ǺϷ���ַ
		if(inet_aton(host,&servaddr.sin_addr)==0){
			struct hostent *hp;
			if((hp=gethostbyname(host))==NULL) ERR_EXIT("gethostbyname");
			servaddr.sin_addr=*(struct in_addr*)hp->h_addr;	
		}	
	}else{
		servaddr.sin_addr.s_addr=htonl(INADDR_ANY);	
	}
	servaddr.sin_port=htons(port);
	
	// ���õ�ַ�ظ�����
	int on=1;
	int ret=setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const char*)&on,sizeof(on));
	if(ret<0) ERR_EXIT("gethostbyname");
	
	// ��
	ret=bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));	
	if(ret<0)	ERR_EXIT("tcp_server->bind");
		
	// ����
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

/* read_timeout - ����ʱ��⺯�������������� 
 * fd:�ļ������� 
 * wait_seconds:�ȴ���ʱ������ ���Ϊ0��ʾ����ⳬʱ�� 
 * �ɹ���δ��ʱ������0��ʧ�ܷ���-1����ʱ����-1����errno = ETIMEDOUT 
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
            ret = select(fd +  1 , &read_fdset,  NULL ,  NULL , &timeout);  //select������ֱ����⵽�¼����߳�ʱ 
             // ���select��⵽�ɶ��¼����ͣ����ʱ����read�������� 
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

/* write_timeout - д��ʱ��⺯��������д���� 
 * fd:�ļ������� 
 * wait_seconds:�ȴ���ʱ������ ���Ϊ0��ʾ����ⳬʱ�� 
 * �ɹ���δ��ʱ������0��ʧ�ܷ���-1����ʱ����-1����errno = ETIMEDOUT 
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

/* accept_timeout - ����ʱ��accept 
 * fd: �׽��� 
 * addr: ������������ضԷ���ַ 
 * wait_seconds: �ȴ���ʱ���������Ϊ0��ʾ����ģʽ 
 * �ɹ���δ��ʱ�������������׽��֣�ʧ�ܷ���-1����ʱ����-1����errno = ETIMEDOUT 
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

/* activate_nonblock - ����IOΪ������ģʽ 
 * fd: �ļ������� 
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

/* deactivate_nonblock - ����IOΪ����ģʽ 
 * fd: �ļ������� 
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

/* connect_timeout - ����ʱ��connect 
 * fd: �׽��� 
 * addr: ������������ضԷ���ַ 
 * wait_seconds: �ȴ���ʱ���������Ϊ0��ʾ����ģʽ 
 * �ɹ���δ��ʱ������0��ʧ�ܷ���-1����ʱ����-1����errno = ETIMEDOUT 
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
             /* һ�����ӽ������׽��־Ϳ�д */ 
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
             /* ret����Ϊ1�����������������һ�������ӽ����ɹ���һ�����׽��ֲ������� 
             * ��ʱ������Ϣ���ᱣ����errno�����У�selectû����,��ˣ���Ҫ���� 
             * getsockopt����ȡ */ 
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
    *p_fds = send_fd; // ͨ�����ݸ������ݵķ�ʽ�����ļ�������
    
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1; //��ҪĿ�Ĳ��Ǵ������ݣ���ֻ��1���ַ�
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
     ptr += res;/* ��ʣ�µĵط�����д     */ 
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

static int  read_cnt;//�տ�ʼ������Ϊһ����ֵ���ҵ���⣩  
static char *read_ptr;  
static char read_buf[MAX_COMMAND_LINE];  
  
static ssize_t  
my_read(int fd, char *ptr)//ÿ������ȡMAXLINE���ַ�������һ�Σ�ÿ��ֻ����һ���ַ�  
{  
  
    if (read_cnt <= 0) {  
again:  
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {//�����ȡ�ɹ�������read_cnt=��ȡ���ַ�           if (errno == EINTR)  
                goto again;  
            return(-1);  
        } else if (read_cnt == 0)  
            return(0);  
        read_ptr = read_buf;  
    }  
  
    read_cnt--;//ÿ�εݼ�1��ֱ��<0���꣬��ִ������if�����  
    *ptr = *read_ptr++;//ÿ�ζ�ȡһ���ַ���ת��һ���ַ�  
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
