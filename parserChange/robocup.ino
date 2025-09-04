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

char[255] inputBuffer; //buffer for storing overflow command
int bufferSize = 0; //how many actual chars it contains
unsigned int kick = 0;
unsigned int charge_timer = 0;

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

    int commandLen = 0;
    int firstTime = 1;
    for(int i = 0; i < size; i++){  //loop to split strings
      commandLen++;
      if(buffer[i] == "\n"){
        //split
        char commandLine[commandLen+bufferSize]; //bufferSize should be 0 if it is already appended
        if(firstTime){ //append the rest
          memcpy(commandLine, inputBuffer, bufferSize); //append inputBuffer first
          memcpy(commandLine[bufferSize], buffer[i-commandLen+1], commandLen);
        } else {
          memcpy(commandLine, buffer[i-commandLen+1], commandLen);
        }
        processCommand(commandLine, commandLen); //command is parsed and sent
        commandLen = 0;
      }
    }
    if (commandLen != 0) { //grab remaining letters and stuff into global buffer
      memcpy(inputBuffer, buffer[size-commandLen], commandLen);
      bufferSize = commandLen;
    }
  }

  if (kick) {
    digitalWrite(START_CHARGE_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    Serial.println("charging");
    charge_timer = 1;
    kick = 0;
  }

  if (charge_timer) {
    if (charge_timer >= 2000) {
      Serial.println("charge off, kicking!");
      digitalWrite(START_CHARGE_PIN, LOW);
      digitalWrite(KICK_PIN, LOW);
      digitalWrite(LED_PIN, LOW);

      delay(100);

      digitalWrite(KICK_PIN, HIGH);
      Serial.println("---------KICKED----------");
      charge_timer = 0;

    } else {
      charge_timer++;
    }
  }
}

void processCommand(char* buffer, int size){
  char com[4]; //grab string
  char* endPtr;
  memcpy(com, &buffer[j], 4); //note: we only grabbed 4 chars, so it can only be 4 word strings

  if(strcmp(com, "kick")) { //if kick 
    kick = 1;
  } else if(strcmp(com, "turn")) {
    std::array<uint8_t, 8> msg;
    double angle = strtod(&buffer[5], &endPtr);
    action_to_byte_array(msg, 0, angle);
    formatAndSend(msg);
    

  } else if(strcmp(com, "dash")) {
    std::array<uint8_t, 8> msg;
    double pow = strtod(&buffer[5], &endPtr);
    double angle = strtod(endPtr+1, &endPtr);
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
  robotSerial.write(full_message.data(), full_message.size());
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
