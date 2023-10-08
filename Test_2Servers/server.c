#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<fcntl.h>
#include<sys/poll.h>
#include<sys/wait.h>
#include<sys/time.h>
#define PORT 7098
char buff[1024];
char* msg="From server\n";

int main(){
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(PORT);
	serveraddr.sin_addr.s_addr=INADDR_ANY;
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	int option=1;
	setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&option,sizeof option);
	if(bind(sfd,(struct sockaddr*)&serveraddr,sizeof serveraddr)==-1){
		perror("bind ");
	}
	listen(sfd,2);
	while(1){
		getchar();
		int nsfd=accept(sfd,NULL,NULL);
		printf("Accepted\n");
		int sz = recv(nsfd,buff,sizeof buff,0);
		perror("recv ");
		buff[sz]='\0';
		printf("rcvd - %s ",buff);
		send(nsfd,msg,strlen(msg),0);
	}
}
