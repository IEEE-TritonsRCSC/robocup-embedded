/* BEGIN INCLUDE */
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
/* END INCLUDE */

/* BEGIN DEFINE */
/* BEGIN ROBOT DEFINE*/
#define ROBOT_ID 1
/* END ROBOT DEFINE*/

/* BEGIN WIFI DEFINE */
#define ENABLE_MULTICAST 0
#define PORT 10000
#define WiFi_SSID "wlan3" // your network SSID (name)
#define PASSWORD "a1b2c3d4" // your network password
/* END WIFI DEFINE*/

/* BEGIN COMMAND DEFINE */
#define DASH_CMD_CHAR 'd'
#define KICK_CMD_CHAR 'k'
#define STOP_CMD_CHAR 's'
#define DROP_CMD_CHAR 'o'
#define CATCH_CMD_CHAR 'c'
#define TURN_CMD_CHAR 't'
#define SHORTKICK_CMD_CHAR 'h'

#define DASH_NUM_ARGS 2
#define TURN_NUM_ARGS 1
#define SHORTKICK_NUM_ARGS 1
#define KICK_NUM_ARGS 0
#define STOP_NUM_ARGS 0
#define DROP_NUM_ARGS 0
#define CATCH_NUM_ARGS 0

// Base components
#define CMD_PREFIX "%d %c" // Every command starts with: [int] [char]
#define TWO_ARGS   "%f %f" // Argument pattern 1
#define ONE_ARG    "%f" // Argument pattern 2

// Combined formats
#define FORMAT_TWO_ARGS CMD_PREFIX " " TWO_ARGS
#define FORMAT_ONE_ARG  CMD_PREFIX " " ONE_ARG
#define FORMAT_NO_ARGS  CMD_PREFIX
/* END COMMAND DEFINE */

/* BEGIN PACKET DEFINE */
#define PACKET_LENGTH 255
// READABLE_PACKET_LENGTH is to prevent buffer overflow if packet is 255 chars long
#define READABLE_PACKET_LENGTH PACKET_LENGTH - 1
/* END PACKET DEFINE */

/* BEGIN TIME DEFINE */
#define SERIAL_TIMEOUT 5000 // time to wait for serial monitor to connect before giving up in milliseconds
#define SERIAL_CNCT_BLINK_DELAY 50 // blink delay for waiting for serial to connect in milliseconds
/* END TIME DEFINE*/

/* BEGIN CONST DEFINE */
#define LED_OFF HIGH
#define LED_ON LOW
#define NO_WIFI_SLEEP false
/**
 * Set WiFi Tx Power to maximum (802.11b 20dBm)
 * Range is usually 8 to 78 (representing 2dBm to 19.5-20dBm)
 * Using WIFI_POWER_19_5dBm is the safest "max" constant.
 */
#define MAX_WIFI_PWR WIFI_POWER_19_5dBm
#define BAUD_RATE 115200 // baud rate should be 115200
#define NUM_WHEELS 4 // number of wheels on the robot
#define NUM_MOTORS 5 // number of motors on the robot
#define MAX_ROBOT_ID 6 // maximum possible robot ID
#define ROBOT_ID_INDEX 0 // index in the command string that contains the robot ID number
#define CMD_CHAR_INDEX 2 // index in the command string that contains the command character
/* END CONST DEFINE */
/* END DEFINE */

/* BEGIN STRUCT DECLARATION */
/**
 * contains parsed data from a 
 */
typedef struct {
  float dashPower; // how fast to dash
  float dashDirection; // which direction to dash
  float turnSpeed; // how fast to turn
  float shortKickPower; // how fast to short kick
  bool kick; // should robot kick?
  bool dribblerCatch; // should robot dribble?
  bool stop; // should robot stop everything?
} CommandPacket_T;
/* END STRUCT DECLARATION */

/* BEGIN VARIABLE DECLARATIONS */
int WiFiStatus = WL_IDLE_STATUS;
char packetBuffer[PACKET_LENGTH]; // buffer to hold incoming packet
char replyBuffer[] = "acknowledged"; // a string to send back
/**
 * list of valid command characters
 * s: stop
 * d: dash
 * t: turn
 * k: kick
 * c: catch
 * o: drop
 * h: short kick
 */
const char validCommandChars[] = {
  STOP_CMD_CHAR,
  DASH_CMD_CHAR,
  KICK_CMD_CHAR,
  CATCH_CMD_CHAR,
  TURN_CMD_CHAR,
  DROP_CMD_CHAR,
  SHORTKICK_CMD_CHAR
};

WiFiUDP Udp; // WiFi UDP object for multicast or unicast
#if ENABLE_MULTICAST == 1
  IPAddress MULTICAST_IP(239, 1, 2, 3); // group IP address for Multicast
#endif
CommandPacket_T commandPacket; // struct to hold data from incoming UDP commmands
/* END VARIABLE DECLARATIONS */

/* BEGIN FUNCTION DECLARATION */
constexpr void initWiFi();
void connectAndStartWiFi(WiFiUDP &Udp, int &WiFiStatus);
void connectSerialOrGiveUp();
void connectWiFi(int &WiFiStatus);
void printWiFiSSID();
void printDeviceIP();
void printWiFiStatus();
void printReceivedSignalStrength();
bool isWiFiConnected(const int WiFiStatus);
void printReceivedPacketSize(const int packetSize);
void blink(const unsigned int blinkDelay_ms);
void ledOFF();
void ledON();
void printPacketContents(char packetBuffer[PACKET_LENGTH]);
void sendReplyPacket(WiFiUDP &Udp, char* replyBuffer);
bool readPacket(WiFiUDP &Udp, char packetBuffer[PACKET_LENGTH], char* replyBuffer);
void sendCommandPacket(CommandPacket_T& commandPacket);
void printUDPSourceIPAndPort(WiFiUDP &Udp);
void checkReceivedPacketExistsAndEndBuffer(const int receivedPacketLength, char packetBuffer[PACKET_LENGTH]);
bool isValidCommandPacket(char packetBuffer[PACKET_LENGTH]);
int numberCharToInt(char c);
bool isValidRobotID(char packetBuffer[PACKET_LENGTH]);
bool isValidCommandChar(char packetBuffer[PACKET_LENGTH]);
bool isMatchingRobotID(char packetBuffer[PACKET_LENGTH]);
bool isValidAndMatchingCommandPacket(char packetBuffer[PACKET_LENGTH]);
/* BEGIN STRUCT FUNCTION DECLARATION */
void clearCommandPacket(CommandPacket_T& commandPacket);
void buildCommandPacket(CommandPacket_T &commandPacket, char packetBuffer[PACKET_LENGTH]);
void setDashCommandPacket(CommandPacket_T &commandPacket, const float power, const float direction);
void setTurnCommandPacket(CommandPacket_T &commandPacket, const float speed);
void setShortKickCommandPacket(CommandPacket_T &commandPacket, const float power);
void setKickCommandPacket(CommandPacket_T &commandPacket);
void clearKickCommandPacket(CommandPacket_T &commandPacket);
void setCatchCommandPacket(CommandPacket_T &commandPacket);
void setStopCommandPacket(CommandPacket_T &commandPacket);
void disableStopCommandPacket(CommandPacket_T &commandPacket);
void setDropCommandPacket(CommandPacket_T &commandPacket);
void print(CommandPacket_T &commandPacket);
/* END STRUCT FUNCTION DECLARATION */
/* END FUNCTION DECLARATION */

void setup() {
  initWiFi();

  pinMode(LED_BUILTIN,OUTPUT);

  Serial.begin(BAUD_RATE);

  connectSerialOrGiveUp();
  
  connectAndStartWiFi(Udp,WiFiStatus);

  Serial.println("Setup Done!");
  ledOFF();
}

void loop() {
  if (readPacket(Udp,packetBuffer,replyBuffer) && isValidAndMatchingCommandPacket(packetBuffer)) {
    ledON();
    clearCommandPacket(commandPacket); // TODO: should it clear or keep the same state?
    buildCommandPacket(commandPacket,packetBuffer);
    print(commandPacket);
    // sendCommandPacket(commandPacket);
    // TODO: handle a command packet with a kick. It should send another command packet right after to turn kick off or make the STM32/hardware handle that?
    ledOFF();
  }
}

/* BEGIN FUNCTION DEFINITION */

/**
 * turn the onboard LED off
 */
void ledOFF() {
  digitalWrite(LED_BUILTIN,LED_OFF);
}

/**
 * turn the onboard LED on
 */
void ledON() {
  digitalWrite(LED_BUILTIN, LED_ON);
}

/**
 * connect to WiFi
 * print WiFi status
 * disable WiFi sleep mode
 * set max WiFi transfer power
 * if ENABLE_MULTICAST is 1 then enable multicast
 * if ENABLE_MULTICAST is 0 then enable unicast
 * @param Udp WiFi UDP object for multicast or unicast
 * @param WiFiStatus current status of wifi connection
 */
void connectAndStartWiFi(WiFiUDP &Udp, int &WiFiStatus) {
  connectWiFi(WiFiStatus);
  printWiFiStatus();
  WiFi.setSleep(NO_WIFI_SLEEP); // prevent high latency and missed packets
  WiFi.setTxPower(MAX_WIFI_PWR);

  #if ENABLE_MULTICAST == 1
    if (Udp.beginMulticast(MULTICAST_IP, PORT)) {
      Serial.println("Joined Multicast Group");
    } else {
      Serial.println("Failed to join Multicast Group");
    }
  #else
    Udp.begin(PORT);
  #endif
}

/**
 * Wait for Serial, but give up after SERIAL_TIMEOUT seconds if no USB host connects
 */
void connectSerialOrGiveUp() {
  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime < SERIAL_TIMEOUT)) {
    blink(SERIAL_CNCT_BLINK_DELAY);
  }
}

/**
 * set the WiFi to station mode, disconnect wifi and erase WiFi Access Point
 * the disconnect is basically a wifi factory reset
 */
constexpr void initWiFi() {
  WiFi.mode(WIFI_STA);         // initialize driver first
  WiFi.disconnect(true, true); // now safe: clears credentials from NVS
}

/**
 * check if incoming command packet is valid and matches this robot's ID
 * @param packetBuffer incoming UDP packet with command
 * @return packetBuffer contains a valid command string and the command's robot ID matches this robot's ID
 */
bool isValidAndMatchingCommandPacket(char packetBuffer[PACKET_LENGTH]) {
  return isValidCommandPacket(packetBuffer) && isMatchingRobotID(packetBuffer);
}

/**
 * check if the command packet inside the packet buffer is valid
 * valid means:
 * - the robot ID is valid
 * - command character is valid
 * - number of arguments is valid for the command
 * @param packetBuffer buffer for receiving UDP packet
 */
bool isValidCommandPacket(char packetBuffer[PACKET_LENGTH]) {
  if (!isValidRobotID(packetBuffer)) { return false; }
  if (!isValidCommandChar(packetBuffer)) { return false; }

  int robot_id;
  char cmd;
  float arg1, arg2;

  // We parse for the maximum possible arguments (2)
  // sscanf returns the total number of successfully matched items
  int found = sscanf(packetBuffer, FORMAT_TWO_ARGS, &robot_id, &cmd, &arg1, &arg2);

  // Validate number of arguments based on the command character
  switch (cmd) {
    case DASH_CMD_CHAR: // this expects 2 args, so should be 4
      return (found == (DASH_NUM_ARGS + 2));
    case TURN_CMD_CHAR:
    case SHORTKICK_CMD_CHAR:
      // These both expect 1 argument, so found should be 3
      return (found == (TURN_NUM_ARGS + 2));
    case KICK_CMD_CHAR:
    case STOP_CMD_CHAR:
    case DROP_CMD_CHAR:
    case CATCH_CMD_CHAR:
      // These expect 0 arguments, so found should be 2 (ID and Char only)
      return (found == (KICK_NUM_ARGS + 2));
    default:
      return false;
  }
}

/**
 * checks if the incoming UDP packet's robot ID matches this robot's ID
 * @param packetBuffer buffer for receiving UDP packet
 */
bool isMatchingRobotID(char packetBuffer[PACKET_LENGTH]) {
  return numberCharToInt(packetBuffer[ROBOT_ID_INDEX]) == ROBOT_ID; 
}

/**
 * check if command char from the incoming command packet is a possible command
 * @param packetBuffer buffer for receiving UDP packet
 * @return the command char from the packet buffer is an element in `validCommandChars`
 */
bool isValidCommandChar(char packetBuffer[PACKET_LENGTH]) {
  char commandChar = packetBuffer[CMD_CHAR_INDEX];
  for (const char c : validCommandChars) {
    if (commandChar == c) {return true;}
  }
  return false;
}

/**
 * check if Robot ID char from the incoming command packet is a possible number
 * valid Robot ID range: (0, MAX_ROBOT_ID]
 * @param packetBuffer buffer for receiving UDP packet
 * @return robot ID integer version of character is greater than 0 and less than or equal to MAX_ROBOT_ID 
 */
bool isValidRobotID(char packetBuffer[PACKET_LENGTH]) {
  int robotID = numberCharToInt(packetBuffer[ROBOT_ID_INDEX]);
  return 0 < robotID && robotID <= MAX_ROBOT_ID;
}

/**
 * converts a number digit character into an integer
 * @param c a character that is a digit from 0 to 9
 */
int numberCharToInt(char c) {
  return c - '0';
}

/**
 * Send raw bytes of command packet over UART
 * @param commandPacket struct containing robot command data
 */
void sendCommandPacket(CommandPacket_T& commandPacket) {
  // Send raw bytes: (pointer to data, size of data in bytes)
  Serial.write((uint8_t*)&commandPacket, sizeof(commandPacket));
}

/**
 * receive an incoming UDP packet and write its content to the packetBuffer
 * commented-out code: print the packet size,IP,Port
 * @param Udp WiFi UDP object
 * @param packetBuffer buffer for receiving UDP packet
 * @param replyBuffer buffer for sending a reply UDP packet to UDP source
 * @retval true means that a packet was read from packet length being greater than 0
 * @retval false means that no packet was read from packet length not being greater than 0
 */
bool readPacket(WiFiUDP &Udp, char packetBuffer[PACKET_LENGTH], char* replyBuffer) {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();

  // if packet has data
  if (packetSize) {
    // printReceivedPacketSize(packetSize);
    // printUDPSourceIPAndPort(Udp);

    // read the packet into packetBufffer
    int receivedPacketLength = Udp.read(packetBuffer, READABLE_PACKET_LENGTH);

    checkReceivedPacketExistsAndEndBuffer(receivedPacketLength, packetBuffer);
  }
  return packetSize;
}

/**
 * print the UDP source's IP address and port number
 * @param Udp WiFi UDP object for the UDP source
 */
void printUDPSourceIPAndPort(WiFiUDP &Udp) {
    // print IP and port of UDP source
    Serial.print("From ");
    Serial.print(Udp.remoteIP());
    Serial.print(", port ");
    Serial.println(Udp.remotePort());
}

/**
 * check if the received UDP packet has a length greater than 0 which means it exists
 * if the UDP packet exists, then put set the last
 *
 * @param receivedPacketLength length of the received UDP packet
 * @param packetBuffer buffer for receiving UDP packet
 */
void checkReceivedPacketExistsAndEndBuffer(const int receivedPacketLength, char packetBuffer[PACKET_LENGTH]) {
  // if packet exists then place null terminator at the end of packet
  if (receivedPacketLength > 0) {
    packetBuffer[receivedPacketLength] = 0;
  }
}

/**
 * prints the contents from the packet buffer
 * @param packetBuffer buffer that holds the string of the received UDP packet contents
 */
void printPacketContents(char packetBuffer[PACKET_LENGTH]) {
  // print contents of received packet
  Serial.println("Contents:");
  Serial.println(packetBuffer);
}

/**
 * sends the string from the reply buffer to the UDP source
 *
 * @param Udp the WiFiUDP object that is connected to the UDP source
 * @param replyBuffer the buffer that contains the reply string to send
 */
void sendReplyPacket(WiFiUDP &Udp, char* replyBuffer) {
  // send a reply, to the IP address and port that sent us the packet we received
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());

  // you can blast the reply to the whole MULTICAST group:
  // Udp.beginPacket(IPAddress(239, 1, 2, 3), PORT);

  Udp.write((uint8_t*)replyBuffer, strlen(replyBuffer));
  Udp.endPacket();
}

/**
 * blink onboard LED for some specified delay
 * @param blinkDelay_ms time between LED turns on and off in milliseconds
 */
void blink(const unsigned int blinkDelay_ms) {
  digitalWrite(LED_BUILTIN, LED_ON);
  delay(blinkDelay_ms);
  digitalWrite(LED_BUILTIN, LED_OFF);
  delay(blinkDelay_ms);
}

/**
 * attempt to connect to WiFi until connected
 *
 * @param WiFiStatus reference to current WiFi status
 * @param WiFiSSID SSID of WiFi that you want to connect to
 */
void connectWiFi(int &WiFiStatus) {
  constexpr unsigned short connectWait_ms = 500;
  constexpr unsigned short connectTimeout_ms = 10000;

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(WiFi_SSID);

  // Begin once, outside the loop
  WiFi.begin(WiFi_SSID, PASSWORD);

  unsigned long startTime = millis();
  while (!isWiFiConnected(WiFiStatus) || WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    WiFiStatus = WiFi.status();
    
    // Timeout and retry if taking too long
    if (millis() - startTime > connectTimeout_ms) {
      Serial.println("Connection timed out, retrying...");
      WiFi.disconnect();
      WiFi.begin(WiFi_SSID, PASSWORD);
      startTime = millis();
    }

    blink(connectWait_ms);
  }

  Serial.println("Connected!");
}

/**
 * check if the device has connected to WiFi
 * @return true if the current WiFi status is connected
 */
bool isWiFiConnected(const int WiFiStatus) {
  return WiFiStatus == WL_CONNECTED;
}

/**
 * print status of the connected WiFi
 * prints SSID, Device IP, and Received Signal Strength
 */
void printWiFiStatus() {
  printWiFiSSID();
  printDeviceIP();
  printReceivedSignalStrength();
}

/**
 * print the SSID of the network you're attached to
 */
void printWiFiSSID() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
}

/**
 * print your WiFi component's IP address
 */
void printDeviceIP() {
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

/**
 * print the received signal strength in units of decibel-milliwatts (dBm)
 */
void printReceivedSignalStrength() {
  Serial.print("signal strength (RSSI):");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

/**
 * print size of received packet size
 * @param packetSize the size of the received packet
 */
void printReceivedPacketSize(const int packetSize) {
  Serial.print("Received packet of size ");
  Serial.println(packetSize);
}
/* BEGIN STRUCT FUNCTION DEFINITION */
/**
 * sets all data in commandPacket to 0.0 and false
 * @param commandPacket reference to a command packet to send over UART 
 */
void clearCommandPacket(CommandPacket_T& commandPacket) {
  commandPacket.dashPower = 0.0;
  commandPacket.dashDirection = 0.0;
  commandPacket.turnSpeed = 0.0;
  commandPacket.shortKickPower = 0.0;
  commandPacket.kick = false;
  commandPacket.dribblerCatch = false;
  commandPacket.stop = false;
}

/** 
 * parses command packet buffer to build a command packet to send over UART to STM32
 * this function expects packet buffer to be a valid command packet
 * @param commandPacket struct containing robot command data
 * @param packetBuffer buffer for receiving UDP packet
 */
void buildCommandPacket(CommandPacket_T &commandPacket, char packetBuffer[PACKET_LENGTH]) {
  if (!isMatchingRobotID(packetBuffer)) {return;}

  int robot_id;
  char cmd;
  float arg1, arg2;
  // We parse for the maximum possible arguments (2)
  // sscanf returns the total number of successfully matched items
  sscanf(packetBuffer, FORMAT_TWO_ARGS, &robot_id, &cmd, &arg1, &arg2);

  switch (cmd) {
    case DASH_CMD_CHAR:
      setDashCommandPacket(commandPacket, arg1, arg2);
      break;
    case TURN_CMD_CHAR:
      setTurnCommandPacket(commandPacket, arg1);
      break;
    case SHORTKICK_CMD_CHAR:
      setShortKickCommandPacket(commandPacket, arg1);
      break;
    case KICK_CMD_CHAR:
      setKickCommandPacket(commandPacket);
      break;
    case STOP_CMD_CHAR:
      setStopCommandPacket(commandPacket);
      break;
    case DROP_CMD_CHAR:
      setDropCommandPacket(commandPacket);
      break;
    case CATCH_CMD_CHAR:
      setCatchCommandPacket(commandPacket);
      break;
    // HOW TO ADD NEW COMMAND:
    // add new command character define
    // add new num of args define
    // add to array of valid commands
    // add case for new command character
    // add new set command packet for the specified command
    // break
    default:
      break;
  }
}

/**
 * sets the dash power and direction in command packet and disable stop
 * @param commandPacket struct containing robot command data
 * @param power dash power
 * @param direction direction to dash in radians
 */
void setDashCommandPacket(
  CommandPacket_T &commandPacket, 
  const float power, 
  const float direction) 
{
  commandPacket.dashPower = power;
  commandPacket.dashDirection = direction;
  disableStopCommandPacket(commandPacket);
}
/**
 * sets the turn speed in command packet and disable stop
 * @param commandPacket struct containing robot command data
 * @param speed speed to rotate in degrees per second
 */
void setTurnCommandPacket(CommandPacket_T &commandPacket, const float speed) {
  commandPacket.turnSpeed = speed;
  disableStopCommandPacket(commandPacket);
}
/**
 * sets the short kick power and set dribblerCatch to false in command packet
 * disable stop
 * this should stop the dribbler's forward rotation and rotate it backwards at some `power`
 * @param commandPacket struct containing robot command data
 * @param power power or speed to spin the dribbler motor backwards
 */
void setShortKickCommandPacket(CommandPacket_T &commandPacket, const float power) {
  commandPacket.shortKickPower = power;
  commandPacket.dribblerCatch = false;
  disableStopCommandPacket(commandPacket);
}
/**
 * set the kick true in command packet and disable stop
 * @param commandPacket struct containing robot command data
 */
void setKickCommandPacket(CommandPacket_T &commandPacket) {
  commandPacket.kick = true;
  disableStopCommandPacket(commandPacket);
}
/**
 * set kick flag in the command packet to false
 * @param commandPacket struct containing robot command data
 */
void clearKickCommandPacket(CommandPacket_T &commandPacket) {
  commandPacket.kick = false;
}
/**
 * set dribblerCatch true in command packet and disable stop
 * note that you cannot name it "catch" because that is a reserved keyword
 * @param commandPacket struct containing robot command data
 */
void setCatchCommandPacket(CommandPacket_T &commandPacket) {
  commandPacket.dribblerCatch = true;
  disableStopCommandPacket(commandPacket);
}
/**
 * set stop true in command packet
 * @param commandPacket struct containing robot command data
 */
void setStopCommandPacket(CommandPacket_T &commandPacket) {
  commandPacket.stop = true;
}
/**
 * set stop false in command packet
 * @param commandPacket struct containing robot command data
 */
void disableStopCommandPacket(CommandPacket_T &commandPacket) {
  commandPacket.stop = false;
}
/**
 * set dribblerCatch false in command packet
 * @param commandPacket struct containing robot command data
 */
void setDropCommandPacket(CommandPacket_T &commandPacket) {
  commandPacket.dribblerCatch = false;
}

/**
 * prints the contents of a built command packet
 * @param commandPacket struct containg command packet data
 */
void print(CommandPacket_T &commandPacket) {
  Serial.print("Dash Power: ");Serial.println(commandPacket.dashPower);
  Serial.print("Dash Direction: ");Serial.println(commandPacket.dashDirection);
  Serial.print("Turn Speed: ");Serial.println(commandPacket.turnSpeed);
  Serial.print("Short Kick Power: ");Serial.println(commandPacket.shortKickPower);
  Serial.print("Catch: ");Serial.println(commandPacket.dribblerCatch);
  Serial.print("Kick: ");Serial.println(commandPacket.kick);
  Serial.print("Stop: ");Serial.println(commandPacket.stop);
}
/* END STRUCT FUNCTION DEFINITION */
/* END FUNCTION DEFINITION */
