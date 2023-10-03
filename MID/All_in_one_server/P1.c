#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
int main(){
	char* msg = "Msg from P1\n";
	while(1){
		printf("%s",msg);
		fflush(stdout);
		sleep(10);
	}
}
