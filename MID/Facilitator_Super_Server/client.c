#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<fcntl.h>
#define PORT 8098

int sfd;
int csfd;
struct sockaddr_in serveraddr;
struct sockaddr_in cserveraddr;

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

void* dgramReceiver(void* args){
	char buffer[1024];
	while(1){
		int sz = recvfrom(sfd,buffer,sizeof buffer,0,NULL,0);
		if(sz>0){
			buffer[sz]='\0';
			if(sz)
			printf("rcvd - %s",buffer);
			fflush(stdout);
		}
	}
}

void* receiver(void* args){
	char buff[1024];
	while(1){
		int sz=recv(csfd,buff,sizeof buff,0);
		if(sz>0){
		buff[sz]='\0';
		printf("rcvd - %s",buff);
		fflush(stdout);
		}
	}
}

int main(){
	sfd=socket(AF_INET,SOCK_DGRAM,0);
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(PORT);
	int is=inet_pton(AF_INET,"127.0.0.1",&serveraddr.sin_addr);
	printf("Enter any key to get list of services available ");
	fflush(stdout);
	char buff[3];
	char buffer[1024];
	int sz=read(0,buff,sizeof buff);
	if(sz>0){
		buff[sz]='\0';
		int serveraddr_len=sizeof serveraddr;
		sendto(sfd,buff,sz,0,(struct sockaddr*)&serveraddr,serveraddr_len);
		pthread_t rec;
		pthread_create(&rec,NULL,dgramReceiver,NULL);
		printf("Enter the port number ");
		int cport;
		scanf("%i",&cport);
		printf("%i",cport);
		csfd=socket(AF_INET,SOCK_STREAM,0);
		cserveraddr.sin_family=AF_INET;
		cserveraddr.sin_port=htons(cport);
		int is=inet_pton(AF_INET,"127.0.0.1",&cserveraddr.sin_addr);
		int st=connect(csfd,(struct sockaddr*)&cserveraddr,sizeof cserveraddr);
		if(st == -1){
			printf("Connection failed\n");
		}else{
			printf("Connected to server\n");
			pthread_t crcv;
			pthread_create(&crcv,NULL,receiver,NULL);
		}
		while(1){}
		
	}
}
