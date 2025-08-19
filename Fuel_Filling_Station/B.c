#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<fcntl.h>
#include<sys/poll.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<errno.h>
#include<sys/un.h>
#include<semaphore.h>
#include<sys/shm.h>
#include<fcntl.h>

#define BPATH "bpath"

struct shm_assign{
	int turn;
	int done;
};

int usfd;
struct shm_assign* shmptr1;
int myno;

void* recv_fd_dgram(void* args){
	
	struct iovec e[1];
	char data[10];
	e[0].iov_base=data;
	e[0].iov_len=sizeof data;
	
	char cmsg[CMSG_SPACE(sizeof(int))];
	
	while(1){
	
		struct msghdr m = {NULL, 0, e, 1, cmsg, sizeof(cmsg), 0};
		int recvFd;
		while(shmptr1->turn != myno){}
		int st = recvmsg(usfd, &m, 0);
		if(st == -1){
			perror("Receive ");
		}
		printf("rcvd buffer : %s\n",(char*)e[0].iov_base);
		
		struct cmsghdr *c = CMSG_FIRSTHDR(&m);
		recvFd = *(int*)CMSG_DATA(c);
		printf("recvFd is %i \n",recvFd);
		fflush(stdout);
		shmptr1->done = 1;
	}
}

int main(){

	system("touch assign.txt");
	int key1 = ftok("assign.txt",'B');
	
	int shmid1 = shmget(key1, sizeof(struct shm_assign),0644| IPC_CREAT);
	shmptr1 = shmat(shmid1,NULL,0);
	
	printf("Enter your Number 1/2 ? ");
	scanf("%d",&myno);
	myno--;
	
	usfd = socket(AF_UNIX,SOCK_DGRAM,0);
	if(usfd == -1){
		perror("usfd socket ");
	}
	
	struct sockaddr_un addr;
	memset(&addr,0,sizeof addr);
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path,BPATH,sizeof addr.sun_path - 1);
	
	if(remove(BPATH) == -1 && errno != ENOENT){
		perror("remove ");
	}
	
	if(bind(usfd,(struct sockaddr*)&addr,sizeof addr) == -1){
		perror("bind ");
	}
	
	pthread_t rcvfd;
	pthread_create(&rcvfd,NULL,recv_fd_dgram,NULL);
	
	while(1){}
}
