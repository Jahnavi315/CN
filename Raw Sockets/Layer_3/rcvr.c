#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<linux/ip.h>
#include<linux/tcp.h>
#include<linux/udp.h>
#include<linux/sockios.h>
#include<sys/ioctl.h>
#include<linux/if_ether.h>
#include<netinet/ether.h>
#include<linux/if_packet.h>

int main(){

	int sfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sfd == -1){
		perror("socket ");
	}
	
	char buff[1024];
	memset(buff,0,sizeof buff);
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(23454);
	addr.sin_addr.s_addr = inet_addr("127.0.6.6");
	
	if(bind(sfd,(struct sockaddr*)&addr,sizeof addr) == -1){
		perror("bind ");
	}
	
	int sz = recvfrom(sfd,buff,sizeof buff,0,NULL,NULL);
	if(sz < 0){
		perror("recvfrom ");
	}else{
		printf("Data : %s\n",buff);
	}
	fflush(stdout);
}	
