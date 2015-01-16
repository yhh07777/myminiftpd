#include "common.h"
#include "sysutil.h"
#include "session.h"
#include "str.h"
#include "tunable.h"
#include "parseconf.h"
#include "ftpproto.h"

int main(void){
	
	/*
	list_common();
	exit(EXIT_SUCCESS);
	*/
	
	/*
	// ����str_to_longlong
	long long result=str_to_longlong("1234567891234");
	printf("result=%lld\n",result);
	
	// ����str_octal_to_uint
	int n=str_octal_to_uint("0711");
	printf("n=%d",n);
	*/	
	/*
	int tunable_pasv_enable=1;											// �Ƿ�������ģʽ
	int tunable_port_enable=1;											// �Ƿ�������ģʽ
	unsigned int tunable_listen_port=21;						// ftp�������˿�
	unsigned int tunable_max_clients=2000;					// ���������
	unsigned int tunable_max_per_ip=50;							// ÿIP���������
	unsigned int tunable_accept_timeout=60; 				// accept��ʱ��
	unsigned int tunable_connect_timeout=60;				// connect��ʱ��
	unsigned int tunable_idle_session_timeout=300;	// �������ӳ�ʱʱ��
	unsigned int tunable_data_connection_timeout=300;	// �������ӳ�ʱʱ��
	unsigned int tunable_local_umask=077;							// ����
	unsigned int tunable_upload_max_rate=0;						// ����ϴ��ٶ�
	unsigned int tunable_download_max_rate=0;					// ��������ٶ�
	const char *tunable_listen_address;								// ftp������IP��ַ
	*/	
	printf("===============��ʼ��ӡ������Ϣ===============\n");
	parseconf_load_file(MINIFTP_CONF);
	printf("tunable_pasv_enable=%d\n",tunable_pasv_enable);
	printf("tunable_port_enable=%d\n",tunable_port_enable);
	
	printf("tunable_listen_port=%u\n",tunable_listen_port);
	printf("tunable_max_clients=%u\n",tunable_max_clients);
	printf("tunable_max_per_ip=%u\n",tunable_max_per_ip);
	printf("tunable_accept_timeout=%u\n",tunable_accept_timeout);
	printf("tunable_connect_timeout=%u\n",tunable_connect_timeout);
	printf("tunable_idle_session_timeout=%u\n",tunable_idle_session_timeout);
	printf("tunable_data_connection_timeout=%u\n",tunable_data_connection_timeout);
	printf("tunable_local_umask=0%o\n",tunable_local_umask);
	printf("tunable_upload_max_rate=%u\n",tunable_upload_max_rate);
	printf("tunable_download_max_rate=%u\n",tunable_download_max_rate);
	
	if(tunable_listen_address==NULL){
		printf("tunable_listen_address=NULL\n");
	}else{
		printf("tunable_listen_address=%s\n",tunable_listen_address);
	}
	
	if(getuid()!=0){
		fprintf(stderr,"miniftpd:must be started as root\n");	
	}	

	session_t sess={
		/* �������� */
		0,-1,"","","",
		/* �������� */
		NULL,-1,-1,
		/* ���ӽ���ͨ�� */
		-1,-1,
		/* FTPЭ��״̬ */
		0
	};
	
	printf("listen...\n");
	int listenfd=tcp_server(NULL,5188);
	int conn;
	pid_t pid;
	while(1){
		//����ʱ
		conn=accept_timeout(listenfd,NULL,0);
		if(conn==-1) ERR_EXIT("accept_timeout");
		printf("�пͻ������ӹ���\n");
		pid=fork();
		if(pid==-1) ERR_EXIT("fork");
		if(pid==0){
			close(listenfd);	// �����Ӳ���Ҫ�������
			sess.ctrl_fd=conn;
			begin_session(&sess);
		}else{
			
			close(conn);	// �����̲���Ҫ��������
			sleep(2);
		}
	}
	return 0;
}
