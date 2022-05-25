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
#include <time.h>

int main(){
#define SERWER "192.168.56.102"

int s; //identyfikator gniazda
struct addrinfo h, *r;

memset(&h, 0, sizeof(struct addrinfo));//lepiej wyczyscic(!)
h.ai_family=AF_INET;
h.ai_socktype=SOCK_STREAM;
if(getaddrinfo(SERWER, "3442", &h, &r)!=0){
	printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

if((s=socket(r->ai_family, r->ai_socktype, r->ai_protocol))==-1){
	printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

if(connect(s, r->ai_addr, r->ai_addrlen)!=0){
	printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

#define MAX_BUF 128
int result,counter=0;
char my_hello[MAX_BUF]; 

for(int i=0; i<10; i++){
	time_t t;
	time(&t);
	snprintf(my_hello, MAX_BUF, "%s", ctime(&t));
	result=send(s, my_hello, strlen(my_hello), MSG_NOSIGNAL ); 
	if(result<0){ //jakis bląd podczas wysylania?
	printf("Peer was disconnected (errno: '%s')\n", strerror(errno));break;
	}
	sleep(1.22);
}


freeaddrinfo(r); //oddajemy systemowi ”strukturę addrinfo (r)”
close(s); //zamykamy ’gniazdo’

return(0);
}
