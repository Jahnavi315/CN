#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<fcntl.h>
#include<sys/poll.h>
#include<sys/wait.h>
#include<sys/time.h>

#define PETROL 8090

int main(){
	
	int port;
	char ip[20];
	int choice;
	int option = 1;
	
	printf("Enter port : ");
	scanf("%d",&port);
	
	printf("Enter your IP : ");
	fflush(stdout);
	int sz = read(0,ip,sizeof ip);
	ip[sz-1] = '\0';
	
	printf("Your address %s %i\n",ip,port);
	
	printf("Enter 1 - petrol 2 - diesel 3 - gas : ");
	scanf("%d",&choice);
	
	int sfd = socket(AF_INET,SOCK_STREAM,0);
	if(sfd == -1){
		perror("socket ");
	}
	setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&option,sizeof option);
	
	struct sockaddr_in myaddr;
	
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(port);
	myaddr.sin_addr.s_addr = inet_addr(ip);
	
	if(bind(sfd,(struct sockaddr*)&myaddr,sizeof myaddr) == -1){
		perror("bind ");
	}
	
	
	struct sockaddr_in Daddr;
	Daddr.sin_family = AF_INET;
	Daddr.sin_port = htons(PETROL + choice - 1);
	Daddr.sin_addr.s_addr = inet_addr("127.0.0.100");
	
	if(connect(sfd,(struct sockaddr*)&Daddr,sizeof Daddr) == -1){
		perror("connect ");
	}else{
		printf("Connection Successful\n");
		int quantity;
		printf("Enter quantity of fuel to send : ");
		scanf("%d",&quantity);
		char msg[10];
		snprintf(msg,sizeof msg,"%d",quantity);
		int st = send(sfd,msg,strlen(msg),0);
		if(st == -1){
			perror("send ");
		}
	}
	
	
	while(1){}
}
