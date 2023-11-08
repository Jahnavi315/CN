#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<string.h>
#include<linux/ip.h>
#include<pthread.h>

void printIpHdr(struct iphdr* ip){

	unsigned short iphdrlen;
	struct sockaddr_in source,dest;
	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = ip->saddr;
	memset(&dest, 0, sizeof(dest));
	dest.sin_addr.s_addr = ip->daddr;
	printf("\t|-Version : %d\n" ,(unsigned int)ip->version);
	printf("\t|-Internet Header Length : %d DWORDS or %d Bytes\n ",(unsigned int)ip->ihl,(((unsigned int)(ip->ihl))*4));
	printf("\t|-Type Of Service : %d\n ",(unsigned int)ip->tos);
	printf("\t|-Total Length : %d Bytes\n ",ntohs(ip->tot_len));
	printf("\t|-Identification : %d\n ",ntohs(ip->id));
	printf("\t|-Time To Live : %d\n ",(unsigned int)ip->ttl);
	printf("\t|-Protocol : %d\n ",(unsigned int)ip->protocol);
	printf("\t|-Header Checksum : %d\n ",ntohs(ip->check));
	printf("\t|-Source IP : %s\n ", inet_ntoa(source.sin_addr));
	printf("\t|-Destination IP : %s\n ",inet_ntoa(dest.sin_addr));
	fflush(stdout);

}

void* rcvr(void* args){
	int sfd = *(int*)args;
	printf("Ready to receive on %d\n",sfd);
	char buff[65536];
	while(1){
		memset(buff,0,sizeof buff);
		int sz = recvfrom(sfd,buff,sizeof buff,0,NULL,NULL);
		if(sz == -1){
			perror("recvfrom ");
		}
		else{
			printf("rcvd %d bytes\n",sz);
			struct iphdr* ip;
			ip = (struct iphdr*)buff;
			printf("rcvd IP packet on %i\n",sfd);
			printIpHdr(ip);
		}
	}
	
}

int main(){
	printf("Enter the number of protocols : ");
	int n;
	scanf("%d",&n);
	printf("Enter the protocols now : ");
	int proto[n];
	int sfds[n];
	for(int i = 0;i<n;i++){
		scanf("%d",&proto[i]);
		sfd[i] = socket(AF_INET,SOCK_RAW,proto[i]);
		if(sfd[i] == -1){
			perror("socket ");
		}else{
			pthread_t rcvr_thread;
			pthread_create(&rcvr_thread,NULL,rcvr,&sfd[i]);
		}
		
	}
	
	while(1){}
}
