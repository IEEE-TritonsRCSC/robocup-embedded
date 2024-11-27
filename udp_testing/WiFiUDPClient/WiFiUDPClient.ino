#include <WiFi.h>
#include <WiFiUdp.h>
#include "pb_decode.h"
#include "data.pb.h"

// WiFi credentials
const char* ssid = "wlan3";
const char* password = "a1b2c3d4";

// UDP setup
WiFiUDP udp;
const int udpPort = 3333;

void setup() {
  Serial.begin(115200);
  
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
}

void loop() {
  char packetBuffer[1024];  // Increase buffer size if needed
  int packetSize = udp.parsePacket();
  
  if (packetSize) {
    int len = udp.read(packetBuffer, sizeof(packetBuffer));
    if (len > 0) {
      packetBuffer[len] = '\0';  // Null-terminate the buffer

      // Decode Protocol Buffer message
      SensorData sensorData = SensorData_init_zero;
      pb_istream_t stream = pb_istream_from_buffer((uint8_t*)packetBuffer, len);
      if (pb_decode(&stream, SensorData_fields, &sensorData)) {
        Serial.println("Received valid data:");
        Serial.printf("ID: %d, Temperature: %.2f, Humidity: %.2f\n", 
                      sensorData.id, sensorData.temperature, sensorData.humidity);
      } else {
        Serial.println("Failed to decode message.");
      }
    }
  }
  
  // Optionally: Handle Wi-Fi disconnection
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
