#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/un.h>
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
	if(connect(usfd,(struct sockaddr*)&serveraddr,len)==-1){
		perror("connect ");
	}
	int sz;
	while((sz = read(0,buff,sizeof buff))>0){
		buff[sz]='\0';
		send(usfd,buff,sz,0);
	}
	if(sz==-1){
		perror("read ");
	}
	close(usfd);	
}
