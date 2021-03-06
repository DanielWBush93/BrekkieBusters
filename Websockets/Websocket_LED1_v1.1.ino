#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

WebSocketsServer webSocket = WebSocketsServer(301);

const char* ssid = "SKY772BB"; // <========================= your SSID
const char* password = "AUCQURXA"; // <===================== your Password

//LED pins
const int LED1 = 4;
const int LED2 = 5;

//variables for analog pins
int analogval = 0;


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:

      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);

       if (digitalRead(LED1) == HIGH){
          webSocket.broadcastTXT("L11");
       }
       else{
          webSocket.broadcastTXT("L10");
       }

      //send current value on analog pin
       webSocket.broadcastTXT("A1#" + String(analogval));
       
      }
      break;
      
    case WStype_TEXT:
      {
        //extract text
        String text = String((char *) &payload[0]);

        //toggle LED1
        if (text == "LED1") {
          digitalWrite(LED1, !digitalRead(LED1));
          Serial.println("LED1 toggle");
          if (digitalRead(LED1) == HIGH){
            //webSocket.sendTXT(num, "L11", length);
            webSocket.broadcastTXT("L11");
          }
          else{
            //webSocket.sendTXT(num, "L10", length);
            webSocket.broadcastTXT("L10");
          }
        }

        //Get temp and humidity
        if (text == "getTH") {
          float h = dht.readHumidity();
          float t = dht.readTemperature();

          if (isnan(h) || isnan(t)){
            return;
          }
          else{
          Serial.println("Sending TH");
          webSocket.broadcastTXT("T:" + String(t));
          webSocket.broadcastTXT("H:" + String(h));
          }
        }

        //reset all LEDs
        if (text == "RESET") {
          analogWrite(LED1, LOW);
          analogWrite(LED2, LOW);
          Serial.println("reset");
          //webSocket.sendTXT(num, "RST", length);
          webSocket.broadcastTXT("RST");
        }

        //slider handling
        if (text.substring(0,3) == "A1#") {
            analogval = text.substring(text.indexOf('#') + 1).toInt();
            analogWrite(LED2, (2.55 * analogval));
            Serial.println("LED2 Brightness is " + String(analogval));
            webSocket.broadcastTXT("LED2 " + String(analogval));
        }
        
      }
      //webSocket.sendTXT(num, payload, length);
      webSocket.broadcastTXT(payload, length);
      break;

    case WStype_BIN:
      hexdump(payload, length);
      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
  }
}


void setup() {

  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println(WiFi.localIP());
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {

  webSocket.loop();

}
