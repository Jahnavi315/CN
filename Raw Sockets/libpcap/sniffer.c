#include<time.h>
#include<pcap.h>
#include<netinet/in.h>
#include<netinet/if_ether.h>
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

void printArpHdr(struct ether_arp* arpHdr){

    printf("\n********ARP Header********\n");
    printf("\t|-HW Type : %d\n",ntohs(arpHdr->arp_hrd));
    printf("\t|-Protocol : %d\n",ntohs(arpHdr->arp_pro));
    printf("\t|-HW Len : %d\n",arpHdr->arp_hln);
    printf("\t|-Protocol Len : %d\n",arpHdr->arp_pln);
    printf("\t|-OpCode : %d\n",ntohs(arpHdr->arp_op));
    printf("\t|-Source MAC : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",arpHdr->arp_sha[0],arpHdr->arp_sha[1],arpHdr->arp_sha[2],arpHdr->arp_sha[3],arpHdr->arp_sha[4],arpHdr->arp_sha[5]);
    if(ntohs(arpHdr->arp_op) == 2){
    	printf("\t|-Destination MAC : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",arpHdr->arp_tha[0],arpHdr->arp_tha[1],arpHdr->arp_tha[2],arpHdr->arp_tha[3],arpHdr->arp_tha[4],arpHdr->arp_tha[5]);
    }
    printf("\t|-Sender IP: %u.%u.%u.%u\n",arpHdr->arp_spa[0], arpHdr->arp_spa[1], arpHdr->arp_spa[2], arpHdr->arp_spa[3]);
    printf("\t|-Target IP: %u.%u.%u.%u\n", arpHdr->arp_tpa[0], arpHdr->arp_tpa[1], arpHdr->arp_tpa[2], arpHdr->arp_tpa[3]);
}

void printPacket(char* buff){
	printf("\n*******************NETWORK PACKET*******************\n");
	
	struct ethhdr *eth ;
	eth = (struct ethhdr*)buff;
	printf("\n********ETHERNET HEADER********\n\n");
	
	printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
	printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
	printf("\t|-Protocol : %d\n",ntohs(eth->h_proto));
	
	if(ntohs(eth->h_proto) == ETH_P_IP){
	
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
			
		}else if(ip->protocol == 30){
			
			struct mytcphdr* tcp =(struct mytcphdr*)(buff + sizeof(struct ethhdr) + iphdrlen);
			printMyTcpHdr(tcp);
			char* data = buff + sizeof(struct ethhdr) + iphdrlen + sizeof(struct mytcphdr);
			printf("\t|-Data : %s\n",data);
			
		}
	}else if(ntohs(eth->h_proto) == ETH_P_ARP){
		struct ether_arp *arpHdr=(struct ether_arp*)(buff+sizeof(struct ethhdr));
		printArpHdr(arpHdr);
	}
}

void print_packet_info(const u_char *packet, struct pcap_pkthdr packet_header) {

    printf("\n\nPacket capture length : %d\n", packet_header.caplen);
    printf("Packet total length : %d\n", packet_header.len);
    printPacket((char*)packet);;
}

void my_packet_handler(
    u_char *args,
    const struct pcap_pkthdr *packet_header,
    const u_char *packet_body
){
    print_packet_info(packet_body, *packet_header);
    return;
}


int main(int argc, char *argv[]) {
    char *device;
    char error_buffer[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    int timeout_limit = 10000; /* In milliseconds */

    /*device = pcap_lookupdev(error_buffer);
    if (device == NULL) {
        printf("Error finding device: %s\n", error_buffer);
        return 1;
    }*/
    
    device = "lo";

    /* Open device for live capture */
    handle = pcap_open_live(
            device,
            BUFSIZ,
            0,
            timeout_limit,
            error_buffer
        );
    if (handle == NULL) {
         printf("Could not open device %s: %s\n", device, error_buffer);
         return 2;
     }
     
    pcap_loop(handle, -1, my_packet_handler, NULL);

    return 0;
}
