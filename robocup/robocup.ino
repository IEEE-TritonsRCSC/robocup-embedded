/**
 * @file robocup.ino
 * @brief Main firmware entry point for the RoboCup robot controller.
 *
 * Sets up serial, GPIO, Wi-Fi, UDP, and CAN-related state, then processes
 * incoming UDP commands in the main loop.
 */

#include "commands.h"
#include "credentials.h"
#include "oled.h"

#define BAUD_RATE 115200

static WiFiUDP udp;
static ACAN2517FD can(MCP2517_CS, SPI, MCP2517_INT);
static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static unsigned short displayY = 0;
static char* WiFiThrobberText[4] = {
  "WiFi Connecting |",
  "WiFi Connecting /",
  "WiFi Connecting -",
  "WiFi Connecting \\"
};
static unsigned short throbberIndex = 0;

static Moteus* Motors[NUM_MOTORS]{ nullptr };
static Moteus* Wheels[NUM_WHEELS]{ nullptr };
static unsigned long lastUdpCommandMs = 0;
static bool watchdogStopped = false;
static unsigned long displayPageTimer = 0;
static unsigned short displayPageCounter = 0;

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

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("Serial Started!");

  Wire.begin();

  i2cScanner(Wire);

  Serial.println("Setup display running...");

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C) /*&&
      !display.begin(SSD1306_SWITCHCAPVCC, 0x3D)*/) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.drawBitmap(0, 0, Tritonbots_Logo, 128, 64, WHITE);
  display.display();

  delay(1000);
  display.clearDisplay();

  pinMode(KICKER_PIN, OUTPUT);
  display.println("Kicker Pin Set!");
  display.display();

  delay(1000);

  SPI.begin();
  display.println("SPI Begun!");
  display.display();

  delay(1000);

  // Run CAN-FD at 1 Mbit/s for both arbitration and data.
  ACAN2517FDSettings settings(
    ACAN2517FDSettings::OSC_20MHz, 
    CANFD_BITRATE, 
    DataBitRateFactor::x1
  );

  configCANFDSettings(settings);
  display.println("CANFD Config'ed!");
  display.display();

  delay(1000);

  initPositionCommands(MotorCommands);
  display.println("Position Cmds Init!");
  display.display();

  delay(1000);

  WiFi.config(LOCAL_IP_ADDRESS, GATEWAY_IP_ADDRESS, SUBNET_MASK);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    display.setCursor(0,0);
    display.clearDisplay();
    if (throbberIndex >= 3) {throbberIndex = 0;}
    display.print(WiFiThrobberText[throbberIndex++]);
    display.display();
  }

  display.clearDisplay();

  display.setCursor(0,0);
  display.println("WiFi Connected: ");
  display.println(WiFi.localIP());
  display.display();

  delay(1000);

  udp.begin(UDP_PORT);
  display.println(F("UDP Port: "));
  display.println(UDP_PORT);
  display.display();

  delay(1000);

  // start CAN communication and print error while disconnected
  /* const uint32_t errorCode = can.begin(settings, [] {
    can.isr();
  });
  while (errorCode != 0) {
    Serial.print(F("CAN error 0x"));
    Serial.println(errorCode, HEX);
    delay(1000);
  } */

  // create motor objects
  /*
   for (int i=0;i<NUM_MOTORS;i++) {
      Motors[i] = new Moteus(can, [i]() {
         Moteus::Options options;
         options.id = i+1;
         return options;
      }());
      // Clear any faults
      Motors[i]->BeginStop();
   } */

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Setup Done!");
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();
}

void loop() {
  // Poll for incoming motion and actuator commands over UDP.
  handleUdpPackets(udp, WheelCommands, MotorCommands, lastUdpCommandMs, watchdogStopped);

  const unsigned long now = millis();
  if (!watchdogStopped && lastUdpCommandMs != 0 && (now - lastUdpCommandMs >= WATCHDOG_TIMEOUT)) {
    Serial.print("\r\33[2K\r"); // clears the line and does a carriage return
    Serial.println(F("WATCHDOG timeout: stopping robot"));
    stop(MotorCommands);
    watchdogStopped = true;
  }

  // Keep a live velocity readout in the serial monitor for debugging.
  printMotorVelocitiesInline(MotorCommands);
  
  if (now - displayPageTimer >= 5000) {
    displayPageTimer = now;
    displayPageCounter++;
    if (displayPageCounter >= 2) {
      displayPageCounter = 0;
    }
    switch(displayPageCounter) {
      case 0:
        robotInfoPage(display,WiFi);
        break;
      case 1:
        positionCommandsPage(display, MotorCommands);
        break;
      default:
        break;
    }
  }
}
