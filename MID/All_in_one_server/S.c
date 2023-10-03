#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<fcntl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>
#define PORT 8090

int sfd;
int nsfds[100];
int from=0;
int size=0;
int isChild=0;

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

void setSockOptions(int* sfd){
	int option=1;
	setsockopt(*sfd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&option,sizeof option);
}

void bindNlisten(int* sfd){
	struct sockaddr_in address;
	address.sin_family=AF_INET;
	address.sin_addr.s_addr=htonl(INADDR_ANY);
	address.sin_port=htons(PORT);
	int st = bind(*sfd,(struct sockaddr* )&address,sizeof address);
	if(st != -1){
		//printf("bind done\n");
	}else{
		perror("bind");
	}
	listen(*sfd,3);
}

void connectEServer(int signo){
	printf("Connecting to Echo Server\n");
	if(from<size){
		int nsfd=nsfds[from];
		from++;
		int c=fork();
		if(c>0){
			//wait(NULL);
		}else{
			isChild++;
			dup2(nsfd,0);
			dup2(nsfd,1);
			char* args[]={"./E",NULL};
			execv("./E",args);
			printf("Error in execv\n");
		}
	}else{
		printf("Invalid request\n");
	}
}

int main(){
	int pp[2];
	pipe(pp);
	int c=fork();
	if(c>0){
		printf("My pid %i \n",getpid());
		//sending pid to p4 and setting handler
		mkfifo("sendpid.txt",0666);
		int tfd=open("sendpid.txt",O_WRONLY);
		char pid[10];
		int p_id=getpid();
		sprintf(pid, "%d", p_id);
		write(tfd,pid,sizeof pid);
		close(tfd);
		signal(SIGUSR1,connectEServer);
		//socket
		sfd=getSfd(1);
		setSockOptions(&sfd);
		bindNlisten(&sfd);
		struct pollfd sockfd[1];
		sockfd[0].fd=sfd;
		sockfd[0].events=POLLIN;
		//ipc
		close(pp[1]);
		mkfifo("p2com.txt",0666);
		int ffd=open("p2com.txt",O_RDONLY);
		FILE* fp=popen("./p3","r");
		if(fp==NULL){
			printf("ERROR in popen\n");
		}
		int fd=fileno(fp);
		struct pollfd pfds[4];
		pfds[0].fd=pp[0];
		pfds[0].events=POLLIN;
		pfds[1].fd=ffd;
		pfds[1].events=POLLIN;
		pfds[2].fd=fd;
		pfds[2].events=POLLIN;
		pfds[3].fd=0;
		pfds[3].events=POLLIN;
		//polling
		char buff[1024];
		while(1){
			if(!isChild){
				int ret;
				ret=poll(sockfd,1,10);
				if(ret>0 && !isChild){
					nsfds[size]=accept(sfd,NULL,0);
					size++;
					printf("Accepted Client\n");
				}
				ret=poll(pfds,4,23);
				if(ret>0 && !isChild){
					for(int i=0;i<4;i++){
						if(pfds[i].revents & POLLIN){
							int sz=read(pfds[i].fd,buff,sizeof buff);
							if(sz>0){
								buff[sz]='\0';
								printf("rcvd - %s",buff);
								for(int i=from;i<size;i++){
									send(nsfds[i],buff,sz,0);
								}
							}
						}
					}
				}
			}
		}
	}else{
		close(pp[0]);
		while(1){
			char* msg = "Msg from p1\n";
			write(pp[1],msg,strlen(msg));
			sleep(10);
		}
	}
}
