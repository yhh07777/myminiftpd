#include "common.h"
#include "sysutil.h"
#include "ftpproto.h"
#include "str.h"
#include "ftpcodes.h"
#include "tunable.h"

/* ftp命令的结构体 */
typedef struct ftpcmd
{
	const char* cmd;
	void (*cmd_handler)(session_t *sess);	
}ftpcmd_t;

void ftp_reply(session_t *sess,int status,const char *text){
	char buf[1024]={0};
	sprintf(buf,"%d %s\r\n",status,text);	
	printf("%s\n",buf);	
	writen(sess->ctrl_fd,buf,strlen(buf));
}

void ftp_lreply(session_t *sess,int status,const char *text){
	char buf[1024]={0};
	sprintf(buf,"%d-%s\r\n",status,text);	
	printf("%s\n",buf);	
	writen(sess->ctrl_fd,buf,strlen(buf));
}

// 列出当前目录
int list_common(session_t *sess){
	DIR *dir=opendir(".");
	if(dir==NULL){
		return 0;	
	}
	
	struct dirent *dt;
	struct stat sbuf;
	while((dt=readdir(dir))!=NULL){
		if(lstat(dt->d_name,&sbuf)<0){// 符号连接文件要显示本身状态,不是显示所指向的文件状态
			continue;
		}
		
		// 过滤掉.号
		if(dt->d_name[0]=='.'){
			continue;	
		}
		
		char perms[]="----------";	// 权限位
		perms[0]='?';
		mode_t mode=sbuf.st_mode;
		switch(mode & S_IFMT){		//S_IFMT:文件类型的掩码
			case S_IFREG:
					perms[0]='-';	// 普通文件
					break;
			case S_IFDIR:
					perms[0]='d';	// 目录文件	
					break;
			case S_IFLNK:		// 软连接			
					perms[0]='l';	
					break;
			case S_IFIFO:			// 管道文件
					perms[0]='p';	
					break;
			case S_IFSOCK:		// socket文件
					perms[0]='s';	
					break;
			case S_IFCHR:
					perms[0]='c';	// 字符设备文件
					break;
			case S_IFBLK:				// 块设备文件
					perms[0]='b';	
					break;		
		}
		
		// 获取每个文件的权限
		// 拥有者权限
		if(mode & S_IRUSR){
			perms[1]='r';	
		}
		if(mode & S_IWUSR){
			perms[2]='w';	
		}
		if(mode & S_IXUSR){
			perms[3]='x';	
		}
		// 组权限
		if(mode & S_IRGRP){
			perms[4]='r';	
		}
		if(mode & S_IWGRP){
			perms[5]='w';	
		}
		if(mode & S_IXGRP){
			perms[6]='x';	
		}
		// 其他用户的权限
		if(mode & S_IROTH){
			perms[7]='r';	
		}
		if(mode & S_IWOTH){
			perms[8]='w';	
		}
		if(mode & S_IXOTH){
			perms[9]='x';	
		}
		
		if(mode & S_ISUID){
			perms[3]=(perms[3]=='x'?'s':'S');	
		}
		if(mode & S_ISGID){
			perms[6]=(perms[6]=='x'?'s':'S');	
		}
		if(mode & S_ISGID){
			perms[9]=(perms[3]=='x'?'t':'T');	
		}
		
		// 获取连接数
		char buf[1024]={0};
		int off=0;
		off+=sprintf(buf,"%s",perms);	// 返回格式化到buf中的字符串长度
		
		// 用户id组id等
		off+=sprintf(buf+off," %3d %-8d %-8d ",sbuf.st_nlink,sbuf.st_uid,sbuf.st_gid);
		
		// 文件大小
		off+=sprintf(buf+off,"%8lu ",(unsigned long)sbuf.st_size);
		
		// 日期
		const char *p_date_format="%b %e %H:%M";
		struct timeval tv;
		gettimeofday(&tv,NULL);
		long local_time=tv.tv_sec;		// 获取系统当前时间
		if(sbuf.st_mtime>local_time||(local_time-sbuf.st_mtime)>182*24*60*60){	// 文件时间大于当前时间或者在半年前
			p_date_format="%b %e %Y";
		}
		
		char datebuf[64]={0};
		struct tm* p_tm=localtime(&local_time);	// 将秒转换为结构体
		strftime(datebuf,sizeof(datebuf),p_date_format,p_tm);	// strftime:格式化时间
		off += sprintf(buf+off,"%s ",datebuf);
		
		// 文件名
		if(S_ISLNK(sbuf.st_mode)){		// 如果是符号连接文件
			char tmp[1024]={0};
			readlink(dt->d_name,tmp,sizeof(tmp));		// 获取符号链接链接的文件名称
			off+=sprintf(buf+off,"%s -> %s\r\n",dt->d_name,tmp);
		}else{
			off+=sprintf(buf+off,"%s \r\n",dt->d_name);		
		}
		
		printf("%s",buf);		
		
		// 将数据发送给客户端
		writen(sess->data_fd,buf,strlen(buf));
	}
	printf("\n");
	
	closedir(dir);
	return 1;
		
}

// 是否处于主动模式
int port_active(session_t *sess){
	if(sess->port_addr){
		if(pasv_active(sess)){
			fprintf(stderr,"both port and pasv are active");
			exit(EXIT_FAILURE);
		}
		return 1;	
	}
	return 0;
}

// 是否处于被动模式
int pasv_active(session_t *sess){
	if(sess->pasv_listen_fd!=-1){
		if(port_active(sess)){
			fprintf(stderr,"both port and pasv are active");
			exit(EXIT_FAILURE);
		}
		return 1;	
	}
	return 0;
}

int get_transfer_fd(session_t *sess){
	printf("get_transfer_fd...\n");
	
	// 检测是否收到PORT或者PASV命令
	if(!port_active(sess)&&!pasv_active(sess)){
			printf("get_transfer_fd->未收到PORT或者PASV命令\n");
			ftp_reply(sess,"Use PORT or PASV first.");
			return 0;
	}
	
	// 如果是主动模式
	if(port_active(sess)){
		printf("get_transfer_fd->是主动模式\n");
		// 绑定20端口
		// sess->data_fd=tcp_client(0);
		//int fd=tcp_client(20);
		int fd=tcp_client(0);
		printf("port=%d\n",sess->port_addr->sin_port);
		//printf("addr=%s\n",(char*)(sess->port_addr->sin_addr));
		// 发起连接
		if(connect_timeout(fd,sess->port_addr,tunable_connect_timeout)<0){
				printf("get_transfer_fd->connect_timeout:发起连接出错\n");
				close(fd);
				return 0;
		}
		
		sess->data_fd=fd;
	}
	
	// 如果是被动模式
	if(pasv_active(sess)){
		int pasv_active_fd=accept_timeout(sess->pasv_listen_fd,NULL,tunable_accept_timeout);
		close(sess->pasv_listen_fd);	
		
		if(pasv_active_fd==-1){
			return 0;	// 获取连接套接字失败
		}
		
		sess->data_fd=fd;
		
	}
	
	// 释放do_port释放的内存
	if(sess->port_addr){
		free(sess->port_addr);
		sess->port_addr=NULL;	
	}
	
	return 1;
}

/* 验证用户是否存在 */
static void do_user(session_t *sess){
	struct passwd *pw=getpwnam(sess->arg);
	if(pw==NULL){
		// 用户不存在
		ftp_reply(sess,FTP_LOGINERR,"Login incorrect");
		return;
	}
	sess->uid=pw->pw_uid;
	ftp_reply(sess,FTP_GIVEPWORD,"Please specify the password.");
}

static void do_pass(session_t *sess){
	// 根据uid获取密码结构体
	struct passwd *pw=getpwuid(sess->uid);
	if(pw==NULL){
		// 用户不存在
		printf("用户不存在\n");
		ftp_reply(sess,FTP_LOGINERR,"Login incorrect");
		return;
	}
	
	struct spwd *sp=getspnam(pw->pw_name);	// 获取影子文件信息
	if(sp==NULL){
		printf("用户影子文件不存在\n");
		ftp_reply(sess,FTP_LOGINERR,"Login incorrect");
		return;	
	}
	
	// 将明文进行加密,将加密的结果跟影子文件中密码对比
	char *encrypted_pass = crypt(sess->arg,sp->sp_pwdp);		// crypt(明文,种子);
	if(strcmp(encrypted_pass,sp->sp_pwdp)){
		printf("用户密码验证错误\n");
		ftp_reply(sess,FTP_LOGINERR,"Login incorrect");
		return;			
	}
	
	// 230-登陆成功
	printf("%s登陆成功\n",pw->pw_name);
	
	setegid(pw->pw_gid);	// 更改有效用户组id
	seteuid(pw->pw_uid);	// 更改有效用户uid
	chdir(pw->pw_dir);		// 更改实际用户目录
	ftp_reply(sess,FTP_LOGINOK,"Login successful.");
}
static void do_cwd(session_t *sess){}
static void do_cdup(session_t *sess){}
static void do_quit(session_t *sess){}

/* port模式 */
static void do_port(session_t *sess){
	printf("do_port->客户端使用port模式\n");
	// PORT 192,168,0,100,123,233
	unsigned int v[6];
	sscanf(sess->arg,"%u,%u,%u,%u,%u,%u",&v[2],&v[3],&v[4],&v[5],&v[0],&v[1]);	// 从字符串格式化输入
	printf("v[0]=%d,v[1]=%d\n",v[0],v[1]);
	sess->port_addr=(struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	memset(sess->port_addr,0,sizeof(struct sockaddr_in));
	sess->port_addr->sin_family=AF_INET;
	unsigned char *p=(unsigned char *)&sess->port_addr->sin_port;	// 端口号
	p[0]=v[0];
	p[1]=v[1];
	
	// 设置IP地址 
	p=(unsigned char *)&sess->port_addr->sin_addr;
	p[0]=v[2];
	p[1]=v[3];
	p[2]=v[4];
	p[3]=v[5];
	
	ftp_reply(sess,FTP_PORTOK,"PORT command successful. Consider using PASV.");
}

static void do_pasv(session_t *sess){
	//Entering Passive Mode (192,168,244,100,101,46)
	char ip[16]={0};
	getlocalip(ip);
	int fd=tcp_server(ip,0);
	
	// 保存被动模式监听套接字
	sess->pasv_listen_fd=fd;
	struct sockaddr_in addr;
	socklen_t addrlen=sizeof(addr);
	// fd是需要已连接的
	if(getsockname(fd,(struct sockaddr *)&addr,&addrlen)<0){
		ERR_EXIT("do_pasv->getsockname");	
	}
	// 网络字节序转换为主机字节序
	unsigned short port=ntohs(addr.sin_port);
	unsigned int v[4];
	sscanf(ip,"%u.%u.%u.%u",&v[0],&v[1],&v[2],&v[3]);
	char text[1024]={0};
	sscanf(ip,"%u.%u.%u.%u",&v[0],&v[1],&v[2],&v[3]);
	sprintf(text,"Entering Passive Mode (%u,%u,%u,%u,%u,%u).",
		v[0],v[1],v[2],v[3],port>>8,port&0xFF);
		
	ftp_reply(sess,FTP_PASVOK,text);
}

/* 转换传输模式 */
static void do_type(session_t *sess){
	if(strcmp(sess->arg,"A")==0){	// ascii
		sess->is_ascii=1;
		ftp_reply(sess,FTP_TYPEOK,"Switching to ASCII mode.");	
	}else if(strcmp(sess->arg,"I")==0){	// binary
		sess->is_ascii=0;
		ftp_reply(sess,FTP_TYPEOK,"Switching to Binary mode.");	
	}else{
		ftp_reply(sess,FTP_BADCMD,"Unrecognized TYPE command.");		
	}
	
}
static void do_stru(session_t *sess){}
static void do_mode(session_t *sess){}

static void do_retr(session_t *sess){}
static void do_stor(session_t *sess){}
static void do_appe(session_t *sess){}

// 传输列表
static void do_list(session_t *sess){
	printf("do_list...\n");
	
	// 创建数据连接
	if(get_transfer_fd(sess)==0){
		printf("do_list->get_transfer_fd:创建数据连接失败!\n");
		return ;
	}
	
	// 150
	ftp_reply(sess,FTP_DATACONN,"Here comes the directory listing.");	
	// 传输列表
	list_common(sess);
	// 关闭数据套接字,防止对方接受不到应答
	close(sess->data_fd);
	// 226
	ftp_reply(sess,FTP_TRANSFEROK,"Directory send OK.");	
}
static void do_nlist(session_t *sess){}
static void do_rest(session_t *sess){}
static void do_abor(session_t *sess){}

/* 读取当前目录 */
static void do_pwd(session_t *sess){
	char text[1024]={0};
	char dir[1024+1]={0};
	getcwd(dir,1024);
	sprintf(text,"\"%s\"",dir);
	
	ftp_reply(sess,FTP_PWDOK,text);
}
static void do_mkd(session_t *sess){}
static void do_rmd(session_t *sess){}
static void do_dele(session_t *sess){}
static void do_rnfr(session_t *sess){}
static void do_rnto(session_t *sess){}
static void do_site(session_t *sess){}

static void do_syst(session_t *sess){
	ftp_reply(sess,FTP_SYSTOK,"UNIX Type:L8");
}
static void do_size(session_t *sess){}

/* 服务器特性 */
static void do_feat(session_t *sess){
	ftp_lreply(sess,FTP_FEAT,"Features:");
	writen(sess->ctrl_fd," EPRT\r\n",strlen(" EPRT\r\n"));
	writen(sess->ctrl_fd," EPSV\r\n",strlen(" EPSV\r\n"));
	writen(sess->ctrl_fd," MDTM\r\n",strlen(" MDTM\r\n"));
	writen(sess->ctrl_fd," PASV\r\n",strlen(" PASV\r\n"));
	writen(sess->ctrl_fd," REST STREAM\r\n",strlen("REST STREAM\r\n"));
	writen(sess->ctrl_fd," SIZE\r\n",strlen(" SIZE\r\n"));
	writen(sess->ctrl_fd," TVFS\r\n",strlen(" TVFS\r\n"));
	writen(sess->ctrl_fd," UTF8\r\n",strlen(" UTF8\r\n"));
	ftp_reply(sess,FTP_FEAT,"End:");
}

static void do_stat(session_t *sess){}
static void do_noop(session_t *sess){}
static void do_help(session_t *sess){}
	
static ftpcmd_t ctrl_cmds[]={	
	/* 服务命令 */
	{"USER",do_user},
	{"PASS",do_pass},
	{"CWD",do_cwd},
	{"XCWD",do_cwd},
	{"DCUP",do_cdup},
	{"XCUP",do_cdup},
	{"QUIT",do_quit},
	{"ACCT",NULL},
	{"SMNT",NULL},
	{"REIN",NULL},
	
	/* 传输参数命令 */
	{"PORT",do_port},
	{"PASV",do_pasv},
	{"TYPE",do_type},
	{"STRU",do_stru},
	{"MODE",do_mode},
	
	/* 服务命令 */
	{"RETR",do_retr},
	{"STOR",do_stor},
	{"APPE",do_appe},
	{"LIST",do_list},
	{"NLIST",do_nlist},
	{"REST",do_rest},
	{"ABOR",do_abor},
	{"\377\364\377\362ABOR",do_abor},
	{"PWD",do_pwd},
	{"XPWD",do_pwd},
	{"MKD",do_mkd},
	{"XMKD",do_mkd},
	{"RMD",do_rmd},
	{"XRMD",do_rmd},
	{"DELE",do_dele},
	{"RNFR",do_rnfr},
	{"RNTO",do_rnto},
	{"SITE",do_site},
	{"SYST",do_syst},
	{"FEAT",do_feat/* NULL */},
	{"SIZE",do_size},
	{"STAT",do_stat},
	{"NOOP",do_noop},
	{"HELP",do_help},
	{"STOU",NULL},
	{"ALLO",NULL}
};

void ftp_reply(session_t *sess,int status,const char *text);
// static 代表只能本文件使用
static void do_user(session_t *sess);
static void do_pass(session_t *sess);

void handle_child(session_t *sess){
	printf("handle_child->%d\n",sess->ctrl_fd);
	//writen(sess->ctrl_fd,"220 (miniftpd 0.1)\r\n",strlen("220 (miniftpd 0.1)\r\n"));// 每一条都加上\r\n
	ftp_reply(sess,FTP_GREET,"(miniftpd 0.1)");

	int ret;
	while(1){
		memset(sess->cmdline,0,sizeof(sess->cmdline));
		memset(sess->cmd,0,sizeof(sess->cmd));
		memset(sess->arg,0,sizeof(sess->arg));
		ret=readline(sess->ctrl_fd,sess->cmdline,MAX_COMMAND_LINE);	
		printf("handle_child->ret=%d\n",ret);
		
		if(ret<0) ERR_EXIT("handle_child->readline");
		else if(ret==0) exit(EXIT_SUCCESS);
		
		// 去除\r\n
		str_trim_crlf(sess->cmdline);
		printf("cmdline=[%s]\n",sess->cmdline);
		
		// 解析FTP命令与参数
		str_split(sess->cmdline,sess->cmd,sess->arg,' ');
		printf("cmd=[%s] arg=[%s]",sess->cmd,sess->arg);
		// 将命令转换为大写
		str_upper(sess->cmd);
		
		// 处理FTP命令
		/*if(strcmp("USER",sess->cmd)==0){
			do_user(sess);	
		}else if(strcmp("PASS",sess->cmd)==0){
			do_pass(sess);
		}*/
		int i;
		int size=sizeof(ctrl_cmds)/sizeof(ctrl_cmds[0]);		// 数组长度=数组变量的大小/每个数组大小
		for(i=0;i<size;i++){
			if(strcmp(ctrl_cmds[i].cmd,sess->cmd)==0){
				if(ctrl_cmds[i].cmd_handler!=NULL){
					ctrl_cmds[i].cmd_handler(sess);		
				}else{
					ftp_reply(sess,FTP_COMMANDNOTIMPL,"Unimplement command.");
				}
				
				break;	
			}	
		}
		
		if(i==size){	// 如果没有该命令
			ftp_reply(sess,FTP_BADCMD,"Unknown command.");
		}
		
	}		
	
}

