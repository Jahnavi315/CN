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

void acceptClient(int signo){
	printf("Evaluating request\n");
	int nsfd=accept(2,NULL,0);
	printf("Accepted client");
	fflush(stdout);
	char* msg="thank god\n";
	//while(1){
		int sz = send(2,msg,strlen(msg),0);
		printf("sent %d bytes\n",sz);
		fflush(stdout);
	//}
}

int main(){
	printf("Server pid %i \n",getpid());
	signal(SIGUSR1,acceptClient);
	while(1){}
}