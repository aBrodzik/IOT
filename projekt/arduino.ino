
//a)
#include <ZsutDhcp.h>           //dla pobierania IP z DHCP - proforma dla ebsim'a 
#include <ZsutEthernet.h>       //niezbedne dla klasy 'ZsutEthernetUDP'
#include <ZsutEthernetUdp.h>    //sama klasa 'ZsutEthernetUDP'
#include <ZsutFeatures.h>       //for: ZsutMillis()
#include <Arduino.h>

#define MY_IP 192.168.56.106
#define MY_PORT 16240

//#define THEIR_IP 192.168.56.107
#define THEIR_PORT 16240
#define SEND_BUFFER_SIZE        29
#define RECEIVE_BUFFER_SIZE        9

//b)
byte mac[] = {0x08, 0x00, 0x27, 0xB9, 0x90, 0xFA};
//MAC == 08:00:27:B9:90:FA
//c)
ZsutEthernetUDP Udp;

ZsutIPAddress address_ip = ZsutIPAddress(192, 168, 56, 107); //adres serwera z rule engine

//e)
unsigned int localPort = MY_PORT;


void my_delay(unsigned long d) {  //my_delay() obchodzi problem zlych zmiennych w implementacji liczenia czasu w EBSimUnoEth
  unsigned long t1 = ZsutMillis() + d;
  for (;;)
    if (t1 < ZsutMillis())
      break;
}

unsigned long d = 1000;

void setup() {
  // put your setup code here, to run once:
  //d)
  ZsutEthernet.begin(mac);
  Serial.println(ZsutEthernet.localIP());
  Udp.begin(localPort);
}

void dec2bin(unsigned int dec, unsigned char results[10]) {
  int decInt = (int) dec;
  int i = 9;
  int if0 = dec;
  while (!(dec == 0)) {
    results[i] = dec % 2+'0';
    dec = dec / 2;
    i = i - 1;
  }
  while ( i > 0) {
    i = i - 1;
    results[i] = 0+'0';
  }
  if(if0==0){
    for(int j=0; j<10;j++){
      results[j] = 0+'0';
    }
    
  }
}

unsigned char packetBuffer[RECEIVE_BUFFER_SIZE];
unsigned char sendBuffer[SEND_BUFFER_SIZE];

void send_raport() {
  //strcpy(sendBuffer,"%s%s%s%s",bytes,param1,param2,state);
  Udp.beginPacket(address_ip, THEIR_PORT);
  int r = Udp.write(sendBuffer, SEND_BUFFER_SIZE);
  Udp.endPacket();
  memset(sendBuffer, 0, SEND_BUFFER_SIZE );


}

void receive_command() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int r = Udp.read(packetBuffer, RECEIVE_BUFFER_SIZE);
    
    if (packetBuffer[8] == '0') {
      Serial.println("wylaczam zraszacz");
    }
    if (packetBuffer[8] == '1') {
      Serial.println("wlaczam zraszacz");
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  //NODE1

  unsigned int qWaterFlow = ZsutAnalog0Read();
  unsigned int qAngle = ZsutAnalog1Read();
  unsigned int state =  ZsutDigitalRead();
  Serial.print("Przeplywnosc zraszacza:");
  Serial.println(qWaterFlow);
  Serial.print("Kat zraszacza:");
  Serial.println(qAngle);

  unsigned char bytes1[8] = "01000000";
  unsigned char bWaterFlow[10];
  unsigned char bAngle[10];
  dec2bin(qWaterFlow, bWaterFlow);
  dec2bin(qAngle, bAngle);

  for (int i = 0; i < 8; i++) {
    sendBuffer[i] = bytes1[i];
  }

  int j = 0;
  for (int i = 8; i < 18; i++) {
    sendBuffer[i] = bWaterFlow[j];
    
    j++;
  }
  j = 0;
  for (int i = 18; i < 28; i++) {
    sendBuffer[i] = bAngle[j];
    
    j++;
  }
  sendBuffer[28] = 0+'0';
  send_raport();

  receive_command();
  my_delay(d);
  
  //NODE2
  unsigned int qCapacity = ZsutAnalog2Read();
  unsigned int qWaterClarity = ZsutAnalog3Read();
  Serial.print("Pojemnosc zbiornika:");
  Serial.println(qCapacity);
  Serial.print("Czystosc wody:");
  Serial.println(qWaterClarity);


  unsigned char bytes2[8] = "01001001";
  unsigned char bCapacity[10];
  unsigned char bWaterClarity[10];
  dec2bin(qCapacity, bCapacity);
  dec2bin(qWaterClarity, bWaterClarity);

  for (int i = 0; i < 8; i++) {
    sendBuffer[i] = bytes2[i];
  }
  j = 0;
  for (int i = 8; i < 18; i++) {
    sendBuffer[i] = bCapacity[j];
    j++;
  }
  j = 0;
  for (int i = 18; i < 28; i++) {
    sendBuffer[i] = bWaterClarity[j];
    j++;
  }
  send_raport();
  //odbierz
  receive_command();
  my_delay(d);
  


  //NODE3
  unsigned int qTemperature = ZsutAnalog4Read();
  unsigned int qHumidity = ZsutAnalog5Read();
  Serial.print("Temperatura:");
  Serial.println(qTemperature);
  Serial.print("Wilgotnosc:");
  Serial.println(qHumidity);

  unsigned char bytes3[8] = "01010010";
  unsigned char bTemperature[10];
  unsigned char bHumidity[10];
  dec2bin(qTemperature, bTemperature);
  dec2bin(qHumidity, bHumidity);

  for (int i = 0; i < 8; i++) {
    sendBuffer[i] = bytes3[i];
  }
  j = 0;
  for (int i = 8; i < 18; i++) {
    sendBuffer[i] = bTemperature[j];
    j++;
  }
  j = 0;
  for (int i = 18; i < 28; i++) {
    sendBuffer[i] = bHumidity[j];
    j++;
  }


  //wyslij
  send_raport();
  //odbierz
  receive_command();
  //poczekaj troche
  my_delay(d);
 



}