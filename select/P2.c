#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <sys/time.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>

int main(){
	char buff[1024];
	mkfifo("p2com.txt",0666);
	int ffd=open("p2com.txt",O_WRONLY);
	while(1){
		int sz=read(0,buff,sizeof buff);
		if(sz>0){
			buff[sz]='\0';
			write(ffd,buff,sz);
		}
	}
}
