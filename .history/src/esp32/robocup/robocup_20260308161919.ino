#include "WiFi.h"
#include "helpers.h"
#include "credentials.h"

#define BAUD_RATE 115200

WiFiUDP UDP;
IPAddress multicastIP(239, 42, 42, 42);
HardwareSerial robotSerial(2);

uint16_t packet_size;
char packet_buffer[MAX_PACKET_SIZE];
unsigned long packet_time;
std::array<uint8_t, 11> motor_command;
std::array<uint8_t, 2> motor_cmd_headers = {UART_HEADER_1, UART_HEADER_2_RUNTIME};

void setup() {
  Serial.begin(BAUD_RATE);
  connect_wifi();
  UDP.beginMulticast(multicastIP, MULTICAST_PORT);
  robotSerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
  init_motor_command();
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, LOW);
  pinMode(KICKER_PIN, OUTPUT);
  digitalWrite(KICKER_PIN, LOW);

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
    packet_time = micros();
    PRINT((int)last_packet_size, " | ");
    for (uint16_t i = 0; i < last_packet_size; i++) {
      char c = packet_buffer[i];
      handleNewChar(c);
    }
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
/*
int chargePin = 18;  // CHG - send signal to charge
int kickPin = 5;    // DIS - signal to kick


// Variables
bool kick = false;
bool charge = false;
int userInput = 0; // For incoming serial data


void setup() {
  Serial.begin(115220);


  // Set pin modes
  pinMode(chargePin, OUTPUT);
  digitalWrite(chargePin, LOW);  // Ensure charging is off


  pinMode(kickPin, OUTPUT);
  digitalWrite(kickPin, LOW);  // Ensure solenoid is not kicking
}


void loop() {
 if (Serial.available() > 0) {
  char userInput = Serial.read();

  if (userInput == 'k') {
    digitalWrite(kickPin, HIGH);
    delay(200);
    digitalWrite(kickPin, LOW);
    Serial.println("---------KICKED----------");
    kick = false;
  }

  if (userInput == 'c') {
    digitalWrite(chargePin, HIGH);
    Serial.println("Charging!");
    charge = true;
  }

  if (userInput == 's') {
    digitalWrite(chargePin, LOW);
    Serial.println("Charging stopped");
    charge = false;
  }
}
}
*/
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
