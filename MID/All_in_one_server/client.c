#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<fcntl.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<signal.h>
#define PORT 8090

int sfd;

int getSfd(int isConnected){
	int sockfd;
	if(isConnected){
		sockfd=socket(AF_INET,SOCK_STREAM,0);
	}else{
		sockfd=socket(AF_INET,SOCK_DGRAM,0);
	}
	if(sockfd<0){
		perror("socket");
	}
	return sockfd;
}

int connectSfd(int* sfd){
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(PORT);
	int is=inet_pton(AF_INET,"127.0.0.1",&serveraddr.sin_addr);
	if(is<=0){
		printf("inet error\n");
	}
	int c=connect(*sfd,(struct sockaddr* )&serveraddr,sizeof serveraddr);
	return c;
}

void* sender(void* args){
	char buffer[1024];
	while(1){
		int sz=read(0,buffer,sizeof buffer);
		if(sz>0){
			buffer[sz]='\0';
			send(sfd,buffer,sz,0);
		}
	}
}

void* receiver(void* args){
	char buffer[1024];
	while(1){
		int sz = recv(sfd,buffer,sizeof buffer,0);
		buffer[sz]='\0';
		if(sz>0){
			printf("rcvd - %s",buffer);
		}
	}
}

int main(){
	sfd=getSfd(1);
	int st=connectSfd(&sfd);
	if(st != -1){
		printf("Connected to server\n");
		pthread_t ptd[2];
		pthread_create(&ptd[0],NULL,sender,NULL);
		pthread_create(&ptd[1],NULL,receiver,NULL);
		pthread_join(ptd[0],NULL);
		pthread_join(ptd[1],NULL);
	}
}
