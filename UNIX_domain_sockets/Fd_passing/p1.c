#include<sys/socket.h>
#include<sys/un.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#define PATH "p1address"
#define DEST "p2address"
#define PORT 8787

void send_fd_dgram(int* usfd,int sendFd){
	*usfd = socket(AF_UNIX,SOCK_DGRAM,0);
	struct sockaddr_un addr;
	memset(&addr,0,sizeof addr);
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path,PATH,sizeof addr.sun_path - 1);
	if(remove(PATH)==-1 && errno != ENOENT){
		perror("remove ");
	}
	if(bind(*usfd,(struct sockaddr*)&addr,sizeof addr)==-1){
		perror("bind ");
	}
	struct iovec e = {NULL,0};
	char cmsg[CMSG_SPACE(sizeof(int))];
	struct sockaddr_un destaddr;
	memset(&destaddr,0,sizeof destaddr);
	destaddr.sun_family=AF_UNIX;
	strncpy(destaddr.sun_path,DEST,sizeof destaddr.sun_path - 1);
	struct msghdr m = {(void*)&destaddr, sizeof(destaddr), &e, 1, cmsg, sizeof(cmsg), 0};
	struct cmsghdr *c = CMSG_FIRSTHDR(&m);
	c->cmsg_level = SOL_SOCKET;
	c->cmsg_type = SCM_RIGHTS;
	c->cmsg_len = CMSG_LEN(sizeof(int));
	*(int*)CMSG_DATA(c) = sendFd;
	sendmsg(*usfd, &m, 0);
	perror("send ");
}

void send_fd_stream(int* nusfd,int sendFd){
	struct iovec e[1];
	char buff[1024];
	buff[0]='F';
	buff[1]='\0';
	e[0].iov_base=buff;
	e[0].iov_len=sizeof buff;
	//struct iovec e={NULL,0};
	char cmsg[CMSG_SPACE(sizeof(int))];
	struct msghdr m = {NULL, 0, e, 1, cmsg, sizeof(cmsg), 0};
	struct cmsghdr *c = CMSG_FIRSTHDR(&m);
	c->cmsg_level = SOL_SOCKET;
	c->cmsg_type = SCM_RIGHTS;
	c->cmsg_len = CMSG_LEN(sizeof(int));
	*(int*)CMSG_DATA(c) = sendFd;
	sendmsg(*nusfd, &m, 0);
	perror("send ");
}

int main(){
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(PORT);
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(sfd,(struct sockaddr* )&serveraddr,sizeof serveraddr)==-1){
		perror("bind error");
	}
	listen(sfd,3);
	int nsfd=accept(sfd,NULL,NULL);
	if(nsfd==-1){
		perror("accept ");
	}
	int usfd;
	send_fd_dgram(&usfd,nsfd);
	char buff[1024];
	int sz = recvfrom(usfd,buff,sizeof buff,0,NULL,NULL);
	buff[sz]='\0';
	printf("rcvd - %s",buff);
	//through stream socket - fd passing
	usfd = socket(AF_UNIX,SOCK_STREAM,0);
	struct sockaddr_un addr;
	memset(&addr,0,sizeof addr);
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path,PATH,sizeof addr.sun_path - 1);
	if(remove(PATH)==-1 && errno != ENOENT){
		perror("remove ");
	}
	if(bind(usfd,(struct sockaddr*)&addr,sizeof addr)==-1){
		perror("bind ");
	}
	listen(usfd,1);
	printf("server ready\n");
	fflush(stdout);
	int nusfd=accept(usfd,NULL,NULL);
	perror("accept ");
	send_fd_stream(&nusfd,nsfd);
}
