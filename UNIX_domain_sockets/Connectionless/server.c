#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<errno.h>
#define PATH "socket2"

int main(){
	char buff[1024];
	int usfd;
	usfd=socket(AF_UNIX,SOCK_DGRAM,0);
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
	struct sockaddr_un clientaddr;
	int clientaddr_len=sizeof clientaddr;
	int sz;
	printf("Waiting for client\n");
	while((sz = recvfrom(usfd,buff,sizeof buff,0,(struct sockaddr*)&clientaddr,&clientaddr_len))>0){
			perror("recvfrom ");
			buff[sz]='\0';
			printf("Got some bytes\n");
			fflush(stdout);
			printf("rcvd - %s",buff);
			fflush(stdout);
			int out = sendto(usfd,buff,sz,0,(struct sockaddr*)&clientaddr,sizeof clientaddr);
			perror("sendto ");
	}
	if(sz==0){
		printf("read 0 bytes so ended\n");
	}
	if(sz==-1){
		perror("read ");
	}
}
