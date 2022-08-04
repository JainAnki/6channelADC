#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
WebSocketsClient webSocket;

int a[6]={32,33,34,35,36,39};

const char* ssid = "IITH-Guest-PWD-IITH@2022";
const char* password = "IITH@2022";
//const char* ssid = "Neurolabs";
//const char* password = "neuroTechlab@iith";
int count = 0;
int base ;
int test ;
unsigned long messageInterval = 1;
bool connected = false;
#define DEBUG_SERIAL Serial

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
 
    switch(type) {
        case WStype_DISCONNECTED:
            DEBUG_SERIAL.printf("[WSc] Disconnected!\n");
            connected = false;
            break;
        case WStype_CONNECTED: {
            //DEBUG_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
            connected = true;
 
            // send message to server when Connected
            DEBUG_SERIAL.println("[WSc] SENT: Connected");
        }
            break;
        case WStype_TEXT:
            //DEBUG_SERIAL.printf("[WSc] RESPONSE: %s\n", payload);
            break;
        case WStype_PING:
            // pong will be send automatically
            DEBUG_SERIAL.printf("[WSc] get ping\n");
            break;
        case WStype_PONG:
            // answer to a ping we send
            DEBUG_SERIAL.printf("[WSc] get pong\n");
            break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
        break;
    }
}

void setup() {
    DEBUG_SERIAL.begin(115200);
 
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println();
    for(uint8_t t = 4; t > 0; t--) {
        DEBUG_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        DEBUG_SERIAL.flush();
        delay(1000);
    }
    WiFi.begin(ssid, password);
 
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      DEBUG_SERIAL.print ( "." );
    }
    DEBUG_SERIAL.print("Local IP: "); DEBUG_SERIAL.println(WiFi.localIP());
    // server address, port and URL
    //webSocket.begin("192.168.0.6", 7891);
    webSocket.begin("172.20.85.87", 7891);
 
    // event handler
    webSocket.onEvent(webSocketEvent);

    // initialize serial communication at 115200 bits per second:
    Serial.begin(115200);
    for(int i=0;i<6;i++){
      pinMode(a[i],INPUT);
    }
    pinMode(23, INPUT);
    //set the resolution to 12 bits (0-4096)
    analogReadResolution(12);
}

void loop() {
    int trig;
    Serial.println("void loop");
    Serial.println("websocket loop");
    while(trig = digitalRead(23)!=HIGH){
      if(trig ==HIGH){
        break;
      }
    }
    while (trig == HIGH) {
      webSocket.loop();
      int trigRecd = 0;
      while(connected ){
        String json = "";
        DynamicJsonDocument doc(2048);
        for(int i=0;i<6;i++){
          doc["Force_FSR"][i] = analogRead(a[i]);
        }
        serializeJson(doc, json); 
        webSocket.sendTXT(json);
        count++;

        int y;
        if (y = digitalRead(23) == 1){
            Serial.println(count);
            y = 0;
            trigRecd++;
            if(count < 500){
              String js ="";
              DynamicJsonDocument dc(128);
              dc["Force_FSR"] = "Don't plot";
              serializeJson(dc, js); 
              webSocket.sendTXT(js);
              dc.clear();
            }
            count = 0;
            Serial.println(trigRecd);
            doc.clear();
        }
      }
   }
}
