#include<stdio.h>
#include<unistd.h>
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

int main(){
	int sfd = socket(AF_INET,SOCL_STREAM,0);
	
	struct sockaddr_in addr;
	addr.sn_family
}
