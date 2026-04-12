#include "globals.h"
#include "credentials.h"
#include "helpers.h"
#include <WebServer.h>

IPAddress multicastIP(239, 42, 42, 42);
HardwareSerial robotSerial(2);
WiFiUDP UDP;
CommandPacket_t current_cmd = {0};

void setup() {
  Serial.begin(115200);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  UDP.beginMulticast(multicastIP, 10000);
  robotSerial.begin(UART_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
}

void loop() {
  int packetSize = UDP.parsePacket();
  if (packetSize > 0) {
    char packetBuffer[9];
    UDP.read(packetBuffer, 511);
    packetBuffer[packetSize] = '\0';
    
    // Reset flags before parsing new packet
    current_cmd.kick = 0;
    current_cmd.chip = 0;

    parseMsg(packetBuffer);
    
    // Send 22-byte binary packet to STM32
    robotSerial.write((uint8_t*)&current_cmd, sizeof(current_cmd));
  }
  yield();
}