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
#include<errno.h>
#include<sys/un.h>

#define MAIN "AOSaddr"
#define PATH "S1addr"

#define MSG "Msg from S1\n"

int nsfds[100];
int clients_index = -1;
int usfd;

void* sender(void* args){
	int* nsfd = (int*)args;
	char buffer[1024];
	while(1){
		int sz = send(*nsfd,MSG,strlen(MSG),0);
		if(sz < 0){
			perror("send ");
		}
		sleep(2);
	}
}

void* receiver(void* args){
	int* nsfd = (int*)args;
	char buffer[1024];
	while(1){
		int sz = recv(*nsfd,buffer,sizeof buffer,0);
		if(sz > 0){
			buffer[sz]='\0';
			printf("rcvd through %i : %s",*nsfd,buffer);
		}
	}
}	

void* recv_fd_dgram(void* args){
	char buf[512];
	struct iovec e = {buf, 512};
	char cmsg[CMSG_SPACE(sizeof(int))];
	
	struct msghdr m = {NULL, 0, &e, 1, cmsg, sizeof(cmsg), 0};
	
	while(1){
		int n = recvmsg(usfd, &m, 0);
		perror("Receive");
		clients_index++;
		
		struct cmsghdr *c = CMSG_FIRSTHDR(&m);
		nsfds[clients_index]= *(int*)CMSG_DATA(c);
		printf("recvFd is %i \n",nsfds[clients_index]);
		fflush(stdout);
		
		pthread_t ptd[2];
		pthread_create(&ptd[0],NULL,sender,&nsfds[clients_index]);
		pthread_create(&ptd[1],NULL,receiver,&nsfds[clients_index]);
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
	
	pthread_t ptd;
	pthread_create(&ptd,NULL,recv_fd_dgram,NULL);
	
	
	pthread_join(ptd,NULL);
}
