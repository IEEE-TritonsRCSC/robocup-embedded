#include <WiFi.h>
#include "credentials.h"  // loads ssid and password
#include "velocityConversions.h"

#define MULTICAST_PORT 10000
#define BAUD_RATE 115200
#define TX_PIN 17
#define RX_PIN 16

WiFiUDP UDP;
IPAddress multicastIP(239, 42, 42, 42);
HardwareSerial robotSerial(2);

void setup() {
  Serial.begin(BAUD_RATE);
  connect_wifi();
  UDP.beginMulticast(multicastIP, MULTICAST_PORT);

  robotSerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
}

void loop() {
  int size = UDP.parsePacket();
  if (size) {
    char buffer[256];
    Serial.println(size);
    size = UDP.read(buffer, 255);
    buffer[size] = '\0';
    Serial.println(buffer);
    UDP.flush();

    std::array<uint8_t, 8> msg;
    action_to_byte_array(msg);

    std::array<uint8_t, 2> header = {0xca, 0xfe};
    std::array<uint8_t, 11> full_message;
    full_message[0] = header[0];
    full_message[1] = header[1];
    for (int i = 0; i < 8; i++) {
      full_message[i + 2] = msg[i];
    }
    robotSerial.write(full_message.data(), full_message.size());
  }
}

void connect_wifi() {
  Serial.print("\nConnecting WiFi to ");
  Serial.print(ssid);
  // Attempt connection every 500 ms
  //WiFi.config(ip);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected");
  Serial.print("\nIP address: ");
  Serial.println(WiFi.localIP());
}
