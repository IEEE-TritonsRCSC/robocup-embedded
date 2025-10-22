#include <WiFi.h>
#include "credentials.h"  // loads ssid and password
#include "velocityConversions.h"

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
    Serial.println(size);
    size = UDP.read(buffer, 255);
    buffer[size] = '\0';
    Serial.println(buffer);
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

void formatAndSendCmd(uint8_t cmd, std::array<uint8_t, 9> payload){
  std::array<uint8_t, 12> full_message;
  full_message[0] = 0xCA;
  full_message[1] = 0xFE;
  full_message[2] = cmd;
  for (int i = 0; i < 9; i++) full_message[3 + i] = payload[i];
  robotSerial.write(full_message.data(), full_message.size());
}

void formatAndSend(std::array<uint8_t, 9> payload){
  // default speed command (cmd = 0x00)
  formatAndSendCmd(0x00, payload);
}

void valuesToBytes(std::array<int, 4>& wheel_speeds, std::array<uint8_t, 9>& wheel_speeds_byte) {
  for (int i = 0; i < 4; i++) {
    wheel_speeds_byte[(i * 2) + 1] = (wheel_speeds[i] >> 8) & 0xFF; // high byte after cmd position shift
    wheel_speeds_byte[(i * 2) + 2] = (wheel_speeds[i]) & 0xFF;      // low byte
  }
}

void action_to_byte_array(std::array<uint8_t, 9>& wheel_speeds_byte, double arg1, double arg2) {
  std::array<int, 4> wheel_speeds;
  getVelocityArray(wheel_speeds, 0, 0, arg1, arg2);
  // Build payload: [s0H s0L s1H s1L s2H s2L s3H s3L dribble]
  for (int i = 0; i < 4; i++){
    wheel_speeds_byte[i*2]   = (wheel_speeds[i] >> 8) & 0xFF;
    wheel_speeds_byte[i*2+1] = (wheel_speeds[i]) & 0xFF;
  }
  wheel_speeds_byte[8] = 1; // dribbler on by default
}

void processCommand(char* buffer, int size){
  char com[5];
  char* endPtr;

  if(strtod(&buffer[0], &endPtr) != ROBOT_NO){
    return;
  }
  memcpy(com, &buffer[2], 4);
  com[4] = '\0';
  if(strcmp(com, "kick") == 0){
    Serial.println("executing kick command");
    kick = 1;

  } else if(strcmp(com, "turn") == 0) {
    Serial.println("executing turn command");
    std::array<uint8_t, 8> msg;
    double angle = strtod(&buffer[7], &endPtr);
    Serial.printf("turn: %f\n", angle);
    action_to_byte_array(msg, 0, angle);
    formatAndSend(msg);
    

  } else if(strcmp(com, "dash") == 0) {
    Serial.println("executing dash command");
    // expects: "<id> dash <power> <rot>\n"
    double power = 0, rot = 0;
    sscanf(buffer + 7, "%lf %lf", &power, &rot);
    std::array<uint8_t, 9> payload;
    action_to_byte_array(payload, power, rot);
    formatAndSend(payload);
  } else if(strcmp(com, "pidu") == 0){
    Serial.println("executing pidu command");
    // expects: "<id> pidu <idx> <kp_q> <ki_q> <kd_q>\n" (kp/ki/kd in milli-units)
    int idx = 0; int kp_q = 0; int ki_q = 0; int kd_q = 0;
    sscanf(buffer + 7, "%d %d %d %d", &idx, &kp_q, &ki_q, &kd_q);
    std::array<uint8_t, 9> payload = {0};
    payload[0] = (uint8_t)idx;
    payload[1] = (uint8_t)((kp_q >> 8) & 0xFF);
    payload[2] = (uint8_t)(kp_q & 0xFF);
    payload[3] = (uint8_t)((ki_q >> 8) & 0xFF);
    payload[4] = (uint8_t)(ki_q & 0xFF);
    payload[5] = (uint8_t)((kd_q >> 8) & 0xFF);
    payload[6] = (uint8_t)(kd_q & 0xFF);
    payload[7] = 0;
    payload[8] = 0;
    formatAndSendCmd(0xA0, payload);
  } else {
    return; //if empty or malformed command, do nothing
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
