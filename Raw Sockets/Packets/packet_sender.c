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

#define DEST_MAC "08:00:27:21:58:3e"
#define DEST_IP "10.0.2.255"
#define LOOPB_MAC "00:00:00:00:00:00"
#define LOOPB_IP "127.0.0.11"

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

void printPacket(char* buff){
	printf("\n*******************NETWORK PACKET*******************\n");
	
	struct ethhdr *eth ;
	eth = (struct ethhdr*)buff;
	printf("\n********ETHERNET HEADER********\n\n");
	
	printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
	printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
	printf("\t|-Protocol : %d\n",ntohs(eth->h_proto));
	
	if(ntohs(eth->h_proto) == 0x0800){
	
		struct iphdr* ip = (struct iphdr*)(buff + sizeof(struct ethhdr));
		printIpHdr(ip);
		unsigned int iphdrlen = (ip -> ihl)*4;
		
		if(ip -> protocol == 6){
			struct tcphdr* tcp = (struct tcphdr*)(buff + sizeof(struct ethhdr) + iphdrlen);
			printTcpHdr(tcp);
			
			char* data = buff + sizeof(struct ethhdr) + iphdrlen + (tcp -> doff)*4;
			printf("\t|-Data : %s\n",data);
						
		}else if(ip -> protocol == 17){
		
			struct udphdr* udp = (struct udphdr*)(buff + sizeof(struct ethhdr) + iphdrlen);
			printUdpHdr(udp);
			
			char* data = buff + sizeof(struct ethhdr) + iphdrlen + sizeof(struct udphdr);
			printf("\t|-Data : %s\n",data);
		}
	}
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

void sendPacket(char* buff){

	int total_len = 0;

	struct ifreq if_req1,if_req2,if_req3;
	memset(&if_req1,0,sizeof if_req1);
	memset(&if_req2,0,sizeof if_req2);
	memset(&if_req3,0,sizeof if_req3);
	
	char* interface_name = "lo";
	strncpy(if_req1.ifr_name,interface_name,IFNAMSIZ - 1);
	strncpy(if_req2.ifr_name,interface_name,IFNAMSIZ - 1);
	strncpy(if_req3.ifr_name,interface_name,IFNAMSIZ - 1);
	
	if (ioctl(sfd, SIOCGIFINDEX, &if_req1) < 0) {
	    perror("ioctl SIOCGIFINDEX");
	}

	if (ioctl(sfd, SIOCGIFHWADDR, &if_req2) < 0) {
	    perror("ioctl SIOCGIFHWADDR");
	}

	if (ioctl(sfd, SIOCGIFADDR, &if_req3) < 0) {
	    perror("ioctl SIOCGIFADDR");
	}
	
	printf("Index of %s : %d\n",interface_name,if_req1.ifr_ifindex);
	printf("Hardware Address of %s : ",interface_name);
	for(int i=0;i<6;i++){
		printf("%.2X",if_req2.ifr_hwaddr.sa_data[i]);
		if(i<5){
			printf("-");
		}
	}
	printf("\n");
	
	printf("IP Address of %s : %s\n ",interface_name,inet_ntoa((((struct sockaddr_in *)&(if_req3.ifr_addr))->sin_addr)));
	
	//ethernet hdr
	
	struct ethhdr* eth = (struct ethhdr*)(buff);
	
	for(int i=0;i<6;i++){
		eth->h_source[i] = (unsigned char)(if_req2.ifr_hwaddr.sa_data[i]);
	}
	
	memcpy(eth->h_dest,ether_aton("00:00:00:00:00:00"),6);
	
	eth->h_proto = htons(ETH_P_IP);
	
	total_len += sizeof(struct ethhdr);
	
	//ip hdr
	struct iphdr* ip = (struct iphdr*)(buff + sizeof(struct ethhdr));
	
	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 16;
	ip->id = htons(10201);
	ip->ttl = 64;
	ip->protocol = 17;
	ip->saddr = inet_addr(inet_ntoa((((struct sockaddr_in *)&(if_req3.ifr_addr))->sin_addr)));
	ip->daddr = inet_addr("127.0.0.11"); 
	total_len += sizeof(struct iphdr);
	
	//udp hdr
	struct udphdr* udp = (struct udphdr*)(buff + sizeof(struct ethhdr) + sizeof(struct iphdr));
	
	udp->source = htons(23451);
	udp->dest = htons(23452);
	udp->check = 0;
	total_len += sizeof(struct udphdr);
	
	//data field
	char* msg = "HEYY!";
	char* data = buff + total_len;
	memcpy(data,msg,strlen(msg));
	
	total_len += strlen(msg);
	
	udp->len = htons((total_len - sizeof(struct iphdr) - sizeof(struct ethhdr)));
	ip->tot_len = htons(total_len - sizeof(struct ethhdr));
	
	ip->check = checksum((unsigned short*)(buff + sizeof(struct ethhdr)), (sizeof(struct iphdr)/2));
	
	printPacket(buff);
	
	struct sockaddr_ll sadr_ll;
	memset(&sadr_ll,0,sizeof sadr_ll);
	sadr_ll.sll_family = AF_PACKET;
	sadr_ll.sll_protocol = htons(ETH_P_IP);
	sadr_ll.sll_ifindex = if_req1.ifr_ifindex;
	
	if(bind(sfd,(struct sockaddr *)&sadr_ll, sizeof(struct sockaddr_ll)) == -1){
		perror("bind ");
    	}
		
	int sz = send(sfd,buff,strlen(buff),0);
	if(sz < 0){
		perror("send ");
	}
	
}

int main(){
	sfd = socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW);
	if(sfd == -1){
		perror("socket" );
	}
	char buff[1024];
	memset(buff,0,sizeof buff);
	
	sendPacket(buff);
}
