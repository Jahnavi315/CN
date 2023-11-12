#include<stdio.h>
#include<sys/socket.h>	//socket()
#include<arpa/inet.h>	//IPPROTO_RAW
#include<string.h>	//memset
#include<net/if.h>	//if_req
#include<linux/ip.h>
#include<linux/tcp.h>
#include<linux/udp.h>
#include<linux/sockios.h>
#include<sys/ioctl.h>
#include<linux/if_ether.h>
#include<netinet/ether.h>
#include<linux/if_packet.h>
#include<sys/wait.h>
#include<unistd.h>

int sfd;

void printUdpHdr(struct udphdr* udp){

	printf("\n********UDP HEADER********\n\n");
	printf("\t|-Source Port : %d\n" , ntohs(udp->source));
	printf("\t|-Destination Port : %d\n", ntohs(udp->dest));
	printf("\t|-UDP Length : %d\n" , ntohs(udp->len));
	printf("\t|-UDP Checksum : %d\n" , ntohs(udp->check));
}

void printTcpHdr(struct tcphdr* tcp){
	
    	printf("\n********TCP HEADER********\n\n");
    	printf("\t|-Source Port      : %u\n",ntohs(tcp->source));
    	printf("\t|-Destination Port : %u\n",ntohs(tcp->dest));
    	printf("\t|-Sequence Number    : %u\n",ntohl(tcp->seq));
    	printf("\t|-Acknowledge Number : %u\n",ntohl(tcp->ack_seq));
    	printf("\t|-Header Length      : %d DWORDS or %d BYTES\n" ,(unsigned int)tcp->doff,(unsigned int)tcp->doff*4);
    	printf("\t\t|-Urgent Flag          : %d\n",(unsigned int)tcp->urg);
    	printf("\t\t|-Acknowledgement Flag : %d\n",(unsigned int)tcp->ack);
    	printf("\t\t|-Push Flag            : %d\n",(unsigned int)tcp->psh);
    	printf("\t\t|-Reset Flag           : %d\n",(unsigned int)tcp->rst);
    	printf("\t\t|-Synchronise Flag     : %d\n",(unsigned int)tcp->syn);
    	printf("\t\t|-Finish Flag          : %d\n",(unsigned int)tcp->fin);
    	printf("\t|-Window         : %d\n",ntohs(tcp->window));
    	printf("\t|-Checksum       : %d\n",ntohs(tcp->check));
    	printf("\t|-Urgent Pointer : %d\n",tcp->urg_ptr);
        
}

void printIpHdr(struct iphdr* ip){

	printf("\n********IP HEADER********\n\n");
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

unsigned short checksum(unsigned short* buff, int _16bitword)
{
	unsigned long sum;
	for(sum=0;_16bitword>0;_16bitword--)
	sum+=htons(*(buff)++);
	sum = ((sum >> 16) + (sum & 0xFFFF));
	sum += (sum>>16);
	return (unsigned short)(~sum);
}

void sendPacket(){

	char buff[1024];
	memset(buff,0,sizeof buff);
	
	int total_len = 0;
	
	struct iphdr* ip = (struct iphdr*)(buff);
	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 0;
	ip->id = htons(10201);
	ip->ttl = 64;
	
	int protocol;
	printf("Enter Protcol : ");
	scanf("%d",&protocol);
	ip->protocol = protocol;
	
	ip->saddr = inet_addr("127.0.0.1");
	ip->daddr = inet_addr("127.0.6.6"); 
	total_len += sizeof(struct iphdr);
	
	struct udphdr* udp;
	
	if(protocol == 17){
	
		udp = (struct udphdr*)(buff + sizeof(struct ethhdr) + sizeof(struct iphdr));
	
		udp->source = htons(23453);
		udp->dest = htons(23454);
		udp->check = 0;
		total_len += sizeof(struct udphdr);
	}
	
	char* msg = "HELLO!";
	char* data = buff + total_len;
	memcpy(data,msg,strlen(msg));
	total_len += strlen(msg);
	
	if(protocol == 17){
		udp->len = htons((total_len - sizeof(struct iphdr) - sizeof(struct ethhdr)));
	}
	
	ip->tot_len = htons(total_len - sizeof(struct ethhdr));
	
	ip->check = checksum((unsigned short*)(buff + sizeof(struct ethhdr)), (sizeof(struct iphdr)/2));
	
	printIpHdr(ip);
	if(protocol == 17){
		printUdpHdr(udp);
	}
	
	while(1){
		struct sockaddr_in addr;
		memset(&addr,0,sizeof addr);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(23454);
		addr.sin_addr.s_addr = inet_addr("127.0.6.6");//htonl(INADDR_ANY);
		int sz = sendto(sfd,buff,total_len,0,(struct sockaddr*)&addr,sizeof addr);
		if(sz < 0){
			perror("send ");
		}else if(sz == total_len){
			printf("success\n");
		}
		sleep(5);
	}
}

int main(){
	sfd = socket(AF_INET,SOCK_RAW,IPPROTO_RAW);
	if(sfd == -1){
		perror("socket ");
	}
	sendPacket();
}
