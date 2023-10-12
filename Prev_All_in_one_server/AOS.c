#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<fcntl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/un.h>
#include<errno.h>

#define PATH "AOSaddr"
#define BASEPORT 8080
#define DEST1 "S1addr"
#define DEST2 "S2addr"
#define DEST3 "S3addr"

int usfd;

void send_fd_dgram(int sendFd,char* DEST){
	
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
	
	int st = sendmsg(usfd, &m, 0);
	if(st<0){
		perror("send ");
	}
}

int main(){

	usfd = socket(AF_UNIX,SOCK_DGRAM,0);
	
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

	int sfds[4];
	struct sockaddr_in serveraddrs[4];
	
	for(int i=0;i<4;i++){
	
		sfds[i]=socket(AF_INET,SOCK_STREAM,0);
		
		int option=1;
		setsockopt(sfds[i],SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&option,sizeof option);
		
		serveraddrs[i].sin_family=AF_INET;
		serveraddrs[i].sin_addr.s_addr=htonl(INADDR_ANY);
		serveraddrs[i].sin_port=htons(BASEPORT+i);
		int st = bind(sfds[i],(struct sockaddr* )&serveraddrs[i],sizeof serveraddrs[i]);
		if(st<0){
			perror("bind ");
		}
		
		listen(sfds[i],3);
	}
	
	struct pollfd pfds[4];
	
	for(int i=0;i<4;i++){
		pfds[i].fd=sfds[i];
		pfds[i].events=POLLIN;
	}
	
	while(1){
		int ret=poll(pfds,4,20);
		if(ret>0){
			for(int i=0;i<4;i++){
				if(pfds[i].revents & POLLIN){
					int nsfd=accept(pfds[i].fd,NULL,0);
					printf("Accepted Client..will be served by server %d\n",i+1);
					if(i==0){
						send_fd_dgram(nsfd,DEST1);
					}else if(i==1){
						
					}else if(i==2){
					
					}
				}
			}
		}
	}
		
}
