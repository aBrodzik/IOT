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


#define UDP_PORT 16240
#define UDP_REMOTE_PORT 16240
#define MAX_BUF 29 //rozmiar buffora
#define SMART_FARM "192.168.56.107" //ip lokalne
#define ARDUINO "192.168.56.106" // ip drugiej maszyny
                          //typ  lokal  obie on/of  1-on 0-off
#define OPEN "100000001" // 10 - 000 - 000- 1
#define CLOSE "100000000"


/*
    TO DO:

*/
//pow(2,i)
short my_pow(short base, short power){
  short result=1;
  while(power != 0){
  result = result * base;
  power=power-1;
  }
  return result;

}


typedef struct rule{
    short actual_value;
    short greater;
    short equal;
    short lesser; //w less nie lesser, ale juz niech tak zostanie
    char* action_greater;
    char* action_equal;
    char* action_lesser;
}rule;

struct node{
    char node_name[15];
    //uint32_t node ip; (nie potrzebujemy bo w demie będziemy robić aruduino na jednej VMce
    //unsigned short int node_port;
    union{
        struct sensor_type_1{ //meter
            rule temperature;
            rule humidity;
        }sensor_type_1;

        struct sensor_type_2{ //osprinkler
            rule waterFlow;
            rule angle;
        }sensor_type_2;

        struct sensor_type_3{ //waterTank
            rule capacity;
            rule water_clarity;
        }sensor_type_3;

        struct state_type_1{
            short on_off;
        }state_type_1;

        struct actuator_type_1{
            short open_close;
        }actuator_type_1;
    }u_node;
}Nodes[3];

int main(int argc, char *argv[])
{

    /*szybka sciaga:
    Nodes[0] = weatherStation
    Nodes[1] = irrigation
    Nodes[2] = waterTankPlant
    sensor_type_1 - meter (czyli "czujnik pogody")
    sensor_type_2 - sprinkler
    sensor_type_3 - water tank

    Nasze demo zaklada ze jest 1 obiekt w kazdej lokacji
    wiec lokacja jest rownowazna wezlu, nie trzeba
    sprawdzac 2 razy lokacj i obiektu, wystarczy raz
    sprawdzic lokacje i wiemy jakie jest w niej obiekt
    */


    //przypisanie każdej wartości reguly = -1
    // -1 czyli ten parametr nie ma odpowiedniej reguly
    // jezeli ta wartosc zostanie nadpisana (przy czytaniu rule file) to regula sie sprawdzi i ew. wykona
    Nodes[0].u_node.sensor_type_1.humidity.greater=-1;
    Nodes[0].u_node.sensor_type_1.humidity.equal=-1;
    Nodes[0].u_node.sensor_type_1.humidity.lesser=-1;
    Nodes[0].u_node.sensor_type_1.temperature.greater=-1;
    Nodes[0].u_node.sensor_type_1.temperature.equal=-1;
    Nodes[0].u_node.sensor_type_1.temperature.lesser=-1;
    Nodes[1].u_node.sensor_type_2.angle.greater=-1;
    Nodes[1].u_node.sensor_type_2.angle.equal=-1;
    Nodes[1].u_node.sensor_type_2.angle.lesser=-1;
    Nodes[1].u_node.sensor_type_2.waterFlow.greater=-1;
    Nodes[1].u_node.sensor_type_2.waterFlow.equal=-1;
    Nodes[1].u_node.sensor_type_2.waterFlow.lesser=-1;
    Nodes[2].u_node.sensor_type_3.capacity.greater=-1;
    Nodes[2].u_node.sensor_type_3.capacity.equal=-1;
    Nodes[2].u_node.sensor_type_3.capacity.lesser=-1;
    Nodes[2].u_node.sensor_type_3.water_clarity.greater=-1;
    Nodes[2].u_node.sensor_type_3.water_clarity.equal=-1;
    Nodes[2].u_node.sensor_type_3.water_clarity.lesser=-1;

    printf("Otwieram plik z zasadami\n");
    //otwieranie pliku
    FILE * fp;
    char * lineIF = NULL;
    char * lineTHEN = NULL;
    size_t len_if = 0;
    size_t len_then = 0;
    ssize_t readIF, readTHEN;
    fp = fopen("/home/psir/Brodzik_Adam/projekt/rule.txt", "r");
    if (fp == NULL){
        exit(EXIT_FAILURE);}

    char if_value_short=0;
    char * action;
    //paroswanie pliku po dwie linie IF i THEN
    while ( ((readIF = getline(&lineIF, &len_if, fp)) != -1) && (readTHEN = getline(&lineTHEN, &len_then, fp))){
            int i;
            int j=0;

            //wydobycie liczby z liny IF
            char ifValue[sizeof(lineIF)];
            strcpy (ifValue, lineIF);
            for (i = 0; ifValue[i]; i++)
            {
                if (ifValue[i] >= '0' && ifValue[i] <= '9')
                {
                    ifValue[j] = ifValue[i];
                    j++;
                }
            }
            ifValue[j]='\0'; //tutaj mamy sama liczbe z lini IF w char
            if_value_short = (short)atoi(ifValue); // konwersja char na short

            //sprawdzenie czy akcja ma byc open czy close
            if(strstr(lineTHEN,"aOpen")){
                action=OPEN;
            }
            if(strstr(lineTHEN,"aClose")){
                action=CLOSE;
            }

            //sprawdzanie lini IF
            if(strstr(lineIF, "lIrrigation") != NULL){
                if(strstr(lineIF, "qWaterFlow") != NULL){
                     if(strstr(lineIF, ">") != NULL) {
                        Nodes[1].u_node.sensor_type_2.waterFlow.greater = if_value_short;
                        Nodes[1].u_node.sensor_type_2.waterFlow.action_greater=action;}
                     if(strstr(lineIF, "=") != NULL) {
                        Nodes[1].u_node.sensor_type_2.waterFlow.equal= if_value_short;
                        Nodes[1].u_node.sensor_type_2.waterFlow.action_equal=action;}
                     if(strstr(lineIF, "<") != NULL) {
                        Nodes[1].u_node.sensor_type_2.waterFlow.lesser=if_value_short;
                        Nodes[1].u_node.sensor_type_2.waterFlow.action_lesser=action;}
                }
                if(strstr(lineIF, "qAngle") != NULL){
                    if(strstr(lineIF, ">") != NULL){
                        Nodes[1].u_node.sensor_type_2.angle.greater=if_value_short;
                        Nodes[1].u_node.sensor_type_2.angle.action_greater=action;}
                    if(strstr(lineIF, "=") != NULL){
                        Nodes[1].u_node.sensor_type_2.angle.equal=if_value_short;
                        Nodes[1].u_node.sensor_type_2.angle.action_equal=action;}
                    if(strstr(lineIF, "<") != NULL){
                        Nodes[1].u_node.sensor_type_2.angle.lesser=if_value_short;
                        Nodes[1].u_node.sensor_type_2.angle.action_lesser=action;}
                }
            }
            if(strstr(lineIF, "lWaterTank") != NULL){
                 if(strstr(lineIF, "qCapacity") != NULL) {
                     if(strstr(lineIF, ">") != NULL) {
                        Nodes[2].u_node.sensor_type_3.capacity.greater= if_value_short;
                        Nodes[2].u_node.sensor_type_3.capacity.action_greater= action;}
                     if(strstr(lineIF, "=") != NULL) {
                        Nodes[2].u_node.sensor_type_3.capacity.equal=if_value_short;
                        Nodes[2].u_node.sensor_type_3.capacity.action_equal=action;}
                     if(strstr(lineIF, "<") != NULL) {
                        Nodes[2].u_node.sensor_type_3.capacity.lesser= if_value_short;
                        Nodes[2].u_node.sensor_type_3.capacity.action_lesser= action;}
                }
                 if(strstr(lineIF, "QWaterClarity") != NULL) {
                     if(strstr(lineIF, ">") != NULL) {
                        Nodes[2].u_node.sensor_type_3.water_clarity.greater= if_value_short;
                        Nodes[2].u_node.sensor_type_3.water_clarity.action_greater= action;}
                     if(strstr(lineIF, "=") != NULL) {
                        Nodes[2].u_node.sensor_type_3.water_clarity.equal=if_value_short;
                        Nodes[2].u_node.sensor_type_3.water_clarity.action_equal= action;}
                     if(strstr(lineIF, "<") != NULL) {
                        Nodes[2].u_node.sensor_type_3.water_clarity.lesser= if_value_short;
                        Nodes[2].u_node.sensor_type_3.water_clarity.action_lesser= action;}
                }
             }
            if(strstr(lineIF, "lWeatherStation") != NULL){
                 if(strstr(lineIF, "qTemperature") != NULL) {
                     if(strstr(lineIF, ">") != NULL) {
                        Nodes[0].u_node.sensor_type_1.temperature.greater= if_value_short;
                        Nodes[0].u_node.sensor_type_1.temperature.action_greater= action;}
                     if(strstr(lineIF, "=") != NULL) {
                        Nodes[0].u_node.sensor_type_1.temperature.equal=if_value_short;
                        Nodes[0].u_node.sensor_type_1.temperature.action_equal= action;}
                     if(strstr(lineIF, "<") != NULL) {
                        Nodes[0].u_node.sensor_type_1.temperature.lesser= if_value_short;
                        Nodes[0].u_node.sensor_type_1.temperature.action_lesser=action;}
                }
                 if(strstr(lineIF, "Qhumidity") != NULL) {
                     if(strstr(lineIF, ">") != NULL) {
                        Nodes[0].u_node.sensor_type_1.humidity.greater = if_value_short;
                        Nodes[0].u_node.sensor_type_1.humidity.action_greater = action;}
                     if(strstr(lineIF, "=") != NULL) {
                        Nodes[0].u_node.sensor_type_1.humidity.equal = if_value_short;
                        Nodes[0].u_node.sensor_type_1.humidity.action_equal = action;}
                     if(strstr(lineIF, "<") != NULL) {
                        Nodes[0].u_node.sensor_type_1.humidity.lesser = if_value_short;
                        Nodes[0].u_node.sensor_type_1.humidity.action_lesser = action;}
                }
             }
        }
    //zwolnienie pamieci, od teraz wszystkie dane z rule file sa przechowywane w strukturze Nodes
    fclose(fp);
    if (lineIF){
        free(lineIF);}
     if (lineTHEN){
        free(lineTHEN);}

      printf("Czytanie zasad skonczone\n");
      


     // do tego momentu na pewno dziala, czyli prawidlowo czytamy rule file i zapisujemy go do struktury!

    //connection stutff
    
    //socket nasluchujacy UDP_PORT 16240 SMART_FARM "192.168.56.107" 

    int s;
  	s = socket(PF_INET, SOCK_DGRAM, 0);
	  if (s == -1){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);}

  	struct sockaddr_in smart_farm;
  	socklen_t smart_farm_addr_size = sizeof(smart_farm);
  	smart_farm.sin_addr.s_addr = inet_addr(SMART_FARM);
  	smart_farm.sin_family = PF_INET;
  	smart_farm.sin_port = htons(UDP_PORT);


	if(bind(s, (struct sockaddr *)&smart_farm, sizeof(smart_farm)) != 0)
	{
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}
   //wysylanie
   struct sockaddr_in arduino;
	 socklen_t arduino_addr_size = sizeof(arduino);
   arduino.sin_family = PF_INET;
   arduino.sin_addr.s_addr = inet_addr(ARDUINO);
   arduino.sin_port = htons(UDP_REMOTE_PORT);
  


    //rejestrowanie wezlow

    //


    //odbieranie wiadomosci raportujacej
  char message_buf[MAX_BUF];
	int message_size; //rozmiar odebranej i wysylanej wiadomosci

    char        message_type[]="01"; // nie mozemy "hard size'owac" wymiaru tablicy bo jest problem z pozniejszym prownywaniem
    char        localization[]="000"; // te poczatkowe wartosci i tak beda pozniej nadpisywane
    char        object[]="000"; 
    char        param_1[10];
    char        param_2[10];
    char        state_1[0];
    short       param_1d;
    short       param_2d;
    short       state_value;
    while(1){


		message_size = recvfrom(s, message_buf, MAX_BUF, 0, (struct sockaddr *)&smart_farm, &smart_farm_addr_size);
		if(message_size < 0){
			printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
			exit(-4);
		}
    printf("\n");
        
        // 2bity - typ wiadomości 3 bity lokalizacja 3 bity obiekt 10 bitów warosc 10 bitow wartosc 1 bit stan

        message_buf[MAX_BUF]='\0';
	      //printf("Recv:%s len=%d\n",&message_buf,sizeof(message_buf));     
        
        
        for(int i=0; i<2;i++){
        message_type[i]=message_buf[i];
        }
        message_type[2]='\0';
        
        int j=0;
        for(int i=2; i<5;i++){
          localization[j]=message_buf[i];
          j++;
        }
        localization[3]='\0';
 
        j=0;
        for(int i=5; i<8;i++){
          object[j]=message_buf[i];
          j++;
        }
        object[3]='\0';
        
        j=0;
        for(int i=8; i<18;i++){
          param_1[j]=message_buf[i];
          j++;
        }
        param_1[10]='\0';
        
        j=0;
        for(int i=18; i<28;i++){
          param_2[j]=message_buf[i];
          j++;
        }
        param_2[10]='\0';
        
        state_1[0]=message_buf[28];
       

        //konwersja bint - short
        param_1d=0;
        for(int i=9; i>=0;i--){
           if(param_1[9-i]=='1'){
          param_1d=param_1d+my_pow(2,i);
          }
        }
        
        param_2d=0;
        for(int i=9; i>=0;i--){
          if(param_2[9-i]=='1'){
          param_2d=param_2d+my_pow(2,i);
          }
        }
       
       
        state_value= state_value + state_1[0]; // konwersja char - > short
        
        //char type[] = "01";
        //char irrigation[] = "000";
        //char irrigation[] = "000";
        //char irrigation[] = "000";
        

        //przypisywanie wartosci
        if(strcmp(message_type,"01")==0){ //czy wiadomosc rapor
             if( strcmp(localization,"000")==0){ //czy lokalizacja lIrrigation strcmp(localization, "000" init == 'R', 'I', 'F', 'F'
             printf("Odebrano wiadomosc z lIrrigation\n");
             printf("kat spryskiwacza: %d\n", param_1d); 
             printf("przeplywnosc spryskiwacza: %d\n", param_2d);
                Nodes[1].u_node.sensor_type_2.angle.actual_value= param_1d;
                    if((Nodes[1].u_node.sensor_type_2.angle.actual_value > Nodes[1].u_node.sensor_type_2.angle.greater) //>
                     &&(Nodes[1].u_node.sensor_type_2.angle.greater != -1)){
                       printf("Regula Kat spryskiwacza (%d) wiekszy niz: %d. Wysylam odpowiednia komende (%s).\n",
                         Nodes[1].u_node.sensor_type_2.angle.actual_value, 
                         Nodes[1].u_node.sensor_type_2.angle.greater,
                         Nodes[1].u_node.sensor_type_2.angle.action_greater);
                         
                            if(sendto(s, Nodes[1].u_node.sensor_type_2.angle.action_greater,
                                        strlen(Nodes[1].u_node.sensor_type_2.angle.action_greater), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                    if((Nodes[1].u_node.sensor_type_2.angle.actual_value < Nodes[1].u_node.sensor_type_2.angle.lesser) // <
                     &&(Nodes[1].u_node.sensor_type_2.angle.lesser != -1)){
                         printf("Regula Kat spryskiwacza (%d) mniejszy niz: %d. Wysylam odpowiednia komende (%s).\n",
                         Nodes[1].u_node.sensor_type_2.angle.actual_value,
                          Nodes[1].u_node.sensor_type_2.angle.lesser,
                          Nodes[1].u_node.sensor_type_2.angle.action_lesser); 
                            if(sendto(s, Nodes[1].u_node.sensor_type_2.angle.action_lesser,
                                        strlen(Nodes[1].u_node.sensor_type_2.angle.action_lesser), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                    if((Nodes[1].u_node.sensor_type_2.angle.actual_value == Nodes[1].u_node.sensor_type_2.angle.equal) // =
                     &&(Nodes[1].u_node.sensor_type_2.angle.equal != -1)){
                        printf("Regula Kat spryskiwacza (%d) rowny: %d. Wysylam odpowiednia komende (%s).\n",
                        Nodes[1].u_node.sensor_type_2.angle.actual_value,
                         Nodes[1].u_node.sensor_type_2.angle.equal,
                         Nodes[1].u_node.sensor_type_2.angle.action_equal); 
                            if(sendto(s, Nodes[1].u_node.sensor_type_2.angle.action_equal,
                                        strlen(Nodes[1].u_node.sensor_type_2.angle.action_equal), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                Nodes[1].u_node.sensor_type_2.waterFlow.actual_value= param_2d;
                    if((Nodes[1].u_node.sensor_type_2.waterFlow.actual_value > Nodes[1].u_node.sensor_type_2.waterFlow.greater) //>
                     &&(Nodes[1].u_node.sensor_type_2.waterFlow.greater != -1)){
                        printf("Regula przeplywnosc spryskiwacza (%d) wieksza niz: %d. Wysylam odpowiednia komende (%s).\n",
                        Nodes[1].u_node.sensor_type_2.waterFlow.actual_value,
                        Nodes[1].u_node.sensor_type_2.waterFlow.greater,
                        Nodes[1].u_node.sensor_type_2.waterFlow.action_greater); 
                            if(sendto(s, Nodes[1].u_node.sensor_type_2.waterFlow.action_greater,
                                        strlen(Nodes[1].u_node.sensor_type_2.waterFlow.action_greater), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                    if((Nodes[1].u_node.sensor_type_2.waterFlow.actual_value < Nodes[1].u_node.sensor_type_2.waterFlow.lesser) //<
                     &&(Nodes[1].u_node.sensor_type_2.waterFlow.lesser != -1)){
                        printf("Regula przeplywnosc spryskiwacza (%d) mniejsza niz: %d. Wysylam odpowiednia komende (%s).\n",
                        Nodes[1].u_node.sensor_type_2.waterFlow.actual_value,
                        Nodes[1].u_node.sensor_type_2.waterFlow.lesser,
                        Nodes[1].u_node.sensor_type_2.waterFlow.action_lesser); 
                            if(sendto(s, Nodes[1].u_node.sensor_type_2.waterFlow.action_lesser,
                                        strlen(Nodes[1].u_node.sensor_type_2.waterFlow.action_lesser), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                    if((Nodes[1].u_node.sensor_type_2.waterFlow.actual_value == Nodes[1].u_node.sensor_type_2.waterFlow.equal) //=
                     &&(Nodes[1].u_node.sensor_type_2.waterFlow.equal != -1)){
                        printf("Regula przeplywnosc spryskiwacza (%d) rowna: %d. Wysylam odpowiednia komende (%s).\n",
                        Nodes[1].u_node.sensor_type_2.waterFlow.actual_value,
                        Nodes[1].u_node.sensor_type_2.waterFlow.equal,
                        Nodes[1].u_node.sensor_type_2.waterFlow.action_equal);
                            if(sendto(s, Nodes[1].u_node.sensor_type_2.waterFlow.action_equal,
                                        strlen(Nodes[1].u_node.sensor_type_2.waterFlow.action_equal), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                Nodes[1].u_node.state_type_1.on_off= state_value;
            }
            if( strcmp(localization,"001") == 0 ){ //czy lokalizacja lWaterTank strcmp(localization, "001"
            printf("Odebrano wiadomosc z lWaterTank\n");
            printf("pojemnosc zbiornika: %d\n", param_1d); 
            printf("czystosc wody: %d\n", param_2d);
                Nodes[2].u_node.sensor_type_3.capacity.actual_value= param_1d;
                 if((Nodes[2].u_node.sensor_type_3.capacity.actual_value > Nodes[2].u_node.sensor_type_3.capacity.greater) //>
                     &&(Nodes[2].u_node.sensor_type_3.capacity.greater != -1)){
                       printf("Regula pojemnosc zbiornika (%d) wieksza niz: %d. Wysylam odpowiednia komende (%s).\n",
                       Nodes[2].u_node.sensor_type_3.capacity.actual_value,
                       Nodes[2].u_node.sensor_type_3.capacity.greater,
                       Nodes[2].u_node.sensor_type_3.capacity.action_greater); 
                            if(sendto(s, Nodes[2].u_node.sensor_type_3.capacity.action_greater,
                                        strlen(Nodes[2].u_node.sensor_type_3.capacity.action_greater), 0,
                                        (struct sockaddr *)&arduino,arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                    if((Nodes[2].u_node.sensor_type_3.capacity.actual_value < Nodes[2].u_node.sensor_type_3.capacity.lesser) // <
                     &&(Nodes[2].u_node.sensor_type_3.capacity.lesser != -1)){
                        printf("Regula pojemnosc zbiornika (%d) mniejsza niz: %d. Wysylam odpowiednia komende (%s).\n",
                        Nodes[2].u_node.sensor_type_3.capacity.actual_value,
                        Nodes[2].u_node.sensor_type_3.capacity.lesser,
                        Nodes[2].u_node.sensor_type_3.capacity.action_lesser);  
                            if(sendto(s, Nodes[2].u_node.sensor_type_3.capacity.action_lesser,
                                        strlen(Nodes[2].u_node.sensor_type_3.capacity.action_lesser), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                    if((Nodes[2].u_node.sensor_type_3.capacity.actual_value == Nodes[2].u_node.sensor_type_3.capacity.equal) // =
                     &&(Nodes[2].u_node.sensor_type_3.capacity.equal != -1)){
                        printf("Regula pojemnosc zbiornika (%d) rowna: %d. Wysylam odpowiednia komende (%s).\n",
                        Nodes[2].u_node.sensor_type_3.capacity.actual_value,
                        Nodes[2].u_node.sensor_type_3.capacity.equal,
                        Nodes[2].u_node.sensor_type_3.capacity.action_equal);  
                            if(sendto(s, Nodes[2].u_node.sensor_type_3.capacity.action_equal,
                                        strlen(Nodes[2].u_node.sensor_type_3.capacity.action_equal), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                Nodes[2].u_node.sensor_type_3.water_clarity.actual_value= param_2d;
                 if((Nodes[2].u_node.sensor_type_3.water_clarity.actual_value > Nodes[2].u_node.sensor_type_3.water_clarity.greater) //>
                     &&(Nodes[2].u_node.sensor_type_3.water_clarity.greater != -1)){
                        printf("Regula czystosc wody (%d) wieksza niz: %d. Wysylam odpowiednia komende (%s).\n",
                        Nodes[2].u_node.sensor_type_3.water_clarity.actual_value,
                        Nodes[2].u_node.sensor_type_3.water_clarity.greater,
                        Nodes[2].u_node.sensor_type_3.water_clarity.action_greater);  
                            if(sendto(s, Nodes[2].u_node.sensor_type_3.water_clarity.action_greater,
                                        strlen(Nodes[2].u_node.sensor_type_3.water_clarity.action_greater), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                    if((Nodes[2].u_node.sensor_type_3.water_clarity.actual_value < Nodes[2].u_node.sensor_type_3.water_clarity.lesser) // <
                     &&(Nodes[2].u_node.sensor_type_3.water_clarity.lesser != -1)){
                      printf("Regula czystosc wody (%d) mniejsza niz: %d. Wysylam odpowiednia komende (%s).\n",
                      Nodes[2].u_node.sensor_type_3.water_clarity.actual_value,
                      Nodes[2].u_node.sensor_type_3.water_clarity.lesser,
                      Nodes[2].u_node.sensor_type_3.water_clarity.action_lesser); 
                        if(sendto(s, Nodes[2].u_node.sensor_type_3.water_clarity.action_lesser,
                                        strlen(Nodes[2].u_node.sensor_type_3.water_clarity.action_lesser), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                    if((Nodes[2].u_node.sensor_type_3.water_clarity.actual_value == Nodes[2].u_node.sensor_type_3.water_clarity.equal) // =
                     &&(Nodes[2].u_node.sensor_type_3.water_clarity.equal != -1)){
                     printf("Regula czystosc wody (%d) wieksza niz: %d. Wysylam odpowiednia komende (%s).\n",
                     Nodes[2].u_node.sensor_type_3.water_clarity.actual_value,
                     Nodes[2].u_node.sensor_type_3.water_clarity.equal,
                     Nodes[2].u_node.sensor_type_3.water_clarity.action_equal);
                            if(sendto(s, Nodes[2].u_node.sensor_type_3.water_clarity.action_equal,
                                        strlen(Nodes[2].u_node.sensor_type_3.water_clarity.action_equal), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
            }
            if( strcmp(localization,"010") == 0) { //czy lokalizacja lWeatherStation strcmp(localization, "010"
            printf("Odebrano wiadomosc z lWeatherStation\n");
            printf("temperatura: %d\n", param_1d); 
            printf("wilgotnosc: %d\n", param_2d);
                Nodes[0].u_node.sensor_type_1.temperature.actual_value=param_1d;
                  if((Nodes[0].u_node.sensor_type_1.temperature.actual_value > Nodes[0].u_node.sensor_type_1.temperature.greater) //>
                     &&(Nodes[0].u_node.sensor_type_1.temperature.greater != -1)){
                        printf("Regula Temperatura (%d) wieksza niz: %d. Wysylam odpowiednia komende (%s).\n",
                        Nodes[0].u_node.sensor_type_1.temperature.actual_value, 
                        Nodes[0].u_node.sensor_type_1.temperature.greater,
                        Nodes[0].u_node.sensor_type_1.temperature.action_greater); 
                            if(sendto(s, Nodes[0].u_node.sensor_type_1.temperature.action_greater,
                                        strlen(Nodes[0].u_node.sensor_type_1.temperature.action_greater), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                    if((Nodes[0].u_node.sensor_type_1.temperature.actual_value < Nodes[0].u_node.sensor_type_1.temperature.lesser) // <
                     &&(Nodes[0].u_node.sensor_type_1.temperature.lesser != -1)){
                    printf("Regula Temperatura (%d) mniejsza niz: %d. Wysylam odpowiednia komende (%s).\n",
                     Nodes[0].u_node.sensor_type_1.temperature.actual_value,
                      Nodes[0].u_node.sensor_type_1.temperature.lesser,
                      Nodes[0].u_node.sensor_type_1.temperature.action_lesser); 
                            if(sendto(s, Nodes[0].u_node.sensor_type_1.temperature.action_lesser,
                                        strlen(Nodes[0].u_node.sensor_type_1.temperature.action_lesser), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                    if((Nodes[0].u_node.sensor_type_1.temperature.actual_value == Nodes[0].u_node.sensor_type_1.temperature.equal) // =
                     &&(Nodes[0].u_node.sensor_type_1.temperature.equal != -1)){
                       printf("Regula Temperatura (%d) rowna: %d. Wysylam odpowiednia komende (%s).\n", 
                       Nodes[0].u_node.sensor_type_1.temperature.actual_value,
                        Nodes[0].u_node.sensor_type_1.temperature.equal,
                         Nodes[0].u_node.sensor_type_1.temperature.action_equal); 
                            if(sendto(s, Nodes[0].u_node.sensor_type_1.temperature.action_equal,
                                        strlen(Nodes[0].u_node.sensor_type_1.temperature.action_equal), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                Nodes[0].u_node.sensor_type_1.humidity.actual_value= param_2d;
                  if((Nodes[0].u_node.sensor_type_1.humidity.actual_value > Nodes[0].u_node.sensor_type_1.humidity.greater) //>
                     &&(Nodes[0].u_node.sensor_type_1.humidity.greater != -1)){
                    printf("Regula wilgotnosc (%d) wieksza niz: %d. Wysylam odpowiednia komende (%s).\n",
                    Nodes[0].u_node.sensor_type_1.humidity.actual_value,
                    Nodes[0].u_node.sensor_type_1.humidity.greater,
                    Nodes[0].u_node.sensor_type_1.humidity.action_greater);  
                            if(sendto(s, Nodes[0].u_node.sensor_type_1.humidity.action_greater,
                                        strlen(Nodes[0].u_node.sensor_type_1.humidity.action_greater), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                    if((Nodes[0].u_node.sensor_type_1.humidity.actual_value < Nodes[0].u_node.sensor_type_1.humidity.lesser) // <
                     &&(Nodes[0].u_node.sensor_type_1.humidity.lesser!= -1)){
                     printf("Regula wilgotnosc (%d) mniejsza niz: %d. Wysylam odpowiednia komende (%s).\n",
                     Nodes[0].u_node.sensor_type_1.humidity.actual_value,
                     Nodes[0].u_node.sensor_type_1.humidity.lesser,
                     Nodes[0].u_node.sensor_type_1.humidity.action_lesser); 
                        if(sendto(s, Nodes[0].u_node.sensor_type_1.humidity.action_lesser,
                                        strlen(Nodes[0].u_node.sensor_type_1.humidity.action_lesser), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
                    if((Nodes[0].u_node.sensor_type_1.humidity.actual_value == Nodes[0].u_node.sensor_type_1.humidity.equal) // =
                     &&(Nodes[0].u_node.sensor_type_1.humidity.equal != -1)){
                     printf("Regula wilgotnosc (%d) rowna: %d. Wysylam odpowiednia komende (%s).\n",
                     Nodes[0].u_node.sensor_type_1.humidity.actual_value,
                     Nodes[0].u_node.sensor_type_1.humidity.equal,
                     Nodes[0].u_node.sensor_type_1.humidity.action_equal );
                            if(sendto(s, Nodes[0].u_node.sensor_type_1.humidity.action_equal,
                                        strlen(Nodes[0].u_node.sensor_type_1.humidity.action_equal), 0,
                                        (struct sockaddr *)&arduino, arduino_addr_size) < 0){

                                printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
                                exit(-1);
                            }
                    }
            }
        }
    }

	//zamykanie wszystkich aktywnych gniazd
	close(s);
	return 0;


}


