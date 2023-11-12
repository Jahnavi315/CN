#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<string.h>

int main(){
	int protocol;
	scanf("%i",&protocol);
	//int sfd = socket(AF_INET,SOCK_RAW,protocol);
	int sfd = socket(AF_INET,SOCK_DGRAM,17);
	if(sfd == -1){
		perror("socket ");
	}
	struct sockaddr_in destaddr;
	destaddr.sin_family = AF_INET;
	inet_pton(AF_INET,"127.0.1.1",&destaddr.sin_addr);
	destaddr.sin_port = htons(8079);
	
	struct sockaddr_in myAddr;
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(8089);
	myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sfd,(struct sockaddr*)&myAddr,sizeof myAddr) == -1){
		perror("bind ");
	}
	char* msg = "hopes";
	int st = sendto(sfd,msg,strlen(msg),0,(struct sockaddr*)&destaddr,sizeof destaddr);
	if(st == -1){
		perror("Send ");
	}
	while(1){}
}
