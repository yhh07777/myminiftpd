#include "common.h"
#include "session.h"
#include "privparent.h"
#include "ftpproto.h"

void begin_session(session_t *sess){
	// ����socketpair���ӽ���֮��ͨ��
	int sockfds[2];
	int ret=socketpair(PF_UNIX,SOCK_STREAM,0,sockfds);
	if(ret<0) ERR_EXIT("socketpair");
	
	pid_t pid;
	pid=fork();		// һ��sessio����������
	if(pid<0){
		ERR_EXIT("fork");
	}else if(pid==0){		//ftp�������:�����������,��������
		printf("ftp�������[%d]����\n",pid);
		close(sockfds[0]);
		sess->child_fd=sockfds[1];
		handle_child(sess);
	}else{							
		// ��ȡ�û���¼�������Ϣ
		struct passwd *pw=getpwnam("nobody");
		if(pw==NULL)return;
			
		// ��gid,uid�������̸�Ϊnobody����
		// ������������Ϊ��ftp���̾��з���Ӱ���ļ���Ȩ��
		ret=setegid(pw->pw_gid);
		if(ret<0) ERR_EXIT("setegid");
		ret=seteuid(pw->pw_uid);
		if(ret<0) ERR_EXIT("seteuid");
		
		//nobody����:�������ͨ��,Ȩ�ޱ���ͨ����Ȩ�޸�						
		close(sockfds[1]);
		sess->parent_fd=sockfds[0];
		handle_parent(sess);
	}			
}
