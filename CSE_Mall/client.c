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

#define BASEPORT 8000

int sfd; 

void* sender(void* args){
	char buffer[1024];
	while(1){
		int sz=read(0,buffer,sizeof buffer);
		buffer[sz]='\0';
		int st = send(sfd,buffer,sz,0);
		if(st < 0){
			perror("send ");
		}
	}
}

void* receiver(void* args){
	char buffer[100];
	while(1){
		int sz = recv(sfd,buffer,sizeof buffer,0);
		if(sz <0){
			perror("recv ");
		}else if(sz > 0){
			buffer[sz]='\0';
			printf("rcvd - %s\n",buffer);
			fflush(stdout);
		}
	}
}

int main(){

	int sfdd = socket(AF_INET,SOCK_DGRAM,0);

	int gate;
	char ticket_num[10];
	printf("Enter the gate number : ");
	scanf("%d",&gate);
	
	struct sockaddr_in gateaddr;
	gateaddr.sin_family=AF_INET;
	inet_pton(AF_INET,"127.0.0.1",&gateaddr.sin_addr);
	gateaddr.sin_port=htons(BASEPORT+gate-1);
	printf("Enter the Mall : ");
	fflush(stdout);
	char mall[3];
	int sz  = read(0,mall,3);
	
	if(sz != 2){
		printf("Invalid input\n");
		return 1;
	}else{
		mall[sz-1]='\0';
	}
	
	int st = sendto(sfdd,mall,sizeof mall,0,(struct sockaddr*)&gateaddr,sizeof gateaddr);
	if(st < 0){
		perror("sendto ");
	}
	
	char ticketaddr[20];
	sz = recvfrom(sfdd,ticketaddr,sizeof ticketaddr,0,NULL,NULL);
	if(sz < 0){
		perror("recvfrom ");
	}
	else{
		ticketaddr[sz]='\0';
		printf("Ticket %s\n",ticketaddr);
		fflush(stdout);
	}
	int port=0;
	for(int i=0;i<strlen(ticketaddr);i++){
		if(ticketaddr[i]==' '){
			break;
		}
		port=port*10 + (ticketaddr[i]-'0');
	}
	printf("Port is %d\n",port);
	
	sfd = socket(AF_INET,SOCK_STREAM,0);
	
	struct sockaddr_in malladdr;
	malladdr.sin_family=AF_INET;
	inet_pton(AF_INET,"127.0.0.1",&malladdr.sin_addr);
	malladdr.sin_port=htons(port);
	
	st = connect(sfd,(struct sockaddr*)&malladdr,sizeof malladdr);
	if(st < 0){
		perror("connect ");
		return 1;
	}
	else printf("Connected to server\n");
	
	st = send(sfd,ticketaddr,strlen(ticketaddr),0);
	if(st<0){
		perror("send ");
	}
	
	char buff[10];
	
	sz = recv(sfd,buff,sizeof buff,0);
	if(sz < 0){
		perror("recv ");
		
	}
	printf("Verified\n");
	fflush(stdout);
	pthread_t ptd[2];
	pthread_create(&ptd[0],NULL,sender,NULL);
	pthread_create(&ptd[1],NULL,receiver,NULL);
	while(1){}
}
