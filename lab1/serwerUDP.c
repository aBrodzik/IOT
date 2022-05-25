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
#include <stddef.h>


#define MIN_TIME 100
#define MAX_TIME 1080
#define MAX_BUFF 128


int main(){

struct addrinfo h, *r;
struct sockaddr_in c;
int pos, s, c_len = sizeof(c);
unsigned char m[MAX_BUFF];
memset(&h, 0, sizeof(struct addrinfo));

h.ai_family=PF_INET; h.ai_socktype=SOCK_DGRAM; h.ai_flags=AI_PASSIVE;

if(getaddrinfo(NULL,"16240",&h, &r)!=0){ printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
exit(-1);}
srand(time(NULL));
if((s=socket(r->ai_family, r->ai_socktype, r->ai_protocol))==-1){ printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
exit(-1);}
if(bind(s, r->ai_addr, r->ai_addrlen)!=0){ printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
exit(-1);}

uint16_t port;
uint16_t ports[10];

char current_addr[INET_ADDRSTRLEN]; //tymczasowy adres klienta
char clients[10][INET_ADDRSTRLEN]; //tablica 2d wszystkich klientow
int client_counter = 0;

//keep listening for data
for(;;)
{

	if((pos=recvfrom(s, m, MAX_BUFF, 0, (struct sockaddr*)&c, &c_len))<0){
		printf("ERROR: %s\n", strerror(errno));
		exit(-1);
	}
	m[pos]='\0';


	inet_ntop(PF_INET, &(c.sin_addr), current_addr, INET_ADDRSTRLEN);
	port = ntohs(c.sin_port);


	int new_client_flag = 0; // 0 - nowy klient, 1 -znany klient
	for(int i=0; i<client_counter; i++){ 
		char a1[sizeof(current_addr)];
		char a2[sizeof(current_addr)];
		for(int j=0; j<INET_ADDRSTRLEN; j++){
			a1[j]=clients[i][j];
			a2[j]=current_addr[j];
		}
		if(strcmp(a1,a2)==0){
			new_client_flag=1;
			break;
		}
	}

	if(new_client_flag == (int) 0){
		for(int i=0; i<INET_ADDRSTRLEN; i++){
			clients[client_counter][i]=current_addr[i];
		}
	ports[client_counter]=port;
	client_counter = client_counter + 1;
	}
	else{
		printf("%s\n",m);
		fflush(stdout);
	}


	int random_time = ((rand() % (MAX_TIME + 1 - MIN_TIME)) + MIN_TIME); //wynik w ms
	sleep(random_time/1000); //w s

	char random_message[15];
	static const char pool[] = "1234567890aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOqQpPrRsStTuUvVwWxXyYzZ";
	for (int i = 0; i< 20; i++){
		random_message[i]=pool[rand()%strlen(pool)];
	}
	random_message[15]='\0';

	int random_client;
	random_client = rand()%client_counter;

	char random_addr[INET_ADDRSTRLEN];
	for(int i=0; i<INET_ADDRSTRLEN; i++){
		random_addr[i]=clients[random_client][i];
	}

	inet_pton(PF_INET, random_addr, &c.sin_addr.s_addr);
	c.sin_port = htons(ports[random_client]);

	if((pos=sendto(s,random_message,strlen(random_message),0,(struct sockaddr *)&c, c_len ))<0){
		printf("ERROR: %s (%s:%d) \n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}

}
freeaddrinfo(r);
close(s); //zamykamy ’gniazdo’
return (0);

}
