// #include<WiFi.h>
// #include<WiFiUdp.h>
// #include "pb_decode.h"
// #include "messages_robocup_ssl_detection.pb.h"
// #include "ssl_simulation_robot_control.pb.h"
// #include "triton_bot_communication.pb.h"
// #include "velocityConversions.h"

// Define pins
#define TX 17
#define RX 16

// WiFi credentials
const char* ssid = "wlan3";
const char* password = "a1b2c3d4";

// IPAddress local_IP(192, 168, 8, 80);
// IPAddress gateway(192, 168, 8, 1);
// IPAddress subnet(255, 255, 255, 0);


// // UDP setup
// WiFiUDP udp;
// const int udpPort = 3333;

// UART setup
HardwareSerial espSerial(2);

int count;

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200, SERIAL_8N1, RX, TX);

  // if (!WiFi.config(local_IP, gateway, subnet)) {
  //   Serial.println("STA Failed to configure");
  // }

  // // Connect to Wi-Fi
  // Serial.print("Connecting to Wi-Fi");
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("\nConnected to Wi-Fi!");
  
  // Start UDP
  // udp.begin(udpPort);
  // Serial.printf("Listening on UDP port %d\n", udpPort);

  count = 0;
}

void loop() {
  std::array<unsigned char, 11> msg = {0xca, 0xfe, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

  
  espSerial.write(msg.data(), msg.size());

  Serial.printf("(%d) ", count);
  for (int i = 0; i < 11; i++) {
      Serial.printf("%x ", msg.data()[i]);
  } Serial.printf("\n");

  // espSerial.flush();
  count++;

  Serial.println("wrote");

  delay(500);


  /*char packetBuffer[1024];  // Increase buffer size if needed
  int packetSize = udp.parsePacket();
  
  if (packetSize) {
    int len = udp.read(packetBuffer, sizeof(packetBuffer));
    if (len > 0) {
      packetBuffer[len] = '\0';  // Null-terminate the buffer

      // Decode Protocol Buffer message
      proto_triton_TritonBotMessage messageData = proto_triton_TritonBotMessage_init_zero;
      pb_istream_t stream = pb_istream_from_buffer((uint8_t*)packetBuffer, len);
      
      if (pb_decode(&stream, proto_triton_TritonBotMessage_fields, &messageData)) {
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

        std::array<unsigned char, 8> msg;

        action_to_byte_array(msg, messageData.command.move_command);

        std::array<unsigned char, 2> header = {0xca, 0xfe};
        std::array<unsigned char, 1> kick = {0x00};

        std::array<unsigned char, 11> full_message;

        full_message[0] = header[0];
        full_message[1] = header[1];

        int motor_speed = 50;

        for (int i = 0; i < 4; i++) {
          full_message[(i * 2 + 2)] = (motor_speed >> 8 & 0xff);
          full_message[(i * 2 + 3)] = (motor_speed & 0xff);
        }

        full_message[10] = kick[0];

        espSerial.write(full_message.data(), full_message.size());
        espSerial.flush();
        //Serial.println("Message sent!");

        for (int i = 0; i < 11; i++) {
          Serial.printf("%x ", full_message[i]);
        } Serial.printf("\n");


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
  } */
}

