#ifndef _SYS_UTIL_H_
#define _SYS_UTIL_H_

#include "common.h"

int tcp_client(unsigned short port);

int tcp_server(const char *host,unsigned short port);
int getlocalip(char *ip);

int read_timeout(int fd,unsigned int wait_seconds);
int write_timeout(int fd,unsigned int wait_seconds);
int accept_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds);
void activate_nonblock(int fd);
void deactivate_nonblock(int fd);
int connect_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds);

ssize_t readn(int fd,void *buf,size_t count);
ssize_t writen(int fd,const void *buf,size_t count);
ssize_t recv_peek(int fd,void *buf,size_t len);
ssize_t readline(int sockfd,void *buf,size_t maxline);

// ·¢ËÍÎÄ¼şÃèÊö·û
void send_fd(int sock_fd,int fd);
int recv_fd(const int sock_fd);

#endif /*_SYS_UTIL_H_*/
