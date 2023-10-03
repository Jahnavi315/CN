#include<stdio.h>
#include<signal.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>

int main(){
	mkfifo("sendpid.txt",0666);
	int fd=open("sendpid.txt",O_RDONLY);
	char buff[10];
	int sz=read(fd,buff,sizeof buff);
	buff[sz]='\0';
	int s_pid=atoi(buff);
	printf("Type Y if you want to signal server %i \n",s_pid);
	while(1){
		int sz=read(0,buff,sizeof buff);
		if(sz>0){
			buff[sz]='\0';
			if(!strcmp(buff,"Y\n")){
				int st = kill(s_pid,SIGUSR1);
				if(st != -1){
					printf("Signalled Server\n");
				}
			}
		}
	}
}
