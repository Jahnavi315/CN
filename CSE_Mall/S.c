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
#define CPORT 8084
#define SPORT 8086
#define EPORT 8087

#define PATH "Saddress"
#define FDADDR "Saddr"
#define DEST "TISaddr"
#define FDDEST "Eaddr"

#define CAPACITY 2

int usfd;
int fd_usfd;
int clients=0;
int nsfds[CAPACITY];


void* sender(void* args){
	int* t=(int*)args;
	int nsfd=*t;
	char* msg="From S ";
	int count=1;
	fflush(stdout);
	while(1){
		sleep(7);
		char text[20];
		snprintf(text,sizeof text,"%s %i",msg,count);
		int st = send(nsfd,text,strlen(text),0);
		if(st<0){
			perror("send ");
		}
		count++;
		sleep(5);
	}
}

void* receiver(void* args){
	int* t=(int*)args;
	int nsfd=*t;
	char buffer[1024];
	while(1){
		int sz = recv(nsfd,buffer,sizeof buffer,0);
		if(sz < 0){
			perror("recv ");
		}else if(sz > 0){
			buffer[sz]='\0';
			printf("rcvd : %s\n",buffer);
			fflush(stdout);
		}
	}
}

void* newScreenings(void* args){
	char buff[100];
	while(1){
		int sz = read(0,buff,sizeof buff);
		buff[sz]='\0';
		char* text = "New Screening - ";
		char msg[200];
		snprintf(msg,sizeof msg,"%s %s",text,buff);
		sleep(1);
		for(int i=0;i<clients && i<CAPACITY;i++){
			int st = send(nsfds[i],msg,strlen(msg),0);
			if(st < 0){
				printf("%i %i client\n",i,nsfds[i]);
				fflush(stdout);
				perror("send ");
			}
		}
	}
}

void send_fd_dgram(int sendFd){

	struct iovec e[1];
	char buff[10];
	buff[0]='S';
	buff[1]='\0';
	e[0].iov_base=buff;
	e[0].iov_len=sizeof buff;
	
	char cmsg[CMSG_SPACE(sizeof(int))];
	
	struct sockaddr_un destaddr;
	memset(&destaddr,0,sizeof destaddr);
	destaddr.sun_family=AF_UNIX;
	strncpy(destaddr.sun_path,FDDEST,sizeof destaddr.sun_path - 1);
	
	struct msghdr m = {(void*)&destaddr, sizeof(destaddr), e, 1, cmsg, sizeof(cmsg), 0};
	
	struct cmsghdr *c = CMSG_FIRSTHDR(&m);
	c->cmsg_level = SOL_SOCKET;
	c->cmsg_type = SCM_RIGHTS;
	c->cmsg_len = CMSG_LEN(sizeof(int));
	*(int*)CMSG_DATA(c) = sendFd;
	sendmsg(usfd, &m, 0);
	perror("send ");
}

void* recv_fd_dgram(void* args){
		
	while(1){
		struct iovec e[1];
		char buff[10];
		e[0].iov_base=buff;
		e[0].iov_len=sizeof buff;
		
		char cmsg[CMSG_SPACE(sizeof(int))];
		
		struct msghdr m = {NULL, 0, e, 1, cmsg, sizeof(cmsg), 0};
		int recvFd;
		int n = recvmsg(fd_usfd, &m, 0);
		perror("Receive");
		printf("rcvd buffer : %s\n",(char*)e[0].iov_base);
		struct cmsghdr *c = CMSG_FIRSTHDR(&m);
		recvFd = *(int*)CMSG_DATA(c);
		printf("recvFd is %i \n",recvFd);
		fflush(stdout);
		//inc client and create threads
		clients++;
		if(clients > CAPACITY){
			send_fd_dgram(recvFd);
		}else{
			nsfds[clients - 1]=dup(recvFd);
			pthread_t ptd[2];
			pthread_create(&ptd[0],NULL,sender,&recvFd);
			pthread_create(&ptd[1],NULL,receiver,&recvFd);
		}
	}
}

int main(){
	
	//creating a unix domain socket
	usfd=socket(AF_UNIX,SOCK_DGRAM,0);
	struct sockaddr_un myaddr;
	bzero(&myaddr,sizeof(myaddr));
	myaddr.sun_family=AF_UNIX;
	strncpy(myaddr.sun_path,PATH,sizeof myaddr.sun_path - 1);
	int len=sizeof myaddr;
	if(remove(PATH) == -1 && errno != ENOENT){
		perror("remove ");
	}
	if(bind(usfd,(struct sockaddr*)&myaddr,len)==-1){
		perror("bind ");
	}
	
	//creating a unix domain socket for sharing fds
	fd_usfd=socket(AF_UNIX,SOCK_DGRAM,0);
	struct sockaddr_un myfdaddr;
	bzero(&myfdaddr,sizeof(myfdaddr));
	myfdaddr.sun_family=AF_UNIX;
	strncpy(myfdaddr.sun_path,FDADDR,sizeof myfdaddr.sun_path - 1);
	len=sizeof myfdaddr;
	if(remove(FDADDR) == -1 && errno != ENOENT){
		perror("remove ");
	}
	if(bind(fd_usfd,(struct sockaddr*)&myfdaddr,len)==-1){
		perror("bind ");
	}
	
	
	//dest TIS address creation
	struct sockaddr_un destaddr;
	bzero(&destaddr,sizeof(destaddr));
	destaddr.sun_family=AF_UNIX;
	strncpy(destaddr.sun_path,DEST,sizeof destaddr.sun_path - 1);

	//creating a stream socket
	int sfd = socket(AF_INET,SOCK_STREAM,0);
	
	struct sockaddr_in address;
	address.sin_family=AF_INET;
	address.sin_addr.s_addr=htonl(INADDR_ANY);
	address.sin_port=htons(SPORT);
	
	int st = bind(sfd,(struct sockaddr* )&address,sizeof address);
	
	listen(sfd,3);
	
	char buff[100];
	
	//starting rev_fd_dgram thread
	pthread_t rcvfd;
	pthread_create(&rcvfd,NULL,recv_fd_dgram,NULL);
	
	//creating thread for sending new screenings
	pthread_t newSc;
	pthread_create(&newSc,NULL,newScreenings,NULL);
	
	while(1){
		int nsfd = accept(sfd,NULL,NULL);
		if(nsfd<0){
			perror("accept ");
		}
		printf("Accepted Client\n");
		int sz = recv(nsfd,buff,sizeof buff,0);
		if(sz<0){
			perror("recv ");
		}
		buff[sz]='\0';
		printf("client sent addr_ticket %s \n",buff);
		fflush(stdout);
		
		st = sendto(usfd,buff,sizeof buff,0,(struct sockaddr*)&destaddr,sizeof destaddr);
		if(st<0){
			perror("sendto ");
		}
		sz = recvfrom(usfd,buff,sizeof buff,0,NULL,NULL);
		if(sz<0){
			perror("recvfrom ");
		}
		buff[sz]='\0';
		printf("Response from TIS - %s \n",buff);
		if(!strcmp("V",buff)){
			clients++;
			if(clients>CAPACITY){
				send_fd_dgram(nsfd);
			}else{
				nsfds[clients - 1]=dup(nsfd);
				st = send(nsfd,buff,strlen(buff),0);
				if(st < 0){
					perror("send ");
				}
				printf("sent ack V to client\n");
				fflush(stdout);
				pthread_t ptd[2];
				pthread_create(&ptd[0],NULL,sender,&nsfd);
				pthread_create(&ptd[1],NULL,receiver,&nsfd);
			}
		}
		
	}
}
