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

#define SERWER1 "192.168.56.102"
#define SERWER2 "192.168.56.103"
#define SERWER3 "192.168.56.104"

#define MAX_BUFF 128


int main()
{

struct addrinfo h1,h2,h3, *r1, *r2, *r3;

struct sockaddr_in serwer1;
struct sockaddr_in serwer2;
struct sockaddr_in serwer3;

socklen_t serwer_addrlen1 = sizeof(serwer1);
socklen_t serwer_addrlen2 = sizeof(serwer2);
socklen_t serwer_addrlen3 = sizeof(serwer3);


serwer1.sin_addr.s_addr = inet_addr(SERWER1);
serwer2.sin_addr.s_addr = inet_addr(SERWER2);
serwer3.sin_addr.s_addr = inet_addr(SERWER3);

serwer1.sin_family = PF_INET;
serwer2.sin_family = PF_INET;
serwer3.sin_family = PF_INET;

serwer.sin_port1 = htons(16240);
serwer.sin_port2 = htons(16240);
serwer.sin_port3 = htons(16240);

int pos1, pos2, pos3;
int s1,s2,s3; //identyfikatory gniazda

memset(&h1, 0, sizeof(struct addrinfo));
memset(&h2, 0, sizeof(struct addrinfo));
memset(&h3, 0, sizeof(struct addrinfo));

h1.ai_family=PF_INET;
h2.ai_family=PF_INET;
h3.ai_family=PF_INET;

h1.ai_socktype=SOCK_DGRAM;
h2.ai_socktype=SOCK_DGRAM;
h3.ai_socktype=SOCK_DGRAM;

//--------------------------------------
if(getaddrinfo(SERWER1, "16240", &h1, &r1) !=0){
	printf("ERROR: %s\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

if(getaddrinfo(SERWER2, "16240", &h2, &r2) !=0){
	printf("ERROR: %s\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

if(getaddrinfo(SERWER3, "16240", &h3, &r3) !=0){
	printf("ERROR: %s\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}
//--------------------------------------

if((s1=socket(r1->ai_family, r1->ai_socktype, r1->ai_protocol))==-1){
	printf("ERROR: %s\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

if((s2=socket(r2->ai_family, r2->ai_socktype, r2->ai_protocol))==-1){
	printf("ERROR: %s\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}

if((s3=socket(r3->ai_family, r3->ai_socktype, r3->ai_protocol))==-1){
	printf("ERROR: %s\n", strerror(errno), __FILE__, __LINE__);
	exit(-1);
}
//--------------------------------------

char message[MAX_BUFF]; 
int message_size;

fd_set readfds;
struct timeval tv;
tv.tv_sec=0;
tv.tv_usec=5000;
int r;

int time1= 3080, time2= 5460, time3 = 5960;
int ready1=0,ready2=0,ready3=0; //gotowosc serwera 0-N, 1-Y
fflush(stdout);

char req[1] = "1";

for(;;)
{
	
	
	//sprawdzanie czy serwer1 jest gotowy
	if(ready1== (int) 0){
		FD_ZERO(&readfds);
		FD_SET(s1, &readfds);//ustawiamy jeden bit związany z socketem ’s1’
		if((r=select(s1+1, &readfds, NULL, NULL, &tv))<0){
		printf("ERROR: %s\n", strerror(errno));//...
		}
		else if(r>0){ //dostalismy I wiadomosc od serwera
			if(FD_ISSET(s1, &readfds)){//dla formalności: związane z ’s’
					printf("serwer1 jest gotowy \n");
					ready1=(int) 1;
			}
		}
	
	}
	//sprawdzanie czy serwer2 jest gotowy
	if(ready2== (int) 0){
		FD_ZERO(&readfds);
		FD_SET(s2, &readfds);//ustawiamy jeden bit związany z socketem ’s1’
		if((r=select(s2+1, &readfds, NULL, NULL, &tv))<0){
		printf("ERROR: %s\n", strerror(errno));//...
		}
		else if(r>0){ //dostalismy I wiadomosc od serwera
			if(FD_ISSET(s2, &readfds)){//dla formalności: związane z ’s’
				printf("serwer2 jest gotowy \n");
				ready2=(int) 1;
			}
		}
	}
	//sprawdzanie czy serwer3 jest gotowy
	if(ready3== (int) 0){
		FD_ZERO(&readfds);
		FD_SET(s3, &readfds);//ustawiamy jeden bit związany z socketem ’s1’
		if((r=select(s3+1, &readfds, NULL, NULL, &tv))<0){
		printf("ERROR: %s\n", strerror(errno));//...
		}
		else if(r>0){ //dostalismy I wiadomosc od serwera
			if(FD_ISSET(s3, &readfds)){//dla formalności: związane z ’s’
				printf("serwer3 jest gotowy \n");
				ready3=(int) 1;
			}
		}
		
		
	
	}
	
	
	
	//1------------------------------------------
	sleep(time1*1000); 
	if(ready1==(int) 1){
		//send the message
		if (sendto(s1, req, strlen(req) , 0 , (struct sockaddr *) &serwer1, serwer_addrlen1) == -1)
		{
			printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__); 
			exit(-1);
		}
		
		//w8 4 response
		FD_ZERO(&readfds);
		FD_SET(s, &readfds);//ustawiamy jeden bit związany z socketem ’s’
		if((r=select(s+1, &readfds, NULL, NULL, &tv))<0){
			printf("ERROR: %s\n", strerror(errno));//...
		}
		else if(r>0){ //nadeszły dane
			if(FD_ISSET(s1, &readfds)){//dla formalności: związane z ’s’
				if (((message_size=recvfrom(s1,message,MAX_BUFF,0,(struct sockaddr *) &serwer1, &serwer_addrlen1))) == -1){
					   printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
					   exit(-1);
					}
					else
					{
					message[message_size] = '\0';
					printf("Odebralem S1: (%s)\n",message);
					fflush(stdout);
					}
			}
		}
	}
	
	//2------------------------------------------
	sleep(time2*1000-time1*1000); 
	if(ready1==(int) 1){	
	if (sendto(s2, req, strlen(req) , 0 , (struct sockaddr *) &serwer2, serwer_addrlen2) == -1)
		{
			printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__); 
			exit(-1);
		}
		FD_ZERO(&readfds);
		FD_SET(s2, &readfds);//ustawiamy jeden bit związany z socketem ’s’
		if((r=select(s2+1, &readfds, NULL, NULL, &tv))<0){
			printf("ERROR: %s\n", strerror(errno));//...
		}
		else if(r>0){ //nadeszły dane
			if(FD_ISSET(s2, &readfds)){//dla formalności: związane z ’s’
				if (((message_size=recvfrom(s2,message,MAX_BUFF,0,(struct sockaddr *) &serwer2, &serwer_addrlen2))) == -1){
					   printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
					   exit(-1);
					}
					else
					{
					message[message_size] = '\0';
					printf("Odebralem S2: (%s)\n",message);
					fflush(stdout);
					}
			}
		}
	}
	
	//3------------------------------------------
	sleep(time3*1000-time1*1000 - time3*1000);
	if(ready1==(int) 1){	
	if (sendto(s3, req, strlen(req) , 0 , (struct sockaddr *) &serwer3, serwer_addrlen3) == -1)
		{
			printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__); 
			exit(-1);
		}
		FD_ZERO(&readfds);
		FD_SET(s3, &readfds);//ustawiamy jeden bit związany z socketem ’s’
		if((r=select(s3+1, &readfds, NULL, NULL, &tv))<0){
			printf("ERROR: %s\n", strerror(errno));//...
		}
		else if(r>0){ //nadeszły dane
			if(FD_ISSET(s3, &readfds)){//dla formalności: związane z ’s’
				if (((message_size=recvfrom(s3,message,MAX_BUFF,0,(struct sockaddr *) &serwer3, &serwer_addrlen3))) == -1){
					   printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
					   exit(-1);
					}
					else
					{
					message[message_size] = '\0';
					printf("Odebralem S3: (%s)\n",message);
					fflush(stdout);
					}
			}
		}
	}	

}

freeaddrinfo(r);
close(s); //zamykamy ’gniazdo’

return(0);
}
