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
#include<signal.h>

struct st{
	int fd;
};

void* sender(void* args){
	struct st* temp;
	temp=(struct st*)args;
	int nsfd=temp->fd;
	char* msg="From service Process\n";
	while(1){
		int sz = send(nsfd,msg,strlen(msg),0);
		//printf("sent %d bytes\n",sz);
		//fflush(stdout);
		sleep(2);
	}
}

void acceptClient(int signo){
	printf("Evaluating request\n");
	int nsfd=accept(2,NULL,0);
	printf("Accepted client\n");
	fflush(stdout);
	pthread_t sndr;
	struct st store;
	store.fd=nsfd;
	pthread_create(&sndr,NULL,sender,&store);
}

int main(){
	printf("Server pid %i \n",getpid());
	signal(SIGUSR1,acceptClient);
	while(1){}
}
