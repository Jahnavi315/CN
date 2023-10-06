#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<errno.h>
#define PATH "socket2"
#define CLIENTPATH "clientsfd"

int main(){
	char buff[1024];
	int usfd;
	usfd=socket(AF_UNIX,SOCK_DGRAM,0);
	struct sockaddr_un serveraddr;
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sun_family=AF_UNIX;
	strcpy(serveraddr.sun_path,PATH);
	/*struct sockaddr_un clientaddr;
	bzero(&clientaddr,sizeof(clientaddr));
	clientaddr.sun_family=AF_UNIX;
	strcpy(clientaddr.sun_path,CLIENTPATH);
	int len=sizeof clientaddr;
	if(remove(CLIENTPATH) == -1 && errno != ENOENT){
		perror("remove ");
	}
	if(bind(usfd,(struct sockaddr*)&clientaddr,len)==-1){
		perror("bind ");
	}*/
	int sz;
	char c[1024];
	while((sz = read(0,buff,sizeof buff))>0){
		buff[sz]='\0';
		sendto(usfd,buff,sz,0,(struct sockaddr*)&serveraddr,sizeof serveraddr);
		int len=sizeof serveraddr;
		int bytes = recvfrom(usfd,c,sizeof c,0,NULL,NULL);
		c[bytes]='\0';
		printf("rcvd - %s",c);
	}
	printf("got out of loop\n");
	if(sz==-1){
		perror("read ");
	}
	close(usfd);	
}
