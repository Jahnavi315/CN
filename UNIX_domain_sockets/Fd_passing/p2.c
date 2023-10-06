#include<sys/socket.h>
#include<sys/un.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#define PATH "p2address"
#define DEST "p1address"

int main(){
	int usfd=socket(AF_UNIX,SOCK_DGRAM,0);
	struct sockaddr_un addr;
	memset(&addr,0,sizeof addr);
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path,PATH,sizeof addr.sun_path - 1);
	if(remove(PATH)==-1 && errno != ENOENT){
		perror("remove ");
	}
	if(bind(usfd,(struct sockaddr*)&addr,sizeof addr)==-1){
		perror("bind ");
	}
	char buf[512];
	struct iovec e = {buf, 512};
	char cmsg[CMSG_SPACE(sizeof(int))];
	struct msghdr m = {NULL, 0, &e, 1, cmsg, sizeof(cmsg), 0};
	int n = recvmsg(usfd, &m, 0);
	printf("Receive: %d\n", n);
	struct cmsghdr *c = CMSG_FIRSTHDR(&m);
	int fd = *(int*)CMSG_DATA(c);
	printf("fd is %i",fd);
	char buff[1024];
	int sz = read(fd,buff,sizeof buff);
	perror("read ");
	buff[sz]='\0';
	printf("read %s \n",buff);
	fflush(stdout);
	struct sockaddr_un destaddr;
	memset(&destaddr,0,sizeof destaddr);
	destaddr.sun_family=AF_UNIX;
	strncpy(destaddr.sun_path,DEST,sizeof destaddr.sun_path - 1);
	sendto(usfd,buff,sz,0,(struct sockaddr*)&destaddr,sizeof destaddr);
	perror("sendto ");
}
