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

// Motor objects are created only when CAN output is enabled.
static Moteus* Motors[NUM_MOTORS]{ nullptr };
static unsigned long lastUdpCommandMs = 0;
static bool watchdogStopped = false;
// static unsigned long displayPageTimer = 0;
// static unsigned short displayPageCounter = 0;
static constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 60000;
static int status = WL_IDLE_STATUS;
static unsigned long lastVelocityPrintMs = 0;

// One cached command object per motor slot.
static PositionCommand FrontLeftWheelCmd;
static PositionCommand FrontRightWheelCmd;
static PositionCommand BackRightWheelCmd;
static PositionCommand BackLeftWheelCmd;
static PositionCommand DribblerCmd;

#if ENABLE_TEST_MOTORS == 0
  // Full robot layout: four wheels plus one dribbler.
  static PositionCommand* MotorCommands[NUM_MOTORS] = {
    &FrontLeftWheelCmd,
    &FrontRightWheelCmd,
    &BackRightWheelCmd,
    &BackLeftWheelCmd,
    &DribblerCmd
  };

  // Locomotion-only view used by dash/turn commands.
  static PositionCommand* WheelCommands[NUM_WHEELS] = {
    &FrontLeftWheelCmd,
    &FrontRightWheelCmd,
    &BackRightWheelCmd,
    &BackLeftWheelCmd,
  };
#else
  // Test layout: compile only the first N slots to save memory.
  static PositionCommand* MotorCommands[NUM_MOTORS] = {
    #if NUM_TEST_MOTORS >= 1
    &FrontLeftWheelCmd,
    #endif
    #if NUM_TEST_MOTORS >= 2
    &FrontRightWheelCmd,
    #endif
    #if NUM_TEST_MOTORS >= 3
    &BackRightWheelCmd,
    #endif
    #if NUM_TEST_MOTORS >= 4
    &BackLeftWheelCmd,
    #endif
    #if NUM_TEST_MOTORS == 5
    &DribblerCmd
    #endif
  };

  static PositionCommand* WheelCommands[NUM_WHEELS] = {
    #if NUM_TEST_MOTORS >= 1
    &FrontLeftWheelCmd,
    #endif
    #if NUM_TEST_MOTORS >= 2
    &FrontRightWheelCmd,
    #endif
    #if NUM_TEST_MOTORS >= 3
    &BackRightWheelCmd,
    #endif
    #if NUM_TEST_MOTORS >= 4
    &BackLeftWheelCmd,
    #endif
  };
#endif

static void connectWiFi() {
  // Apply the fixed IP configuration before joining the network.
  WiFi.config(LOCAL_IP_ADDRESS, GATEWAY_IP_ADDRESS, GATEWAY_IP_ADDRESS, SUBNET_MASK);
  Serial.println(F("WiFi.config() applied"));
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
  Serial.print(F("Gateway: "));
  Serial.println(WiFi.gatewayIP());
  Serial.print(F("Subnet: "));
  Serial.println(WiFi.subnetMask());
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());
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

  // Configure the CAN-FD driver for the drivetrain bus.
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

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }
  
  Serial.println("WiFi Module Communication succeeded!");
  
  delay(1000);

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  Serial.println("WiFi firmware is up to date!");

  delay(1000);

  // Connect to Wi-Fi before opening the UDP socket.
  connectWiFi();
  status = WiFi.status();
  if (status != WL_CONNECTED) {
    Serial.println(F("WiFi failed to connect; UDP will not receive packets."));
    while (true) {
      delay(1000);
    }
  }
  
  delay(1000);

  udp.begin(UDP_PORT);
  Serial.print(F("UDP Port: "));
  Serial.println(UDP_PORT);
  // display.display();

  delay(1000);

  #if ENABLE_MOTORS == 1
    // Start CAN communication and keep retrying until the bus responds.
    const uint32_t errorCode = can.begin(settings, [] {
      can.isr();
    });
    while (errorCode != 0) {
      Serial.print(F("CAN error 0x"));
      Serial.println(errorCode, HEX);
      if (errorCode == 0x01) {
        Serial.println("No Motor Connected!");
      }
      delay(1000);
    }

    // Create one motor object per configured slot.
    for (int i = 0; i < NUM_MOTORS; i++) {
      Motors[i] = new Moteus(can, [i]() {
        Moteus::Options options;
        options.id = i + 1;
        return options;
      }());
      // Clear any faults
      Motors[i]->BeginStop();
    }
  #endif

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
  unsigned long now = millis();

  // Poll for incoming motion and actuator commands over UDP.
  handleUdpPackets(udp, WheelCommands, MotorCommands, lastUdpCommandMs, watchdogStopped);

  // Refresh the clock after packet handling so the watchdog uses the latest time.
  now = millis();

  // If commands stop arriving, force the robot back to a safe stopped state.
  if (!watchdogStopped && lastUdpCommandMs != 0 && (now - lastUdpCommandMs >= WATCHDOG_TIMEOUT)) {
    Serial.println(F("WATCHDOG timeout: stopping robot"));
    #if ENABLE_MOTORS == 1
      stop(MotorCommands);
    #endif
    watchdogStopped = true;
  }

  #if ENABLE_MOTORS == 1
    sendPositionCommands(Motors, MotorCommands);
  #endif

  // Emit a periodic snapshot to help confirm the latest command state over Serial.
  if (now - lastVelocityPrintMs >= 1000) {
    lastVelocityPrintMs = now;
    Serial.print(F("Velocities @ "));
    Serial.print(now);
    Serial.print(F(" ms -> "));
    printMotorVelocities(MotorCommands);
  }
}
