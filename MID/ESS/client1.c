#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<fcntl.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<signal.h>
#define PORT 8090

struct shm{
	pid_t agreement[100];
	int curr;
	int rear;
	int isAccessing;
};

struct shm *shmptr;
int sfd;
int shmid;
int stop;

void avoidBlocking(int fd){
	int flags = fcntl(fd,F_GETFL,0);
	flags |= O_NONBLOCK;
	fcntl(fd,F_SETFL,flags);
}

void initaliseShm(){
	system("touch sh.txt");
	key_t key=ftok("sh.txt",'B');
	shmid=shmget(key,sizeof(struct shm),0644|IPC_CREAT);	
	shmptr=shmat(shmid,NULL,0);
}

int getSfd(int isConnected){
	int sockfd;
	if(isConnected){
		sockfd=socket(AF_INET,SOCK_STREAM,0);
	}else{
		sockfd=socket(AF_INET,SOCK_DGRAM,0);
	}
	if(sockfd<0){
		perror("socket");
	}
	return sockfd;
}

int connectSfd(int* sfd){
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(PORT);
	int is=inet_pton(AF_INET,"127.0.0.1",&serveraddr.sin_addr);
	if(is<=0){
		printf("inet error\n");
	}
	int c=connect(*sfd,(struct sockaddr* )&serveraddr,sizeof serveraddr);
	return c;
}

void* sender(void* args){
	char buffer[1024];
	while(1){
		if(stop){// if client wants to end or when someother client is requesting
			send(sfd,"END\n",4,0);
			break;
		}
		int sz = read(0,buffer,sizeof buffer);
		if(sz>0){
			buffer[sz] = '\0';
			if(!strcmp(buffer,"END\n")){//if client wants to end
				kill(getpid(),SIGUSR2);
			}else{
				send(sfd,buffer,sz,0);
			}
		}
	}
}

void* receiver(void* args){
	char buffer[1024];
	while(1){
		if(stop){
			break;
		}
		int sz = recv(sfd,buffer,sizeof buffer,0);
		if(sz>0){
			buffer[sz]='\0';
			printf("rcvd - %s",buffer);
			fflush(stdout);
		}
	}
}

void connectToServer(int signo){
	sfd=getSfd(1);
	int status = connectSfd(&sfd);
	stop=0;
	if(status != -1){
		printf("Connected to server\n");
		pthread_t ptd[2];
		pthread_create(&ptd[0],NULL,sender,NULL);
		pthread_create(&ptd[1],NULL,receiver,NULL);
		pthread_join(ptd[0],NULL);
		pthread_join(ptd[1],NULL);
		fflush(stdout);
	}else{
		printf("connection failed\n");
	}
}

void disconnectFromServer(int signo){
	stop=1;
	while(shmptr->isAccessing){}
	shmptr->isAccessing=1;
	shmptr->curr = (shmptr->curr + 1)%(shmptr->rear + 1);
	shmptr->isAccessing=0;
	sleep(2);
	kill(shmptr->agreement[shmptr->curr],SIGUSR1);
	printf("Disconnected from server\n");
}

void printQueue(){
	int size=shmptr->rear + 1;
	int ptr=shmptr->curr;
	int rear=size-1;
	for(int i=0;i<size;i++){
		printf("%i ",shmptr->agreement[ptr]);
		ptr=(ptr+1)%size;
	}
}

int main(){
	avoidBlocking(0);
	initaliseShm();
	signal(SIGUSR1,connectToServer);
	signal(SIGUSR2,disconnectFromServer);
	shmptr->isAccessing=1;
	shmptr->curr = 0;
	shmptr->rear=0;
	pid_t pid=getpid();
	shmptr->agreement[shmptr->rear]=pid;
	shmptr->isAccessing=0;
	printQueue();
	printf("\n");
	sfd=getSfd(1);
	int status = connectSfd(&sfd);
	stop=0;
	if(status != -1){
		printf("connected to server\n");
		pthread_t ptd[2];
		pthread_create(&ptd[0],NULL,sender,NULL);
		pthread_create(&ptd[1],NULL,receiver,NULL);
		pthread_join(ptd[0],NULL);
		pthread_join(ptd[1],NULL);
	}else{
		printf("Error connecting to server\n");
	}
	while(1){}
}
