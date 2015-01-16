#include "privparent.h" 

void handle_parent(session_t *sess){
	char cmd;
	while(1){
		// 从子进程中读取数据
		read(sess->parent_fd,&cmd,1);	
		// 解析内部命令
		
		// 处理内部命令
	}
}
