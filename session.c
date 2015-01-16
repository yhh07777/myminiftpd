#include "common.h"
#include "session.h"
#include "privparent.h"
#include "ftpproto.h"

void begin_session(session_t *sess){
	// 利用socketpair父子进程之间通信
	int sockfds[2];
	int ret=socketpair(PF_UNIX,SOCK_STREAM,0,sockfds);
	if(ret<0) ERR_EXIT("socketpair");
	
	pid_t pid;
	pid=fork();		// 一个sessio有两个进程
	if(pid<0){
		ERR_EXIT("fork");
	}else if(pid==0){		//ftp服务进程:处理控制连接,数据连接
		printf("ftp服务进程[%d]启动\n",pid);
		close(sockfds[0]);
		sess->child_fd=sockfds[1];
		handle_child(sess);
	}else{							
		// 获取用户登录的相关信息
		struct passwd *pw=getpwnam("nobody");
		if(pw==NULL)return;
			
		// 改gid,uid将父进程改为nobody进程
		// 放在这里是因为让ftp进程具有访问影子文件的权限
		ret=setegid(pw->pw_gid);
		if(ret<0) ERR_EXIT("setegid");
		ret=seteuid(pw->pw_uid);
		if(ret<0) ERR_EXIT("seteuid");
		
		//nobody进程:不与外界通信,权限比普通进程权限高						
		close(sockfds[1]);
		sess->parent_fd=sockfds[0];
		handle_parent(sess);
	}			
}
