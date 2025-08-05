#include<WiFi.h>
#include<WiFiUdp.h>
#include "pb_decode.h"
#include "messages_robocup_ssl_detection.pb.h"
#include "ssl_simulation_robot_control.pb.h"
#include "triton_bot_communication.pb.h"
#include "velocityConversions.h"

// Define pins
#define TX 17
#define RX 16
#define START_CHARGE_PIN 15      //KIL - send signal to charge
#define KICK_PIN 2 
#define LED_PIN 4

// Buffer size for receiving messages
#define BUFF_SIZE 1024

// Robot ID number
#define ROBOT_ID 2

// WiFi credentials
const char* ssid = "wlan3";
const char* password = "a1b2c3d4";

IPAddress local_IP(192, 168, 8, 80);
IPAddress gateway(192, 168, 8, 1);
IPAddress subnet(255, 255, 255, 0);

// Multicast setup
WiFiUDP udp;
IPAddress multicastIP(224, 1, 1, 1);
const int multicastPort = 10500;

// Unicast setup
const int udpPort = 3333;

// UART setup
HardwareSerial espSerial(2);

// timers
unsigned int count = 0;
unsigned int charge_timer = 0;

// kick flag
bool kick = false;

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200, SERIAL_8N1, RX, TX);

  // begins charging, High signal opens gate, low closes
  pinMode(START_CHARGE_PIN, INPUT_PULLUP);
  pinMode(START_CHARGE_PIN, OUTPUT);
  digitalWrite(START_CHARGE_PIN, LOW);

  // opens and closes solenoid, HIGH is not kicking, LOW is kicking
  pinMode(KICK_PIN, INPUT_PULLUP);
  pinMode(KICK_PIN, OUTPUT);
  digitalWrite(KICK_PIN, HIGH);

  //turns on and off LED
  pinMode(LED_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

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

  // Start Multicast UDP
  // udp.beginMulticast(multicastIP, multicastPort);
  // Serial.println("Joined multicast group");

  // Unicast
  udp.begin(udpPort);
  Serial.println("Listening for unicast UDP on port 3333");
}

void loop() {
  uint8_t packetBuffer[BUFF_SIZE];  // Increase buffer size if needed
  unsigned int packetSize = udp.parsePacket();
  
  if (espSerial.available() >= 5) {
      if (espSerial.read() == 0xca && espSerial.read() == 0xfe) {
      uint8_t data[3];
      espSerial.readBytes(data, 3);

      int motor = data[0];
      int speed = ((int16_t) (data[1] << 8 | data[2]));

      Serial.printf("Motor %d", motor);
      Serial.printf("Speed %d", speed);
    }
  }

  if (packetSize) {
    int len = udp.read(packetBuffer, BUFF_SIZE);
    Serial.printf("Packet length: %d", len);
    if (len > 0) {
      packetBuffer[len] = '\0';  // Null-terminate the buffer

      Serial.printf("%s\n", packetBuffer);

      // Decode Protocol Buffer message
      proto_triton_TritonBotMessage messageData = proto_triton_TritonBotMessage_init_zero;
      pb_istream_t stream = pb_istream_from_buffer((uint8_t*)packetBuffer, len);
      
      if (pb_decode(&stream, proto_triton_TritonBotMessage_fields, &messageData) && messageData.id == ROBOT_ID) {

        Serial.println("Received valid message:");
        Serial.printf("ID: %d\n", messageData.id);
        Serial.printf("Vision Data:\n");
        Serial.printf(".   confidence: %f\n", messageData.vision.confidence);
        Serial.printf(".   robot_id: %d\n", messageData.vision.robot_id);
        Serial.printf(".   x: %f\n", messageData.vision.x);
        Serial.printf(".   y: %f\n", messageData.vision.y);
        Serial.printf(".   orientation: %f\n", messageData.vision.orientation);
        Serial.printf(".   pixel_x: %f\n", messageData.vision.pixel_x);
        Serial.printf(".   pixel_y: %f\n", messageData.vision.pixel_y);
        Serial.printf(".   height: %f\n", messageData.vision.height);
        Serial.printf("Command:\n");
        Serial.printf(".   id: %d\n", messageData.command.id);
        Serial.printf(".   forward: %f\n", messageData.command.move_command.command.local_velocity.forward);
        Serial.printf(".   left: %f\n", messageData.command.move_command.command.local_velocity.left);
        Serial.printf(".   angular: %f\n", messageData.command.move_command.command.local_velocity.angular);
        Serial.printf(".   kick_speed: %f\n", messageData.command.kick_speed);
        Serial.printf(".   kick_angle: %f\n", messageData.command.kick_angle);
        Serial.printf(".   dribbler_speed: %f\n\n", messageData.command.dribbler_speed);

        // turn kicker on/off
        kick = (messageData.command.kick_speed != 0);

        std::array<uint8_t, 8> msg;
        action_to_byte_array(msg, messageData.command.move_command);

        std::array<uint8_t, 2> header = {0xca, 0xfe};
        std::array<uint8_t, 1> dribble = {0x00};

        if (messageData.command.dribbler_speed > 0) {
          dribble[0] = 0x01;
        }

        std::array<uint8_t, 11> full_message;

        full_message[0] = header[0];
        full_message[1] = header[1];

        for (int i = 0; i < 4; i++) {
          full_message[(i * 2 + 2)] = msg[i * 2];
          full_message[(i * 2 + 3)] = msg[i * 2 + 1];
        }

        full_message[10] = dribble[0];

        espSerial.write(full_message.data(), full_message.size());
        // Serial.printf("(%d) Message sent!\n", count);
        count++;

        // for (int i = 0; i < 11; i++) {
        //   Serial.printf("%x ", full_message[i]);
        // } Serial.printf("\n");

      }
    }
  }

  if (kick) {
    digitalWrite(START_CHARGE_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    Serial.println("charging");
    charge_timer = 1;
    kick = false;
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

  delay(1);
}