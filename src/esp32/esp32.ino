#include<WiFi.h>
#include<WiFiUdp.h>
#include "driver/mcpwm.h"
#include "pb_decode.h"
#include "messages_robocup_ssl_detection.pb.h"
#include "ssl_simulation_robot_control.pb.h"
#include "triton_bot_communication.pb.h"
#include "velocityConversions.h"

// Define pins
#define TX 17
#define RX 16
#define BUFF_SIZE 1024
#define START_CHARGE_PIN 15       //KIL - send signal to charge
#define KICK_PIN 2 
#define LED_PIN 4
#define DRIBBLER_PIN 18
#define DRIBBLER_MAX 6

// WiFi credentials
const char* ssid = "wlan3";
const char* password = "a1b2c3d4";

IPAddress local_IP(192, 168, 8, 80);
IPAddress gateway(192, 168, 8, 1);
IPAddress subnet(255, 255, 255, 0);

// UDP setup
WiFiUDP udp;
IPAddress multicastIP(224, 1, 1, 1);
const int multicastPort = 10500;

// UART setup
HardwareSerial espSerial(2);

// timers
unsigned int count = 0;
unsigned int charge_timer = 0;

// kick/dribbler flag
bool kick = false;
bool dribbler_on = false;

// Function to set ESC throttle (0-100%)
void setESCThrottle(float throttle) {
    if (throttle < 0) throttle = 0;
    if (throttle > 100) throttle = 100;

    // Convert throttle % to pulse width (1000µs to 2000µs)
    float pulseWidth = 1000 + (throttle / 100.0) * 1000;  
    float dutyCycle = (pulseWidth / 20000.0) * 100.0;  // Convert to percentage (20ms period)

    Serial.printf("Throttle: %.1f%%, Pulse Width: %.1fµs, Duty Cycle: %.2f%%\n", throttle, pulseWidth, dutyCycle);
    
    // Set duty cycle
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, dutyCycle);
}

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

  // Configure MCPWM for ESC control (50Hz frequency)
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, DRIBBLER_PIN);

  mcpwm_config_t pwm_config;
  pwm_config.frequency = 50;  // 50 Hz for ESC (20ms period)
  pwm_config.cmpr_a = 5;  // Start with minimum throttle (5% duty)
  pwm_config.cmpr_b = 0;
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

  Serial.println("ESC Initialized. Waiting 3 seconds...");
  delay(3000);  // Give ESC time to initialize

  setESCThrottle(0);
  delay(5000);

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

  // Start UDP
  udp.beginMulticast(multicastIP, multicastPort);
  Serial.println("klticast group");
}

void loop() {
  uint8_t packetBuffer[BUFF_SIZE];  // Increase buffer size if needed
  unsigned int packetSize = udp.parsePacket();
  
  if (packetSize) {
    int len = udp.read(packetBuffer, BUFF_SIZE);
    if (len > 0) {
      packetBuffer[len] = '\0';  // Null-terminate the buffer

      // Decode Protocol Buffer message
      proto_triton_TritonBotMessage messageData = proto_triton_TritonBotMessage_init_zero;
      pb_istream_t stream = pb_istream_from_buffer((uint8_t*)packetBuffer, len);
      
      if (pb_decode(&stream, proto_triton_TritonBotMessage_fields, &messageData) && messageData.id == 2) {

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

        if (messageData.command.dribbler_speed != 0) {
          if (!dribbler_on) {
            for (int i = 1; i <= DRIBBLER_MAX; i++) {
              setESCThrottle(i);
              delay(100);
            }
            dribbler_on = true;
          } else {
            setESCThrottle(0);
            dribbler_on = false;
          }
        }
        std::array<uint8_t, 8> msg;
        action_to_byte_array(msg, messageData.command.move_command);

        std::array<uint8_t, 2> header = {0xca, 0xfe};
        std::array<uint8_t, 1> kick = {0x00};

        std::array<uint8_t, 11> full_message;

        full_message[0] = header[0];
        full_message[1] = header[1];

        for (int i = 0; i < 4; i++) {
          full_message[(i * 2 + 2)] = msg[i * 2];
          full_message[(i * 2 + 3)] = msg[i * 2 + 1];
        }

        full_message[10] = kick[0];

        espSerial.write(full_message.data(), full_message.size());
        Serial.printf("(%d) Message sent!\n", count);
        count++;

        for (int i = 0; i < 11; i++) {
          Serial.printf("%x ", full_message[i]);
        } Serial.printf("\n");

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