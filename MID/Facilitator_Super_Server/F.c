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
#define PORT 8098

int sfd;
struct sockaddr_in clientaddrs[100];

int wfd;

void setSockOptions(int* sfd){
	int option=1;
	setsockopt(*sfd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&option,sizeof option);
}

void bindSocket(int* sfd,int cport){
	struct sockaddr_in address;
	address.sin_family=AF_INET;
	address.sin_addr.s_addr=htonl(INADDR_ANY);
	if(cport == 0){
		address.sin_port=htons(PORT);
	}else{
		address.sin_port=htons(cport);
	}
	int st = bind(*sfd,(struct sockaddr* )&address,sizeof address);
	if(st != -1){
		//printf("bind done\n");
	}else{
		perror("bind");
	}
}

int main(){
	char buff[1024];
	//connectionless socket
	int clients=0;
	sfd=socket(AF_INET,SOCK_DGRAM,0);
	bindSocket(&sfd,0);
	setSockOptions(&sfd);
	struct pollfd sfds[1];
	sfds[0].fd=sfd;
	sfds[0].events=POLLIN;
	//sdfi's
	struct pollfd sfdi[100];
	pid_t pids[100];
	int num_of_servers=0;
	wfd=open("socket_info.txt",O_WRONLY|O_TRUNC);
	//input
	struct pollfd input[1];
	input[0].fd=0;
	input[0].events=POLLIN;
	
	int ret;
	int sz;
	while(1){
	
		ret=poll(input,1,13);
		if(ret>0){
			sz = read(0,buff,sizeof buff);
			buff[sz]='\0';
			write(wfd,buff,sz);
			char* msg="Added socket \n";
			for(int i=0;i<clients;i++){
				sendto(sfd,msg,strlen(msg),0,(struct sockaddr*)&clientaddrs[i],sizeof clientaddrs[i]);
				sendto(sfd,buff,sz,0,(struct sockaddr*)&clientaddrs[i],sizeof clientaddrs[i]);
			}
			int newPort=0;
			int i;
			for(i=0;i<sz;i++){
				if(buff[i]==' '){
					i++;
					break;
				}
				int d=buff[i]-'0';
				newPort=newPort*10 + d;
			}
			char path[20];
			int j=0;
			for(i;i<sz;i++){
				path[j]=buff[i];
				j++;
			}
			path[j-1]='\0';
			sfdi[num_of_servers].fd=socket(AF_INET,SOCK_STREAM,0);
			sfdi[num_of_servers].events=POLLIN;
			setSockOptions(&sfdi[num_of_servers].fd);
			bindSocket(&sfdi[num_of_servers].fd,newPort);
			int c=fork();
			if(c>0){
				pids[num_of_servers]=c;
				num_of_servers++;
			}else{
				dup2(sfdi[num_of_servers].fd,2);
				listen(2,3);
				char* args[]={path,NULL};
				execv(path,args);
				printf("Error in execv\n");
			}
			
		}
		ret=poll(sfds,1,10);
		if(ret>0){
			printf("Client invoked sfd\n");
			int clientaddr_len=sizeof clientaddrs[clients];
			sz = recvfrom(sfd,buff,sizeof buff,0,(struct sockaddr*)&clientaddrs[clients],&clientaddr_len);
			buff[sz]='\0';
			//printf("From client %i - %s \n",clients+1,buff);
			clients++;
			int rfd=open("socket_info.txt",O_RDONLY);
			sz = read(rfd,buff,sizeof buff);
			printf("Read %s",buff);
			int x = sendto(sfd,buff,sz,0,(struct sockaddr*)&clientaddrs[clients-1],sizeof clientaddrs[clients-1]);
			//perror("send to ");
			printf("Sent socket details to client - %d bytes\n",x);
			close(rfd);
		}
		if(num_of_servers){
			ret=poll(sfdi,num_of_servers,23);
			if(ret>0){
				for(int i=0;i<num_of_servers;i++){
					if(sfdi[i].revents & POLLIN){
						fflush(stdout);
						int st = kill(pids[i],SIGUSR1);
						if(st != -1){
							printf("Signalled server %i \n",pids[i]);
						}
					}
				}
			}
		}
		
	}
}
