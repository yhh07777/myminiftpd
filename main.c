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
	// 测试str_to_longlong
	long long result=str_to_longlong("1234567891234");
	printf("result=%lld\n",result);
	
	// 测试str_octal_to_uint
	int n=str_octal_to_uint("0711");
	printf("n=%d",n);
	*/	
	/*
	int tunable_pasv_enable=1;											// 是否开启被动模式
	int tunable_port_enable=1;											// 是否开启主动模式
	unsigned int tunable_listen_port=21;						// ftp服务器端口
	unsigned int tunable_max_clients=2000;					// 最大连接数
	unsigned int tunable_max_per_ip=50;							// 每IP最大连接数
	unsigned int tunable_accept_timeout=60; 				// accept超时间
	unsigned int tunable_connect_timeout=60;				// connect超时间
	unsigned int tunable_idle_session_timeout=300;	// 控制连接超时时间
	unsigned int tunable_data_connection_timeout=300;	// 数据连接超时时间
	unsigned int tunable_local_umask=077;							// 掩码
	unsigned int tunable_upload_max_rate=0;						// 最大上传速度
	unsigned int tunable_download_max_rate=0;					// 最大下载速度
	const char *tunable_listen_address;								// ftp服务器IP地址
	*/	
	printf("===============开始打印配置信息===============\n");
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
		/* 控制连接 */
		0,-1,"","","",
		/* 数据连接 */
		NULL,-1,-1,
		/* 父子进程通道 */
		-1,-1,
		/* FTP协议状态 */
		0
	};
	
	printf("listen...\n");
	int listenfd=tcp_server(NULL,5188);
	int conn;
	pid_t pid;
	while(1){
		//不超时
		conn=accept_timeout(listenfd,NULL,0);
		if(conn==-1) ERR_EXIT("accept_timeout");
		printf("有客户端连接过来\n");
		pid=fork();
		if(pid==-1) ERR_EXIT("fork");
		if(pid==0){
			close(listenfd);	// 子连接不需要处理监听
			sess.ctrl_fd=conn;
			begin_session(&sess);
		}else{
			
			close(conn);	// 父进程不需要处理连接
			sleep(2);
		}
	}
	return 0;
}
