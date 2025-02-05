#include<WiFi.h>
#include<WiFiUdp.h>
#include "PID.h"
#include "testing.h"
// Define pins
#define TX 17
#define RX 16

// WiFi credentials
const char* ssid = "wlan3";
const char* password = "a1b2c3d4";

IPAddress local_IP(192, 168, 8, 80);
IPAddress gateway(192, 168, 8, 1);
IPAddress subnet(255, 255, 255, 0);


// UDP setup
WiFiUDP udp;
const int udpPort = 3333;

// UART setup
HardwareSerial espSerial(2);

int count;

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200, SERIAL_8N1, RX, TX);

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
  
  // Start UDP
  udp.begin(udpPort);
  Serial.printf("Listening on UDP port %d\n", udpPort);

  count = 0;
}

void loop() {
  int i = 1;
  if(i == 0){
    sendToEmbedded(espSerial, kick);
    delay(100);
  }
  char packetBuffer[8];  // Increase buffer size if needed
  int packetSize = udp.parsePacket();
  
  if (packetSize) {
    int len = udp.read(packetBuffer, sizeof(packetBuffer));
    if (len > 0) {
        Serial.println("received");
        packetBuffer[len] = '\0';  // Null-terminate the buffer
        if(packetBuffer[0] == 'a'){
          sendToEmbedded(espSerial ,left);
        } 
        if(packetBuffer[0] == 'w')
          sendToEmbedded(espSerial, forwards);
        if(packetBuffer[0] == 's')
          sendToEmbedded(espSerial, backwards);
        if(packetBuffer[0] == 'd')
          sendToEmbedded(espSerial, right);
        
      }
    }
  
  // Optionally: Handle Wi-Fi disconnection
  delay(100);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected! Reconnecting...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nReconnected to Wi-Fi!");
  }

}
