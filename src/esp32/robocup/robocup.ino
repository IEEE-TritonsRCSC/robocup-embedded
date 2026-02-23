#include "WiFi.h"
#include "helpers.h"
#include "credentials.h"

#define BAUD_RATE 115200
#define LED_PIN 2
#define LED_ON HIGH
#define LED_OFF LOW

WiFiUDP UDP;
IPAddress multicastIP(239, 42, 42, 42);
HardwareSerial robotSerial(2);

uint16_t packet_size;
char packet_buffer[MAX_PACKET_SIZE];
unsigned long packet_time;
std::array<uint8_t, 11> motor_command;
std::array<uint8_t, 2> motor_cmd_headers = {0xCA, 0xFE};

void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);
  connect_wifi();
  UDP.beginMulticast(multicastIP, MULTICAST_PORT);
  robotSerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
  init_motor_command();
  pinMode(SOLENOID_PIN, OUTPUT);
  pinMode(KICKER_PIN, OUTPUT);
  start_charging_kicker();
  PRINT("READY\n");
}

void loop() {
  uint16_t last_packet_size = 0;
  packet_size = UDP.parsePacket();
  // The network may have multiple UDP packets queue up in the buffer
  // Read all of them, but only process the last one to reduce jitter
  while (packet_size > 0) {
    UDP.read(packet_buffer, 511);
    last_packet_size = packet_size;
    packet_size = UDP.parsePacket();
  }

  if (last_packet_size > 0) {
    digitalWrite(LED_PIN, LED_ON);
    packet_time = micros();
    PRINT((int)last_packet_size, " | ");
    for (uint16_t i = 0; i < last_packet_size; i++) {
      char c = packet_buffer[i];
      handleNewChar(c);
    }
    digitalWrite(LED_PIN, LED_OFF);
  }

  check_kicker_status();
}

void connect_wifi() {
  PRINT("\nConnecting WiFi to ", SSID);
  // Attempt connection every 500 ms
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    PRINT(".");
  }
  PRINT("\nWiFi connected", "\nIP address: ", WiFi.localIP(), "\n");
}

void init_motor_command() {
  for (int index = 0; index < MOTOR_COMMAND_SIZE; index++) {
    if (index < MOTOR_CMD_HEADER_SIZE) {
      motor_command[index] = motor_cmd_headers[index];
    } else {
      motor_command[index] = 0;
    }
  }
}

void start_charging_kicker() {
  digitalWrite(KICKER_PIN, LOW);  // turn OFF kicker
  digitalWrite(SOLENOID_PIN, HIGH);  // START charging 
  delay(KICKER_CHARGING_TIME);
  digitalWrite(SOLENOID_PIN, LOW);  // STOP charging
  kicker_charged = true;
}

void check_kicker_status() {
  if (charging_kicker) {
    unsigned long time_elasped = millis() - start_charge_time;
    if (time_elasped >= KICKER_CHARGING_TIME) {
      digitalWrite(SOLENOID_PIN, LOW);  // STOP charging
      charging_kicker = false;
      kicker_charged = true;
    }
  } else {
    unsigned long time_elasped = millis() - last_kick_time;
    if (time_elasped >= WAIT_BEFORE_CHARGE_AGAIN) {
      digitalWrite(SOLENOID_PIN, HIGH);  // START charging
      charging_kicker = true;
      start_charge_time = millis();
    } else if (time_elasped >= KICKING_TIME) {
      digitalWrite(KICKER_PIN, LOW);  // turn OFF the kicker
    }
  }
}
