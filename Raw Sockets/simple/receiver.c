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
#include<locale.h>
#include<wchar.h>

void printUdpHdr(struct udphdr* udp){

	printf("\nUDP HEADER\n");
	printf("\t|-Source Port : %d\n" , ntohs(udp->source));
	printf("\t|-Destination Port : %d\n", ntohs(udp->dest));
	printf("\t|-UDP Length : %d\n" , ntohs(udp->len));
	printf("\t|-UDP Checksum : %d\n" , ntohs(udp->check));
}

void printTcpHdr(struct tcphdr* tcp){
	
    	printf("\nTCP HEADER\n");
    	printf("\t|-Source Port      : %u\n",ntohs(tcp->source));
    	printf("\t|-Destination Port : %u\n",ntohs(tcp->dest));
    	printf("\t|-Sequence Number    : %u\n",ntohl(tcp->seq));
    	printf("\t|-Acknowledge Number : %u\n",ntohl(tcp->ack_seq));
    	printf("\t|-Header Length      : %d DWORDS or %d BYTES\n" ,(unsigned int)tcp->doff,(unsigned int)tcp->doff*4);
    	printf("\t|-Urgent Flag          : %d\n",(unsigned int)tcp->urg);
    	printf("\t|-Acknowledgement Flag : %d\n",(unsigned int)tcp->ack);
    	printf("\t|-Push Flag            : %d\n",(unsigned int)tcp->psh);
    	printf("\t|-Reset Flag           : %d\n",(unsigned int)tcp->rst);
    	printf("\t|-Synchronise Flag     : %d\n",(unsigned int)tcp->syn);
    	printf("\t|-Finish Flag          : %d\n",(unsigned int)tcp->fin);
    	printf("\t|-Window         : %d\n",ntohs(tcp->window));
    	printf("\t|-Checksum       : %d\n",ntohs(tcp->check));
    	printf("\t|-Urgent Pointer : %d\n",tcp->urg_ptr);
        
}

void printIpHdr(struct iphdr* ip,char* buff){

	printf("\nIP HEADER\n");
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
	
	unsigned int iphdrlen = (((unsigned int)(ip->ihl))*4);
	
	if((unsigned int)ip->protocol == 17){
	
		struct udphdr* udp;
		udp = (struct udphdr*)(buff + iphdrlen);
		printUdpHdr(udp);
		char* data = buff + iphdrlen + sizeof(struct udphdr);
		printf("\t|-Data size : %li\n",strlen(data));
		printf("\t|-Data Rcvd : %s\n",data);
		
	}else if((unsigned int)ip->protocol == 6){
	
		struct tcphdr* tcp;
		tcp = (struct tcphdr*)(buff + iphdrlen);
		printTcpHdr(tcp);
		char* data = buff + iphdrlen + tcp->doff*4;
		printf("\t|-Data size : %li\n",strlen(data));
		printf("\t|-Data Rcvd : %s",data);
		/*for(int i=0;data[i]!='\0';i++){
			printf("%.2x",data[i]);
		}*/
		printf("\n");
		fflush(stdout);
		
	}else{
		char* data = buff + iphdrlen;
		//must not use ip to get data pointer
		printf("\t|-Data Rcvd : %s\n",data);
	}
	
	printf("\n-------------------------------------------------\n");
	
}

int main(){
	
	int protocol;
	scanf("%i",&protocol);
	int sfd = socket(AF_INET,SOCK_RAW,protocol);
	if(sfd == -1){
		perror("socket ");
	}
	char buff[65536];
	while(1){
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
	}
	//while(1){}
}
