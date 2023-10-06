#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<errno.h>
#define PATH "socket1"
int main(){
	char buff[1024];
	int usfd;
	usfd=socket(AF_UNIX,SOCK_STREAM,0);
	struct sockaddr_un serveraddr;
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sun_family=AF_UNIX;
	strcpy(serveraddr.sun_path,PATH);
	int len=sizeof serveraddr;
	if(remove(PATH) == -1 && errno != ENOENT){
		perror("remove ");
	}
	if(bind(usfd,(struct sockaddr*)&serveraddr,len)==-1){
		perror("bind ");
	}
	listen(usfd,3);
	while(1){
		int nsfd=accept(usfd,NULL,NULL);
		if(nsfd != -1){
			printf("Client accepted\n");
			int sz;
			while((sz = recv(nsfd,buff,sizeof buff,0))>0){
				buff[sz]='\0';
				printf("rcvd - %s",buff);
			}
			if(sz==-1){
				perror("read ");
			}
			close(nsfd);	
		}else{
			perror("accept ");
			break;
		}
		printf("Client disconnected\n");
	}
}
