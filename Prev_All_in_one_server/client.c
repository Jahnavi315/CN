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
#define MAIN "AOS"
#define BASEPORT 8080

int sfd;

void* sender(void* args){
	char buffer[1024];
	while(1){
		int sz=read(0,buffer,sizeof buffer);
		buffer[sz]='\0';
		int st = send(sfd,buffer,sz,0);
		if(st < 0 ){
			perror("send ");
		}
	}
}


void* receiver(void* args){
	char buffer[1024];
	while(1){
		int sz = recv(sfd,buffer,sizeof buffer,0);
		if(sz>0){
			buffer[sz]='\0';
			printf("rcvd : %s",buffer);
		}
	}
}

int main(){
	
	sfd = socket(AF_INET,SOCK_STREAM,0);
	
	struct sockaddr_in serveraddr;
	
	int num;
	printf("Enter number between 1 to 4 : ");
	scanf("%d",&num);
	
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serveraddr.sin_port=htons(BASEPORT+num-1);
	
	int st = connect(sfd,(struct sockaddr*)&serveraddr,sizeof serveraddr);
	if(st<0){
		perror("connect ");
	}
	
	pthread_t ptd[2];
	pthread_create(&ptd[0],NULL,sender,NULL);
	pthread_create(&ptd[1],NULL,receiver,NULL);
	
	while(1){}
	pthread_join(ptd[0],NULL);
	pthread_join(ptd[1],NULL);
}
