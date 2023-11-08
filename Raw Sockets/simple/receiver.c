#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<string.h>
#include<linux/ip.h>
#include<linux/tcp.h>
#include<linux/udp.h>

void printUdpHdr(struct udphdr* udp,char* buff,int iphdrlen){

	printf("\n************UDP HEADER************\n");
	printf("\t|-Source Port : %d\n" , ntohs(udp->source));
	printf("\t|-Destination Port : %d\n", ntohs(udp->dest));
	printf("\t|-UDP Length : %d\n" , ntohs(udp->len));
	printf("\t|-UDP Checksum : %d\n" , ntohs(udp->check));
	char* data = buff + iphdrlen + (sizeof (struct udphdr));
	printf("\t|-Data Rcvd : %s\n",data);
}

void printIpHdr(struct iphdr* ip,char* buff){

	printf("\n************IP HEADER************\n");
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
	if((unsigned int)ip->protocol == 17){
		struct udphdr* udp;
		int iphdrlen = (((unsigned int)(ip->ihl))*4);
		udp = (struct udphdr*)(buff + iphdrlen);
		printUdpHdr(udp,buff,iphdrlen);
	}else{
		char* data = buff + (((unsigned int)(ip->ihl))*4);
		//must not use ip to get data pointer
		printf("\t|-Data Rcvd : %s\n",data);
	}
	
}

int main(){
	int protocol;
	scanf("%i",&protocol);
	int sfd = socket(AF_INET,SOCK_RAW,protocol);
	if(sfd == -1){
		perror("socket ");
	}
	char buff[65536];
	memset(buff,0,sizeof buff);
	int sz = recvfrom(sfd,buff,sizeof buff,0,NULL,NULL);
	if(sz == -1){
		perror("recvfrom ");
	}
	else{
		printf("\t|rcvd %d bytes\n",sz);
		fflush(stdout);
		struct iphdr* ip;
		ip = (struct iphdr*)buff;
		printIpHdr(ip,buff);
	}
	
	while(1){}
}
