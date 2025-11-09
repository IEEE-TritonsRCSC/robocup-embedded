#include <WiFi.h>
#include "credentials.h"  // loads ssid and password
#include "velocityConversions.h"

#define DEBUG_SERIAL // Uncomment this line to enable debug serial output

#ifdef DEBUG_SERIAL
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#define DEBUG_PRINTF(...)
#endif

#define MULTICAST_PORT 10000
#define BAUD_RATE 115200
#define TX_PIN 17
#define RX_PIN 16
#define START_CHARGE_PIN 15
#define KICK_PIN 2
#define LED_PIN 2
#define ROBOT_NO 1
WiFiUDP UDP;
IPAddress multicastIP(239, 42, 42, 42);
HardwareSerial robotSerial(2);

char inputBuffer[255]; //buffer for storing overflow command
int bufferSize = 0; //how many actual chars it contains
unsigned int kick = 0;
unsigned int charge_timer = 0;

void setup() {
  Serial.begin(BAUD_RATE);
  connect_wifi();
  UDP.beginMulticast(multicastIP, MULTICAST_PORT);

  //starts charging
  pinMode(START_CHARGE_PIN, INPUT_PULLUP);
  pinMode(START_CHARGE_PIN, OUTPUT);
  digitalWrite(START_CHARGE_PIN, LOW);

  //opens and closes solenoid
  pinMode(KICK_PIN, INPUT_PULLUP);
  pinMode(KICK_PIN, OUTPUT);
  digitalWrite(KICK_PIN, HIGH);

  //turns on and off led
  pinMode(LED_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);


  robotSerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);

}

void loop() {
  int size = UDP.parsePacket();
  if (size) {
    char buffer[256];
    DEBUG_PRINTLN(size);
    size = UDP.read(buffer, 255);
    buffer[size] = '\0';
    DEBUG_PRINTLN(buffer);
    UDP.flush();

    int commandLen = 0;
    int firstTime = 1;
    for(int i = 0; i < size; i++){  //loop to split strings
      commandLen++;
      if(buffer[i] == '\n'){
        //split
        char commandLine[commandLen+bufferSize]; //bufferSize should be 0 if it is already appended
        if(firstTime){ //append the rest
          memcpy(commandLine, inputBuffer, bufferSize); //append inputBuffer first
          memcpy(&commandLine[bufferSize], &buffer[i-commandLen+1], commandLen);
        } else {
          memcpy(commandLine, &buffer[i-commandLen+1], commandLen);
        }
        processCommand(commandLine, commandLen); //command is parsed and sent
        commandLen = 0;
      }
    }
    if (commandLen != 0) { //grab remaining letters and stuff into global buffer
      memcpy(inputBuffer, &buffer[size-commandLen], commandLen);
      bufferSize = commandLen;
    }
  }

  if (kick) {
    digitalWrite(START_CHARGE_PIN, HIGH);
  	digitalWrite(LED_PIN, HIGH);
    DEBUG_PRINTLN("charging");
    charge_timer = 1;
    kick = 0;
  }

  if (charge_timer) {
    if (charge_timer >= 2000) {
      DEBUG_PRINTLN("charge off, kicking!");
      digitalWrite(START_CHARGE_PIN, LOW);
      digitalWrite(KICK_PIN, LOW);
      digitalWrite(LED_PIN, LOW);

      delay(100);

      digitalWrite(KICK_PIN, HIGH);
      DEBUG_PRINTLN("---------KICKED----------");
      charge_timer = 0;

    } else {
      charge_timer++;
    }
  }
}

void processCommand(char* buffer, int size){
  //per new spcifiation we check number first
  char com[5]; //grab string
  char* endPtr;

  if(strtod(&buffer[0], &endPtr) != ROBOT_NO){
    return;
  }
  memcpy(com, &buffer[2], 4); //note: we only grabbed 4 chars, so it can only be 4 word strings
  com[4] = '\0';
  DEBUG_PRINTF("command: %s\n", com);
  if(strcmp(com, "kick") == 0) { //if kick 
    DEBUG_PRINTLN("executing kick command");
    kick = 1;

  } else if(strcmp(com, "turn") == 0) {
    DEBUG_PRINTLN("executing turn command");
    std::array<uint8_t, 8> msg;
    double angle = strtod(&buffer[7], &endPtr);
    DEBUG_PRINTF("turn: %f\n", angle);
    action_to_byte_array(msg, 0, angle);
    formatAndSend(msg);
    

  } else if(strcmp(com, "dash") == 0) {
    DEBUG_PRINTLN("executing dash command");
    std::array<uint8_t, 8> msg;
    double pow = strtod(&buffer[7], &endPtr);
    double angle = strtod(endPtr+1, &endPtr);
    DEBUG_PRINTF("pow: %f, angle: %f\n", pow, angle);
    action_to_byte_array(msg, pow, angle);
    formatAndSend(msg);
  } else {
    return; //if empty or malformed command, do nothing
  }
}

void formatAndSend(std::array<uint8_t, 8> msg){
  std::array<uint8_t, 2> header = {0xca, 0xfe};
  std::array<uint8_t, 11> full_message;
  full_message[0] = header[0];
  full_message[1] = header[1];
  for (int i = 0; i < 8; i++) {
    full_message[i + 2] = msg[i];
  }
  full_message[10] = 1; //dribbler 
  
  // Send to STM32
  robotSerial.write(full_message.data(), full_message.size());
  
  // Debug: print what we sent
  DEBUG_PRINT("TX to STM32: ");
  for (int i = 0; i < full_message.size(); i++) {
    DEBUG_PRINTF("%02X ", full_message[i]);
  }
  DEBUG_PRINTLN();
  
  // Wait for ACK from STM32 (with timeout)
  unsigned long start = millis();
  while (millis() - start < 20) { // 20ms timeout (reduced to prevent queue buildup)
    if (robotSerial.available() >= 3) {
      uint8_t ack[3];
      robotSerial.readBytes(ack, 3);
      if (ack[0] == 0xAC && ack[1] == 0xCE) {
        DEBUG_PRINTF("STM32 ACK received: %02X\n", ack[2]);
        digitalWrite(LED_PIN, HIGH);
        delay(10);
        digitalWrite(LED_PIN, LOW);
        return;
      }
    }
    delay(1); // Small delay to prevent tight loop
  }
  DEBUG_PRINTLN("WARNING: No ACK from STM32!");
}

void connect_wifi() {
  DEBUG_PRINT("\nConnecting WiFi to ");
  DEBUG_PRINT(ssid);
  // Attempt connection every 500 ms
  //WiFi.config(ip);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(".");
  }
  DEBUG_PRINT("\nWiFi connected");
  DEBUG_PRINT("\nIP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
}
