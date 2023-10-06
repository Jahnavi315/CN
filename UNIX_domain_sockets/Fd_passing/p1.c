#include<sys/socket.h>
#include<sys/un.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#define PATH "p1address"
#define DEST "p2address"

int send_fd(int socket, int fd_to_send)
 {
  struct msghdr socket_message;
  struct iovec io_vector[1];
  struct cmsghdr *control_message = NULL;
  char message_buffer[1];
  /* storage space needed for an ancillary element with a paylod of length is CMSG_SPACE(sizeof(length)) */
  char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
  int available_ancillary_element_buffer_space;

  /* at least one vector of one byte must be sent */
  message_buffer[0] = 'F';
  io_vector[0].iov_base = message_buffer;
  io_vector[0].iov_len = 1;

  /* initialize socket message */
  memset(&socket_message, 0, sizeof(struct msghdr));
  socket_message.msg_iov = io_vector;
  socket_message.msg_iovlen = 1;

  /* provide space for the ancillary data */
  available_ancillary_element_buffer_space = CMSG_SPACE(sizeof(int));
  memset(ancillary_element_buffer, 0, available_ancillary_element_buffer_space);
  socket_message.msg_control = ancillary_element_buffer;
  socket_message.msg_controllen = available_ancillary_element_buffer_space;

  /* initialize a single ancillary data element for fd passing */
  control_message = CMSG_FIRSTHDR(&socket_message);
  control_message->cmsg_level = SOL_SOCKET;
  control_message->cmsg_type = SCM_RIGHTS;
  control_message->cmsg_len = CMSG_LEN(sizeof(int));
  *((int *) CMSG_DATA(control_message)) = fd_to_send;

  return sendmsg(socket, &socket_message, 0);
 }

int main(){
	int fd=open("temp.txt",O_RDONLY);
	int usfd=socket(AF_UNIX,SOCK_DGRAM,0);
	struct sockaddr_un addr;
	memset(&addr,0,sizeof addr);
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path,PATH,sizeof addr.sun_path - 1);
	if(remove(PATH)==-1 && errno != ENOENT){
		perror("remove ");
	}
	if(bind(usfd,(struct sockaddr*)&addr,sizeof addr)==-1){
		perror("bind ");
	}
	int st = send_fd(usfd,fd);
	perror("send ");
	printf("status %i ",st);
	/*struct iovec e = {NULL,0};
	 char cmsg[CMSG_SPACE(sizeof(int))];
	struct sockaddr_un destaddr;
	memset(&destaddr,0,sizeof destaddr);
	addr.sun_family=AF_UNIX;
	strncpy(destaddr.sun_path,DEST,sizeof destaddr.sun_path - 1);
	struct msghdr m = {(void*)&destaddr, sizeof(destaddr), &e, 1, cmsg, sizeof(cmsg), 0};
	struct cmsghdr *c = CMSG_FIRSTHDR(&m);
	 c->cmsg_level = SOL_SOCKET;
	 c->cmsg_type = SCM_RIGHTS;
	 c->cmsg_len = CMSG_LEN(sizeof(int));
	 *(int*)CMSG_DATA(c) = fd;
	 sendmsg(usfd, &m, 0);
	 perror("send ");*/
}
