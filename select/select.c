#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/select.h>
#include <sys/time.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>

int main(){
	int pp[2];
	char buff[1024];
	pipe(pp);
	int c=0;
	c=fork();
	if(c>0){
		close(pp[1]);
		mkfifo("p2com.txt",0666);
		int maxi=-1;
		int fds[4];
		//fds[0]=dup(0);
		fds[0]=0;
		fds[1]=open("p2com.txt",O_RDONLY);
		FILE* fp=popen("./p3","r");
		if(fp==NULL){
			perror("fp ");
		}else{
			fds[2]=fileno(fp);
		}
		fds[3]=dup(pp[0]);
		for(int i=0;i<4;i++){
			if(maxi<fds[i]){
				maxi=fds[i];
			}
			printf("%i ",fds[i]);
		}
		printf("\nmaxi fd %i\n",maxi);
		fd_set rfds;
		struct timeval tv;
		while(1){
			tv.tv_sec=2;
			tv.tv_usec=0;
			FD_ZERO(&rfds);
			for(int i=0;i<4;i++){
				FD_SET(fds[i],&rfds);
			}
			int ret = select(maxi+1,&rfds,NULL,NULL,&tv);
			if(ret>0){
				printf("data recvd\n");
				for(int i=0;i<4;i++){
					if(FD_ISSET(fds[i],&rfds)){
						int sz = read(fds[i],buff,sizeof buff);
						printf("from %i \n",fds[i]);
						perror("read ");
						if(sz>0){
							buff[sz]='\0';
							printf("rcvd - %s",buff);
						}
					}
				}
			}
		}
		
	}else{
		close(pp[0]);
		dup2(pp[1],1);
		char* args[]={"./p1",NULL};
		execv("./p1",args);
		perror("execv ");
	}
}
