#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<linux/if_ether.h>
#include<string.h>
#include<linux/ip.h>
#include<linux/tcp.h>
#include<linux/udp.h>
#include<netinet/ether.h>
#include<linux/if_packet.h>
#include<net/if.h>

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
		}else{
			char* data = buff + sizeof(struct ethhdr) + iphdrlen;
			printf("\t|-Data : %s\n",data);
		}
	}
}

int main(){
	int sfd = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_IP));
	if(sfd == -1){
		perror("socket ");
	}
	
	/*struct sockaddr_ll sadr_ll;
	memset(&sadr_ll,0,sizeof sadr_ll);
	sadr_ll.sll_family = AF_PACKET;
	sadr_ll.sll_protocol = htons(ETH_P_IP);
	sadr_ll.sll_ifindex = if_nametoindex("lo");
	
	if(bind(sfd,(struct sockaddr *)&sadr_ll, sizeof(struct sockaddr_ll)) == -1){
		perror("bind ");
    	}
	
	struct sockaddr_ll sa;
	socklen_t sa_len = sizeof(struct sockaddr_ll);
	if (getsockname(sfd, (struct sockaddr *)&sa, &sa_len) == -1) {
		perror("getsockname ");
	}
	printf("Socket fd %d is bound to interface index %d\n", sfd, sa.sll_ifindex);*/
	
	char buff[65536];
	memset(buff,0,sizeof buff);
	while(1){
		int sz = recvfrom(sfd,buff,sizeof buff,0,NULL,NULL);
		if(sz < 0){
			perror("recvfrom ");
		}else{
			printPacket(buff);
		}
	}
}
