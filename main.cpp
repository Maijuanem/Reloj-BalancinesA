// Select a Timer Clock
#define USING_TIM_DIV1                false           // for shortest and most accurate timer
#define USING_TIM_DIV16               true           // for medium time and medium accurate timer
#define USING_TIM_DIV256               false           // for longest timer but least accurate. Default

#include <Arduino.h>
#include <NTPClient.h> //importamos la librería del cliente NTP
#include <ESP8266WiFi.h> //librería necesaria para la conexión wifi
#include <WiFiUdp.h> // importamos librería UDP para comunicar con 
#include "ESP8266TimerInterrupt.h"

#define intervaloInterrupcion 50 //en microsegundos

/********************************************************************/
 #define numCero     0x3F
 #define numUno      0x06
 #define numDos      0x5B
 #define numTres     0x4F
 #define numCuatro   0x66
 #define numCinco    0x6D
 #define numSeis     0x7D
 #define numSiete    0x07
 #define numOcho     0x7F
 #define numNueve    0x6F 
/*********************************************************************/
typedef union {
  unsigned char valorHexa;
  struct {
    unsigned char A   :1 ; //1
    unsigned char B   :1 ; //2
    unsigned char C   :1 ; //4
    unsigned char D   :1 ; //8
    unsigned char E   :1 ; //1
    unsigned char F   :1 ; //2
    unsigned char G   :1 ; //4
    unsigned char H   :1 ; //8
  }segmento;
}ledSevenSegmentos;
/***********************************************************************/

unsigned char numeroLedArray[10]={  numCero,numUno,numDos,numTres,numCuatro,
                                    numCinco,numSeis,numSiete,numOcho,numNueve};
unsigned char digitoActivado[4]={NUM_1,NUM_2,NUM_3,NUM_4};


volatile int    horaNTP=0,
                minutoNTP=0,
                ajusteConsulta;
volatile int    horaDecena=0,
                horaUnidad=0,
                minutoDecena=0,
                minutoUnidad=0;
volatile  int   contadorTimer=0,
                contadorSegundos=0,
                contadorCiclosCartel=0;
boolean         tomarTiempo = true,
                dosPuntos = true;
ledSevenSegmentos numeroSieteSegmentos[4];

const char *ssid     = "";
const char *password = "";
volatile unsigned char i=0;

WiFiUDP       ntpUDP;
NTPClient     timeClient(ntpUDP, "0.south-america.pool.ntp.org",-10800,6000);

void IRAM_ATTR ISR_Ciclo(void);
void setup(){ 
  Serial.begin(115200);
  Serial.println("\nInicie la comunicacion Serial");
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(CLK164,OUTPUT);
  pinMode(DSB164,OUTPUT);
  pinMode(CLK574,OUTPUT);
  pinMode(NUM_1,OUTPUT); 
  pinMode(NUM_2,OUTPUT); 
  pinMode(NUM_3,OUTPUT); 
  pinMode(NUM_4,OUTPUT); 
  Serial.println("Inicie PUERTOS");
  WiFi.begin(ssid, password); // nos conectamos al wifi
  delay(100);
  Serial.println("Me conencte a la red WiFi");
  delay(1000);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
    Serial.print ( "." );
  }
  timeClient.begin(); 
  delay(100);
  Serial.println("me conecte al servidor NTP");

/***************************Configuracion de Temporizacion***********************************************************/
	timer1_attachInterrupt(ISR_Ciclo); // Add ISR Function
  delay(100);
 	timer1_write(5*intervaloInterrupcion); // Ejemplo = 350 / 5 ticks de TIM_DIV16 == 70 uS intervalo 
  delay(100);
	timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
/********************************************************************************************************************/
  Serial.println("TIMER INICIADO");
  
}
void loop() {
  if(tomarTiempo){
    timeClient.update(); //sincronizamos con el server NTP
    horaNTP = timeClient.getHours();
    horaDecena=horaNTP/10;
    numeroSieteSegmentos[0].valorHexa= numeroLedArray[horaDecena];
    horaUnidad=horaNTP%10;
    numeroSieteSegmentos[1].valorHexa= numeroLedArray[horaUnidad];
    minutoNTP = timeClient.getMinutes();
    minutoDecena = minutoNTP/10;
    numeroSieteSegmentos[2].valorHexa= numeroLedArray[minutoDecena];
    minutoUnidad = minutoNTP%10;
    int sec = timeClient.getSeconds();
    ajusteConsulta=60-sec;
    numeroSieteSegmentos[3].valorHexa= numeroLedArray[minutoUnidad];
    Serial.print(horaNTP);Serial.print(":"); Serial.println(minutoNTP);
    Serial.print(horaDecena);Serial.print(horaUnidad); Serial.print(":"); Serial.print(minutoDecena); Serial.print (minutoUnidad); 
    Serial.print(":");Serial.print(sec); Serial.print("\t"); Serial.println(ajusteConsulta);
    tomarTiempo = false;
  }
}


/*
1s / 70us = 14285.7 ~ 14286 llamadas a la funcion
*/
void IRAM_ATTR ISR_Ciclo(void) {
  static int cartelMuestreo=0;
  switch (contadorCiclosCartel)
  {   
  case 0:
    digitalWrite(CLK164,0);
    digitalWrite(CLK574,0);
    digitalWrite(DSB164,numeroSieteSegmentos[cartelMuestreo].segmento.F);
    break;
  case 1:
    digitalWrite(CLK164,1);
    digitalWrite(CLK574,0);
    break;
  case 2:
    digitalWrite(CLK164,0);
    digitalWrite(CLK574,0);
    digitalWrite(DSB164,numeroSieteSegmentos[cartelMuestreo].segmento.G);
    break;
  case 3:
    digitalWrite(CLK164,1);
    digitalWrite(CLK574,0);
    break;
  case 4:
    digitalWrite(CLK164,0);
    digitalWrite(CLK574,0);
    digitalWrite(DSB164,numeroSieteSegmentos[cartelMuestreo].segmento.A);
    break;
  case 5:
    digitalWrite(CLK164,1);
    digitalWrite(CLK574,0);
    break;
  case 6:
    digitalWrite(CLK164,0);
    digitalWrite(CLK574,0);
    if(cartelMuestreo==1){
      if(dosPuntos) numeroSieteSegmentos[cartelMuestreo].segmento.H = 1;
      else          numeroSieteSegmentos[cartelMuestreo].segmento.H = 0;
    }
    digitalWrite(DSB164,numeroSieteSegmentos[cartelMuestreo].segmento.H);
    break;
  case 7:
    digitalWrite(CLK164,1);
    digitalWrite(CLK574,0);
    break;
  case 8:
    digitalWrite(CLK164,0);
    digitalWrite(CLK574,0);
    digitalWrite(DSB164,numeroSieteSegmentos[cartelMuestreo].segmento.E);
    break;
  case 9:
    digitalWrite(CLK164,1);
    digitalWrite(CLK574,0);
    break;
  case 10:
    digitalWrite(CLK164,0);
    digitalWrite(CLK574,0);
    digitalWrite(DSB164,numeroSieteSegmentos[cartelMuestreo].segmento.B);
    break;
  case 11:
    digitalWrite(CLK164,1);
    digitalWrite(CLK574,0);
    break;
  case 12:
    digitalWrite(CLK164,0);
    digitalWrite(CLK574,0);
    digitalWrite(DSB164,numeroSieteSegmentos[cartelMuestreo].segmento.C);
    break;
  case 13:
    digitalWrite(CLK164,1);
    digitalWrite(CLK574,0);
    break;
  case 14:
    digitalWrite(CLK164,0);
    digitalWrite(CLK574,0);
    digitalWrite(DSB164,numeroSieteSegmentos[cartelMuestreo].segmento.D);
    break;
  case 15:
    digitalWrite(CLK164,1);
    digitalWrite(CLK574,0);
    if(!cartelMuestreo) digitalWrite(digitoActivado[3],0);
    else                digitalWrite(digitoActivado[cartelMuestreo-1],0);
    break;
   case 16:
    digitalWrite(CLK164,0);
    digitalWrite(CLK574,1);
    digitalWrite(digitoActivado[cartelMuestreo],1);      
    break;
  case 17:
    digitalWrite(CLK164,0);
    digitalWrite(CLK574,1);
    digitalWrite(digitoActivado[cartelMuestreo],1);      
    cartelMuestreo++;
    if(cartelMuestreo > 3 ) cartelMuestreo=0;
    break;
  default:
    digitalWrite(CLK164,0);
    digitalWrite(CLK574,0);
    cartelMuestreo=0;
    return;
  }
  contadorCiclosCartel++;
  if(contadorCiclosCartel > 17) contadorCiclosCartel=0;
  contadorTimer++;
  if(contadorTimer > (1000000/intervaloInterrupcion)){
     Serial.println(contadorSegundos);
     dosPuntos=!dosPuntos;
    contadorSegundos++;
    // numeroSieteSegmentos[0].valorHexa=numeroSieteSegmentos[1].valorHexa=numeroSieteSegmentos[2].valorHexa=numeroSieteSegmentos[3].valorHexa=numeroLedArray[i];  
    // i++;
    // if(i>9) i=0;
    digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
    if(contadorSegundos>ajusteConsulta){
      tomarTiempo=true;
      contadorSegundos=0;
    }
    contadorTimer=0;
  }
}
