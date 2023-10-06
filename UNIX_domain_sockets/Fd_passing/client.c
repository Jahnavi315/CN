#include<sys/socket.h>
#include<sys/un.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#define PORT 8787

int main(){
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(PORT);
	int is=inet_pton(AF_INET,"127.0.0.1",&serveraddr.sin_addr);
	char buff[1024];
	int st = connect(sfd,(struct sockaddr*)&serveraddr,sizeof serveraddr);
	if(st!=-1){
		char* msg = "From client\n";
		int sz=send(sfd,msg,strlen(msg),0);
		if(sz>0){
			perror("send ");
		}
	}
}
