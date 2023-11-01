#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<string.h>

int main(){
	int sfd = socket(AF_INET,SOCK_RAW,25);
	if(sfd == -1){
		perror("socket ");
	}
	struct sockaddr_in destaddr;
	destaddr.sin_family = AF_INET;
	inet_pton(AF_INET,"127.0.0.1",&destaddr.sin_addr);
	char* msg = "Hope I learn raw sockets ASAP!";
	int st = sendto(sfd,msg,strlen(msg),0,(struct sockaddr*)&destaddr,sizeof destaddr);
	if(st == -1){
		perror("Send ");
	}
	while(1){}
}
