#ifndef _SESSION_H_
#define _SESSION_H_

#include "common.h"

typedef struct session{
	uid_t uid;
	// 控制连接
	int ctrl_fd;
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];
	
	// 数据连接
	struct sockaddr_in *port_addr;
	int pasv_listen_fd;		// 被动方式连接套接字
	int data_fd;
	
	// 父子进程通道
	int parent_fd;
	int child_fd;
	
	// FTP协议状态:1-ascii 0-binary
	int is_ascii;
	
}session_t;

void begin_session(session_t *sess);

#endif /*_SESSION_H_*/
