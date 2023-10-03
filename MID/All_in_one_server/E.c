#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <sys/time.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<pthread.h>	

void* sender(void* args){
	printf("In the sender\n");
	char buffer[1024];
	while(1){
		printf("Served by Echo Server\n");
		fflush(stdout);
		sleep(2);
	}
}
void* receiver(void* args){
	char buffer[1024];
	while(1){
		int sz = read(0,buffer,sizeof buffer);
		if(sz>0){
			buffer[sz]='\0';
			printf("rcvd at ES - %s",buffer);
			fflush(stdout);
		}
	}
}

int main(){
	printf("Connected to Echo Server\n");
	fflush(stdout);
	pthread_t ptd[2];
	pthread_create(&ptd[0],NULL,sender,NULL);
	pthread_create(&ptd[1],NULL,receiver,NULL);
	pthread_join(ptd[0],NULL);
	pthread_join(ptd[1],NULL);
}
