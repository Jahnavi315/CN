#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<fcntl.h>
#define PORT 8090

int sfd,nsfd;
int acceptNext=0;

void avoidBlocking(int fd){
	int flags = fcntl(fd,F_GETFL,0);
	flags |= O_NONBLOCK;
	fcntl(fd,F_SETFL,flags);
}

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

void setSockOptions(int* sfd){
	int option=1;
	setsockopt(*sfd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&option,sizeof option);
}

void bindNlisten(int* sfd){
	struct sockaddr_in address;
	address.sin_family=AF_INET;
	address.sin_addr.s_addr=htonl(INADDR_ANY);
	address.sin_port=htons(PORT);
	int st = bind(*sfd,(struct sockaddr* )&address,sizeof address);
	if(st != -1){
		//printf("bind done\n");
	}else{
		perror("bind");
	}
	listen(*sfd,1);
}

void* sender(void* args){
	char buffer[1024];
	while(1){
		if(acceptNext){
			break;
		}
		int sz = read(0,buffer,sizeof buffer);
		if(sz>0){
			buffer[sz] = '\0';
			send(nsfd,buffer,sz,0);
		}
	}
}

void* receiver(void* args){
	char buffer[1024];
	while(1){
		int sz = recv(nsfd,buffer,sizeof buffer,0);
		buffer[sz]='\0';
		if(!strcmp(buffer,"END\n")){
			acceptNext=1;
			printf("Client session terminated\n");
			fflush(stdout);
			break;
		}
		else if(sz>0){
			printf("rcvd - %s",buffer);
		}
	}
}

int main(){
	avoidBlocking(0);
	struct sockaddr_in clientaddr;
	int clientaddr_len;
	sfd=getSfd(1);
	printf("sfd is %i \n",sfd);
	setSockOptions(&sfd);
	bindNlisten(&sfd);
	acceptNext=1;
	while(1){
		if(acceptNext){
			acceptNext=0;
			nsfd=accept(sfd,(struct sockaddr* )&clientaddr,&clientaddr_len);
			if(nsfd != -1){
				printf("Accepted Client\n");
				pthread_t ptd[2];
				pthread_create(&ptd[0],NULL,sender,NULL);
				pthread_create(&ptd[1],NULL,receiver,NULL);
				pthread_join(ptd[0],NULL);
				pthread_join(ptd[1],NULL);
			}if(nsfd==-1){
				printf("nsfd error\n");
			}
			//close(nsfd);
		}
	}
}
