#include "privparent.h" 

void handle_parent(session_t *sess){
	char cmd;
	while(1){
		// ���ӽ����ж�ȡ����
		read(sess->parent_fd,&cmd,1);	
		// �����ڲ�����
		
		// �����ڲ�����
	}
}
