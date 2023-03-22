/*PARKING PROJECT*/
#include <WiFi.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>

#define PARKING_SPOT_1 34
#define PARKING_SPOT_2 35
#define PARKING_SPOT_3 32
#define PARKING_SPOT_4 33

#define ENTRANCE 25

#define LED_SPOT_1 19
#define LED_SPOT_2 18
#define LED_SPOT_3 17
#define LED_SPOT_4 16

//WIFI
const char* ssid = "CERTTO-2BDA6";
const char* password = "rdcsmgf1";

//HTTP
const String URL = "https://smart-parking-api.onrender.com";

void setup() {
  Serial.begin(9600);
  delay(3000);

  //ENTRADAS (VAGAS)
  pinMode(PARKING_SPOT_1, INPUT);
  pinMode(PARKING_SPOT_2, INPUT);
  pinMode(PARKING_SPOT_3, INPUT);
  pinMode(PARKING_SPOT_4, INPUT);
  
  pinMode(ENTRANCE, INPUT);

  //SAÍDAS (LEDS)
  pinMode(LED_SPOT_1, OUTPUT);
  pinMode(LED_SPOT_2, OUTPUT);
  pinMode(LED_SPOT_3, OUTPUT);
  pinMode(LED_SPOT_4, OUTPUT);

  //WIFI SETUP
  WiFi.begin(ssid,password);
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED){
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to the WiFi network");
  delay(1000);
  digitalWrite(LED_SPOT_1,LOW);
  digitalWrite(LED_SPOT_2,LOW);
  digitalWrite(LED_SPOT_3,LOW);
  digitalWrite(LED_SPOT_4,HIGH);
  delay(1000);
}

//trocar isso por uma string de 4 characteres
char* last = "0000";

int lastReading [4] = {
  !digitalRead(PARKING_SPOT_1),
  !digitalRead(PARKING_SPOT_2),
  !digitalRead(PARKING_SPOT_3),
  !digitalRead(PARKING_SPOT_4)
};

int timer = 0;

void loop() {
  timer++;
  if(timer==800000){
    timer=0;
    Serial.println("ENTROU NO TIMER");
    if(!digitalRead(ENTRANCE)){
      Serial.println("FAZENDO BUSCAS");
      
      int currentReading [4] = {
        !digitalRead(PARKING_SPOT_1),
        !digitalRead(PARKING_SPOT_2),
        !digitalRead(PARKING_SPOT_3),
        !digitalRead(PARKING_SPOT_4)
      };
  
      //VERIFICANDO SE TEVE ALGUMA ALTERAÇÃO NOS SENSORES
      bool changed = readingsChanged(lastReading,currentReading);
  
      //SE TEVE ALTERAÇÃO, É PRECISO ENVIA-LAS
      if(changed){
        Serial.println("READINGS CHANGED");
  
        //FAZENDO UPDATE DE LAST READINGS
        for(int i = 0;i<4;i++){
          lastReading[i]=currentReading[i];
        }
  
        //CRIANDO STRING PARA ENVIAR AO SERVIDOR
        String sensors = "";
        for(int i=0;i<4;i++){
          if(currentReading[i]==1){
            sensors+="1";
          }else{
            sensors+="0";
          }
        }
  
        //ENVIANDO STRING AO SERVIDOR
        sendSensorsStatus(sensors);
      }
      //FAZENDO UPDATE DOS LEDS
      updateLeds();
    }
  }
}

void updateLeds(){//char*
  Serial.println("BUSCANDO LEDS");
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(URL+"/esp/leds");
    http.addHeader("Content-Type","text/plain");
    
    int httpCode = http.POST("");
    
    if (httpCode > 0) {
      String payload = http.getString();
      //Serial.println("httpCODE: " + httpCode);
      Serial.println("LEDS ENCONTRADOS");
      Serial.println("PAYLOAD: " + payload);
      
      int init = 19;
      for(int i=0;i<4;i++){
        digitalWrite(init-i,((int) payload[i])-48);
      }
    }else{
      Serial.println("Error on HTTP request");
    }
    http.end(); //Free the resources
  }else{
    Serial.println("Lost internet connection");
  }
  delay(1000);
}

bool readingsChanged(int last [4],int current [4]){
  int currentReading [4] = {
    digitalRead(PARKING_SPOT_1),
    digitalRead(PARKING_SPOT_2),
    digitalRead(PARKING_SPOT_3),
    digitalRead(PARKING_SPOT_4)
  };
  for(int i = 0;i<4;i++){
    if(last[i]!=current[i]){
      return true;
    }
  }
  return false;
}

void sendSensorsStatus(String sensors){
  Serial.println("ENVIANDO SENSORES");
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(URL+"/esp/sensors");
    http.addHeader("Content-Type","text/plain");
    
    int httpCode = http.POST(sensors);
    
    if (httpCode > 0) {
      String payload = http.getString();
      //Serial.println("httpCODE: " + httpCode);
      //Serial.println("PAYLOAD: " + payload);
      Serial.println("SENSORES ENVIADOS COM SUCESSO");
    }else{
      Serial.println("Error on HTTP request");
    }
    http.end(); //Free the resources
  }else{
    Serial.println("Lost internet connection");
  }
  delay(1000);
}
