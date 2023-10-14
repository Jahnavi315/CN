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

#define PATH "TISaddr"

int gates[3];
int num_of_clients=0;
struct sockaddr_in clientaddrs[100];
char* ticketsissued[100];
int usfd;
int stop=0;

void avoidBlocking(int fd){
	int flags = fcntl(fd,F_GETFL,0);
	flags |= O_NONBLOCK;
	fcntl(fd,F_SETFL,flags);
}

void* gate(void* args){
	int* t = (int*)args;
	int gatenum=*t;
	char buff[10];
	avoidBlocking(gates[gatenum]);
	printf("Gate %d established\n",gatenum+1);
	while(1){
		struct sockaddr_in client;
		int len = sizeof client;
		if(stop){
			printf("Gate %i Closed\n",gatenum+1);
			break;
		}
		int sz = recvfrom(gates[gatenum],buff,sizeof buff,0,(struct sockaddr*)&client,&len);
		if(sz < 0){
			//perror("recvfrom ");
		}else{
			printf("Client at gate %d ",gatenum+1);
			buff[sz]='\0';
			printf("requested for %s mall\n",buff);
			
			char ticketaddr[20];
			memset(&ticketaddr,0,sizeof ticketaddr);
			int port;
			if(!strcmp("C",buff)){
				port=CPORT;
			}else if(!strcmp("S",buff)){
				port=SPORT;
			}else{
				port=EPORT;
			}
			snprintf(ticketaddr,sizeof ticketaddr,"%i %i",port,num_of_clients);
			ticketsissued[num_of_clients]=ticketaddr;
			clientaddrs[num_of_clients]=client;
			printf("ticket created %s\n",ticketaddr);
			int st = sendto(gates[gatenum],ticketaddr,sizeof ticketaddr,0,(struct sockaddr*)&client,sizeof client);
			if(st<0){
				perror("sendto ");
			}
			num_of_clients++;			
		}
	}
}

void* verifyTicketandStop(void* args){

	printf("Ticket verifier started\n");
	
	char buff[100];
	struct sockaddr_un client;
	int len = sizeof client;
	while(1){
		int sz = recvfrom(usfd,buff,sizeof buff,0,(struct sockaddr*)&client,&len);
		if(sz < 0){
			perror("recvfrom ");
		}	
		buff[sz]='\0';
		if(!strcmp("STOP",buff)){
			printf("STOP rcvd from E mall\n");
			stop=1;
		}else{
			printf("verification req from client\n");
			fflush(stdout);
			int flag=0;
			for(int i=0;i<num_of_clients;i++){
				if(!strcmp(ticketsissued[i],buff)){
					char* msg="V";
					int st = sendto(usfd,msg,strlen(msg),0,(struct sockaddr*)&client,sizeof client);
					if(st<0){
						perror("sendto ");
					}
					flag=1;
					break;
				}
			}
			if(flag==0){
				char* msg="N";
				int st = sendto(usfd,msg,strlen(msg),0,(struct sockaddr*)&client,sizeof client);
				if(st<0){
					perror("sendto ");
				}
			}
		}
	}
}

int main(){

	usfd=socket(AF_UNIX,SOCK_DGRAM,0);
	struct sockaddr_un myaddr;
	bzero(&myaddr,sizeof(myaddr));
	myaddr.sun_family=AF_UNIX;
	strcpy(myaddr.sun_path,PATH);
	int len=sizeof myaddr;
	if(remove(PATH) == -1 && errno != ENOENT){
		perror("remove ");
	}
	if(bind(usfd,(struct sockaddr*)&myaddr,len)==-1){
		perror("bind ");
	}	
	
	struct sockaddr_in gateaddr[3];
	for(int i=0;i<3;i++){
		gates[i]=socket(AF_INET,SOCK_DGRAM,0);
		int option=1;
		setsockopt(gates[i],SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&option,sizeof option);
		gateaddr[i].sin_family=AF_INET;
		gateaddr[i].sin_addr.s_addr=htonl(INADDR_ANY);
		gateaddr[i].sin_port=htons(BASEPORT+i);
		int st = bind(gates[i],(struct sockaddr* )&gateaddr[i],sizeof gateaddr[i]);
		if(st < 0){
			perror("bind ");
		}
	}
	
	pthread_t gatethreads[3];
	for(int i=0;i<3;i++){
		pthread_create(&gatethreads[i],NULL,gate,&i);
		sleep(1);
	}
	
	pthread_t verify;
	pthread_create(&verify,NULL,verifyTicketandStop,NULL);
	
	
	while(1){}
	
}
