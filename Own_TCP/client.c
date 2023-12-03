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

struct mytcphdr{
	uint16_t source_port;        
    	uint16_t dest_port;     
    	uint8_t crf;
    	uint8_t caf;
    	uint8_t final_ack;
    	uint8_t data;     
    	uint16_t ssn;       
    	uint16_t rsn;
};


void printMyTcpHdr(struct mytcphdr* tcp){
	
    	printf("\n********TCP HEADER********\n\n");
    	printf("\t|-Source Port      : %u\n",ntohs(tcp->source_port));
    	printf("\t|-Destination Port : %u\n",ntohs(tcp->dest_port));
    	printf("\t|-CRF : %d \n",(unsigned int)tcp->crf);
    	printf("\t|-CAF : %d \n",(unsigned int)tcp->caf);
    	printf("\t|-Final Acknowledgment : %d \n",(unsigned int)tcp->final_ack);
    	printf("\t|-Data Flag : %d \n",(unsigned int)tcp->data);
    	printf("\t|-Sender Sequence Number : %u\n",ntohs(tcp->ssn));
    	printf("\t|-Receiver Sequence Number : %u\n",ntohs(tcp->rsn));
        
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

void sendPacket(int sfd,int crf,int caf,int fa,int data_flag){

	char buff[1024];
	memset(buff,0,sizeof buff);
	
	int total_len = 0;
	
	struct iphdr* ip = (struct iphdr*)(buff);
	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 0;
	ip->id = htons(10201);
	ip->ttl = 64;
	
	ip->protocol = 30;
	
	ip->saddr = inet_addr("127.0.0.1");
	ip->daddr = inet_addr("127.0.6.30"); 
	total_len += sizeof(struct iphdr);
	
	struct mytcphdr* tcp = (struct mytcphdr*)(buff + total_len);
	tcp->source_port = htons(8888);
	tcp->dest_port = htons(9999);
	tcp->crf = crf;
	tcp->caf = caf;
	tcp->final_ack = fa;
	tcp->data = data_flag;
	tcp->ssn = htons(0);
	tcp->rsn = htons(0);
	total_len += sizeof(struct mytcphdr);
	
	if(data_flag == 1){
		char* msg = "HELLO!";
		char* data = buff + total_len;
		memcpy(data,msg,strlen(msg));
		total_len += strlen(msg);
		tcp->ssn = htons(strlen(msg));
	}
	
	
	ip->tot_len = htons(total_len);
	
	ip->check = 0;//checksum((unsigned short*)(buff + sizeof(struct ethhdr)), (sizeof(struct iphdr)/2));
	
	printIpHdr(ip);
	printMyTcpHdr(tcp);
	
	
	struct sockaddr_in addr;
	memset(&addr,0,sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int sz = sendto(sfd,buff,total_len,0,(struct sockaddr*)&addr,sizeof addr);
	if(sz < 0){
		perror("send ");
	}
}

int myconnect(int sfd){

	int rsfd = socket(AF_INET,SOCK_RAW,30);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(bind(rsfd,(struct sockaddr*)&addr,sizeof addr) == -1){
		perror("bind ");
	}
	
	printf("Sending Server Connect request\n");
	sendPacket(sfd,1,0,0,0);
	
	printf("Waiting for Server to accept\n");
	char buff[1024];
	int sz = recvfrom(rsfd,buff,sizeof buff,0,NULL,NULL);
	if(sz == -1){
		perror("recvfrom ");
	}else{
		printf("Connection Successful\n");
		fflush(stdout);
		struct iphdr* ip;
		ip = (struct iphdr*)buff;
		printIpHdr(ip);
		struct mytcphdr* tcp = (struct mytcphdr*)(buff + ip->ihl*4);
		printMyTcpHdr(tcp);
		
		printf("Sending Acknowledgment\n");
		sendPacket(sfd,0,0,1,0);
	}
	return rsfd;
		
}

void* communicate(int ssfd,int rsfd){
	while(1){
	char buff[1024];
	sendPacket(ssfd,0,0,0,1);
	int sz = recvfrom(rsfd,buff,sizeof buff,0,NULL,NULL);
	if(sz == -1){
		perror("recvfrom ");
	}else{
		struct iphdr* ip = (struct iphdr*)buff;
		printIpHdr(ip);
		struct mytcphdr* tcp = (struct mytcphdr*)(buff + ip->ihl*4);
		printMyTcpHdr(tcp);
	}
	}
}


int main(){
	int ssfd = socket(AF_INET,SOCK_RAW,IPPROTO_RAW);
	if(ssfd == -1){
		perror("socket ");
	}
	int rsfd = myconnect(ssfd);
	communicate(ssfd,rsfd);
	
}
