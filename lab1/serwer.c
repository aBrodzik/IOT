#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(){

int s, new_s; //identyfikator gniazda
unsigned char mip_str[INET_ADDRSTRLEN]; 
struct addrinfo h, *r;

memset(&h, 0, sizeof(struct addrinfo));//lepiej wyczyscic(!)
h.ai_family=PF_INET;
h.ai_socktype=SOCK_STREAM;
h.ai_flags=AI_PASSIVE; ////zwracany socket ma byc przygotowany dla operacji ’bind()’

if(getaddrinfo(NULL, "3442", &h, &r)!=0){
	printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

if((s=socket(r->ai_family, r->ai_socktype, r->ai_protocol))==-1){
	printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

if(bind(s, r->ai_addr, r->ai_addrlen)!=0){
	printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

if(listen(s, 1)!=0){
	printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

struct sockaddr_in their_addr;
socklen_t addr_size=sizeof(their_addr);
if((new_s=accept(s, (struct sockaddr *)&their_addr, &addr_size))==-1){
	printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

if(inet_ntop(AF_INET, &(their_addr.sin_addr), mip_str, INET_ADDRSTRLEN)!=NULL){
printf("IP: %s, new sock desc.: %d\n", mip_str, new_s);
}

//wlasciwa komunikacja
#define MAX_BUF 128
void *message[MAX_BUF];

for(;;){
	int result=recv(new_s, message, MAX_BUF, 0);
	if(result==0){
	printf("Peer was disconeted\n");
	break; 
	}
	else if(result<0){
		printf("ERROR: %s\n", strerror(errno));
		exit(-1); 
	}
	message[3]='\0'; 
	printf("Recv(): %s\n", message);
}

close(new_s);
freeaddrinfo(r); //oddajemy systemowi ”strukture addrinfo (r)”
close(s); //zamykamy ’gniazdo’
return 0;
}
