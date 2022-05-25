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
#include <string.h>

#define SERWER "192.168.56.102"
#define MAX_BUFF 128

int main()
{

struct addrinfo h, *r;
struct sockaddr_in serwer;
socklen_t serwer_addrlen = sizeof(serwer);
serwer.sin_addr.s_addr = inet_addr(SERWER);
serwer.sin_family = PF_INET;
serwer.sin_port = htons(16240);
int pos, s; //identyfikator gniazda

memset(&h, 0, sizeof(struct addrinfo));
h.ai_family=PF_INET;
h.ai_socktype=SOCK_DGRAM;

if(getaddrinfo(SERWER, "16240", &h, &r) !=0){
	printf("ERROR: %s\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

if((s=socket(r->ai_family, r->ai_socktype, r->ai_protocol))==-1){
	printf("ERROR: %s\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

char init[3] = "hi"; //pierwsza wiadomosc do serwera

char message[MAX_BUFF]; 
int message_size;
char re[] = "Re: ";
char response[20];

// "rejestracja klienta na serwerze"
if((pos=sendto(s, init, strlen(init), 0, r->ai_addr, r->ai_addrlen)) == -1)
    {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        exit(-1);
    }

fflush(stdout);
for(;;)
{

if (((message_size=recvfrom(s,message,MAX_BUFF,0,(struct sockaddr *) &serwer, &serwer_addrlen))) == -1)
    {
       printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
       exit(-1);
    }
    else
    {
	message[message_size] = '\0';
	printf("Odebralem: (%s)\n",message);
	fflush(stdout);
    }

	for(int i=0; i<5; i++){
	response[i]=re[i];
	}

	strcat(response, message);

	//send the message
    if (sendto(s, response, strlen(response) , 0 , (struct sockaddr *) &serwer, serwer_addrlen) == -1)
    {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__); 
        exit(-1);
    }


}

freeaddrinfo(r);
close(s); //zamykamy ’gniazdo’

return(0);
}
