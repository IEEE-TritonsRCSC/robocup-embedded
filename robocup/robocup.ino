/**
 * @file robocup.ino
 * @brief Main firmware entry point for the RoboCup robot controller.
 *
 * Sets up serial, GPIO, Wi-Fi, UDP, and CAN-related state, then processes
 * incoming UDP commands in the main loop.
 */

#include "commands.h"
#include "credentials.h"

#define BAUD_RATE 115200

static WiFiUDP udp;
static ACAN2517FD can(MCP2517_CS, SPI, MCP2517_INT);
// static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// static unsigned short displayY = 0;
// static char* WiFiThrobberText[4] = {
//   "WiFi Connecting |",
//   "WiFi Connecting /",
//   "WiFi Connecting -",
//   "WiFi Connecting \\"
// };
// static unsigned short throbberIndex = 0;

static Moteus* Motors[NUM_MOTORS]{ nullptr };
static Moteus* Wheels[NUM_WHEELS]{ nullptr };
static unsigned long lastUdpCommandMs = 0;
static bool watchdogStopped = false;
// static unsigned long displayPageTimer = 0;
// static unsigned short displayPageCounter = 0;
static constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 60000;

// Moteus CANFD Position Commands for each motor
static PositionCommand FrontLeftWheelCmd;
static PositionCommand FrontRightWheelCmd;
static PositionCommand BackRightWheelCmd;
static PositionCommand BackLeftWheelCmd;
static PositionCommand DribblerCmd;

static PositionCommand* MotorCommands[NUM_MOTORS] = {
  &FrontLeftWheelCmd,
  &FrontRightWheelCmd,
  &BackRightWheelCmd,
  &BackLeftWheelCmd,
  &DribblerCmd
};

static PositionCommand* WheelCommands[NUM_WHEELS] = {
  &FrontLeftWheelCmd,
  &FrontRightWheelCmd,
  &BackRightWheelCmd,
  &BackLeftWheelCmd,
};

static void connectWiFi() {
  WiFi.config(LOCAL_IP_ADDRESS, GATEWAY_IP_ADDRESS, SUBNET_MASK);
  Serial.println(F("WiFi config'ed!"));
  // Serial.print(F("WiFi firmware: "));
  // Serial.println(WiFi.firmwareVersion());
  Serial.print(F("Connecting to SSID: "));
  Serial.println(WIFI_SSID);

  const unsigned long startMs = millis();
  while (WiFi.status() != WL_CONNECTED) {
    const int beginResult = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print(F("WiFi.begin() returned: "));
    Serial.println(beginResult);
    const unsigned long elapsed = millis() - startMs;
    Serial.print('.');
    if (elapsed >= WIFI_CONNECT_TIMEOUT_MS) {
      Serial.println();
      Serial.print(F("WiFi connect timeout after "));
      Serial.print(elapsed);
      Serial.println(F(" ms"));
      Serial.print(F("WiFi status code: "));
      Serial.println(WiFi.status());
      return;
    }
    delay(250);
  }

  Serial.println();
  Serial.print(F("WiFi Connected: "));
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("Serial Started!");

  // Wire.begin();

  // i2cScanner(Wire);

  // Serial.println("Setup display running...");

  // // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  // if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C) /*&&
  //     !display.begin(SSD1306_SWITCHCAPVCC, 0x3D)*/) {
  //   Serial.println(F("SSD1306 allocation failed"));
  //   for(;;); // Don't proceed, loop forever
  // }

  // display.clearDisplay();
  // display.setTextSize(1);
  // display.setTextColor(SSD1306_WHITE);
  // display.setCursor(0, 0);
  // display.drawBitmap(0, 0, Tritonbots_Logo, 128, 64, WHITE);
  // display.display();

  delay(1000);
  // display.clearDisplay();

  pinMode(KICKER_PIN, OUTPUT);
  Serial.println("Kicker Pin Set!");
  // display.display();

  delay(1000);

  SPI.begin();
  Serial.println("SPI Begun!");
  // display.display();

  delay(1000);

  // Run CAN-FD at 1 Mbit/s for both arbitration and data.
  ACAN2517FDSettings settings(
    ACAN2517FDSettings::OSC_20MHz, 
    CANFD_BITRATE, 
    DataBitRateFactor::x1
  );

  configCANFDSettings(settings);
  Serial.println("CANFD Config'ed!");
  // display.display();

  delay(1000);

  initPositionCommands(MotorCommands);
  Serial.println("Position Cmds Init!");
  // display.display();

  delay(1000);

  connectWiFi();

  delay(1000);

  udp.begin(UDP_PORT);
  Serial.print(F("UDP Port: "));
  Serial.println(UDP_PORT);
  // display.display();

  delay(1000);

  // // start CAN communication and print error while disconnected
  // const uint32_t errorCode = can.begin(settings, [] {
  //   can.isr();
  // });
  // while (errorCode != 0) {
  //   Serial.print(F("CAN error 0x"));
  //   Serial.println(errorCode, HEX);
  //   delay(1000);
  // }

  // // create motor objects
  
  //  for (int i=0;i<NUM_MOTORS;i++) {
  //     Motors[i] = new Moteus(can, [i]() {
  //        Moteus::Options options;
  //        options.id = i+1;
  //        return options;
  //     }());
  //     // Clear any faults
  //     Motors[i]->BeginStop();
  //  }

  // display.clearDisplay();
  // display.setCursor(0,0);
  Serial.println("Setup Done!");
  // display.display();
  // delay(1000);
  // display.clearDisplay();
  // display.display();
  // delay(1000);
  // display.clearDisplay();
  // display.display();
}

void loop() {
  // Poll for incoming motion and actuator commands over UDP.
  handleUdpPackets(udp, WheelCommands, MotorCommands, lastUdpCommandMs, watchdogStopped);

  const unsigned long now = millis();
  if (!watchdogStopped && lastUdpCommandMs != 0 && (now - lastUdpCommandMs >= WATCHDOG_TIMEOUT)) {
    // Serial.print("\r\33[2K\r"); // clears the line and does a carriage return
    Serial.println(F("WATCHDOG timeout: stopping robot"));
    stop(MotorCommands);
    watchdogStopped = true;
  }

  // Keep a live velocity readout in the serial monitor for debugging.
  // printMotorVelocitiesInline(MotorCommands);
  char buf[128];

  snprintf(buf, sizeof(buf),
    "FL: %.3f\nFR: %.3f\nBR: %.3f\nBL: %.3f\nDribbler: %.3f\n",
    MotorCommands[FL_WHEEL_INDEX]->velocity, 
    MotorCommands[FR_WHEEL_INDEX]->velocity, 
    MotorCommands[BR_WHEEL_INDEX]->velocity, 
    MotorCommands[BL_WHEEL_INDEX]->velocity, 
    MotorCommands[DRIBBLER_INDEX]->velocity
  );

  Serial.print(buf);
  // if (now - displayPageTimer >= 5000) {
  //   displayPageTimer = now;
  //   displayPageCounter++;
  //   if (displayPageCounter >= 2) {
  //     displayPageCounter = 0;
  //   }
  //   switch(displayPageCounter) {
  //     case 0:
  //       robotInfoPage(display,WiFi);
  //       break;
  //     case 1:
  //       positionCommandsPage(display, MotorCommands);
  //       break;
  //     default:
  //       break;
  //   }
  // }
}
