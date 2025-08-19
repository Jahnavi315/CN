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

#define PETROL 8090
#define DIESEL PETROL + 1
#define GAS PETROL + 2

#define BPATH "bpath"
#define DPATH "dpath"

struct shm_assign{
	int turn;
	int done;
};

int sfds[3];
struct sockaddr_in addrs[100];
int locations = 0;
sem_t mutex;
int quantity[] = {5,4,5};
int usfd;
struct shm_assign* shmptr;

void send_fd_dgram(int sendFd,char msg[],int size){

	struct iovec e[1];
	char buff[10];
	for(int i=0;i<size;i++){
		buff[i] = msg[i];
	}
	buff[size] = '\0';
	printf("sending fd with buff %s\n",buff);
	
	e[0].iov_base=buff;
	e[0].iov_len=sizeof buff;
	
	char cmsg[CMSG_SPACE(sizeof(int))];
	
	struct sockaddr_un destaddr;
	memset(&destaddr,0,sizeof destaddr);
	destaddr.sun_family=AF_UNIX;
	strncpy(destaddr.sun_path,BPATH,sizeof destaddr.sun_path - 1);
	
	struct msghdr m = {(void*)&destaddr, sizeof(destaddr), e, 1, cmsg, sizeof(cmsg), 0};
	
	struct cmsghdr *c = CMSG_FIRSTHDR(&m);
	c->cmsg_level = SOL_SOCKET;
	c->cmsg_type = SCM_RIGHTS;
	c->cmsg_len = CMSG_LEN(sizeof(int));
	*(int*)CMSG_DATA(c) = sendFd;
	int st = sendmsg(usfd, &m, 0);
	if(st == -1){
		perror("sendmsg ");
	}else{
		printf("sendmsg success\n");
	}
}

void* fuel(void* args){

	int i = *(int*)args;
	int sfd = sfds[i];
	printf("Sfd %d is created\n",sfd);
	
	while(1){
	
		struct sockaddr_in client_addr;
		int size = sizeof client_addr;
		
		int nsfd = accept(sfd,(struct sockaddr*)&client_addr,&size);
		if(nsfd != -1){
			printf("sfd %d accepted a vehicle ",sfd);
		}
		
		int same_location = 0;
		
		char clientIPStr[20];
		inet_ntop(AF_INET,&client_addr.sin_addr,clientIPStr,sizeof clientIPStr);
		
		sem_wait(&mutex);
		
		for(int i=0;i < locations;i++){
			char locations_ip[20];
			inet_ntop(AF_INET,&addrs[i].sin_addr,locations_ip,sizeof locations_ip);
			if(!strcmp(clientIPStr,locations_ip)){
				same_location = 1;
				break;
			}
		}
		if(!same_location){
			printf("from New location %s\n",clientIPStr);
			addrs[locations] = client_addr;
			locations++;
		}else{
			printf("from Same location %s\n",clientIPStr);
		}
		
		sem_post(&mutex);
		
		char msg[10];
		int sz = recv(nsfd,msg,sizeof msg,0);
		if(sz < 0){
			perror("recv ");
		}
		msg[sz] = '\0';
		int req_quantity = atoi(msg);
		
		printf("Requesting sfd : %d - quantity : %d\n",sfd,req_quantity);
		
		
		if(quantity[i] >= req_quantity){
			quantity[i] -= req_quantity;
			send_fd_dgram(usfd,msg,sz);
		}
		
		
	}
}

int main(){
	
	system("touch assign.txt");
	int key = ftok("assign.txt",'B');
	
	int shmid = shmget(key,sizeof (struct shm_assign), 0644| IPC_CREAT);
	shmptr = shmat(shmid,NULL,0);
	
	shmptr->turn = 0;
	shmptr->done = 0;

	sem_init(&mutex,0,1);
	
	pthread_t fuel_threads[3];
	
	int option = 1;
	
	usfd = socket(AF_UNIX,SOCK_DGRAM,0);
	if(usfd == -1){
		perror("usfd socket ");
	}
	
	struct sockaddr_un addr;
	memset(&addr,0,sizeof addr);
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path,DPATH,sizeof addr.sun_path - 1);
	
	if(remove(DPATH) == -1 && errno != ENOENT){
		perror("remove ");
	}
	
	if(bind(usfd,(struct sockaddr*)&addr,sizeof addr) == -1){
		perror("bind ");
	}
	
	for(int i=0;i<3;i++){
	
		sfds[i] = socket(AF_INET,SOCK_STREAM,0);
		if(sfds[i] == -1){
			perror("socket ");
		}
		setsockopt(sfds[i],SOL_SOCKET,SO_REUSEADDR,&option,sizeof option);
		
		struct sockaddr_in myaddr;
		myaddr.sin_family = AF_INET;
		myaddr.sin_port = htons(PETROL + i);
		myaddr.sin_addr.s_addr = inet_addr("127.0.0.100");
		
		if(bind(sfds[i],(struct sockaddr*)&myaddr,sizeof myaddr) == -1){
			perror("bind ");
		}
		
		listen(sfds[i],3);
		
		pthread_create(&fuel_threads[i],NULL,fuel,&i);
		sleep(1);
	}
	
	for(int i=0;i<3;i++){
		pthread_join(fuel_threads[i],NULL);
	}
}
