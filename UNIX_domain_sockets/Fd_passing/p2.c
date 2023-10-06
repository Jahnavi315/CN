#include<sys/socket.h>
#include<sys/un.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#define PATH "p2address"

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
}
