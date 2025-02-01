// Define pins
#define TX 17
#define RX 16

// UART setup
HardwareSerial espSerial(2);

volatile bool received_interrupt = false;
int transmitCount = 0;
int receiveCount = 0;
std::array<unsigned char, 11> msg = {0xca, 0xfe, 
                                     0x00, 0x64, 0x00, 0x64, 
                                     0x00, 0x64, 0x00, 0x64, 
                                     0x11};

void IRAM_ATTR handleInterrupt();
inline void readReceivedData();
inline void readSentData();

void setup() {

  Serial.begin(115200);
  espSerial.begin(115200, SERIAL_8N1, RX, TX);

  pinMode(16, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RX),
                  handleInterrupt,
                  FALLING);
}

void loop() {

  // receive
  if (received_interrupt) {
    readReceivedData();
    received_interrupt = false;
  }
  
  // send
  espSerial.write(msg.data(), msg.size());
  readSentData();


  delay(2000);

}

void IRAM_ATTR handleInterrupt() {
  received_interrupt = true;
}

inline void readReceivedData() {

  Serial.printf("RECEIVE (%d) ", receiveCount++);
  
  unsigned char receivedMsg[11] = {0};  // Buffer for received data

  // Wait until we have at least 11 bytes in the buffer
  uint32_t startTime = millis();
  while (espSerial.available() < 11) {
    if (millis() - startTime > 100) { // Timeout after 100ms
      Serial.println("Timeout waiting for data.");
      return;
    }
  }

  // Read exactly 11 bytes
  espSerial.readBytes(receivedMsg, 11);

  // Print received data as hex
  for (int i = 0; i < 11; i++) {
    Serial.printf("%02X ", receivedMsg[i]);
  }
  
  Serial.println();
}

inline void readSentData() {

  Serial.printf("TRANSMIT (%d) ", transmitCount++);
  for (int i = 0; i < 11; i++) {
      Serial.printf("%02X ", msg.data()[i]); // Print as uppercase hex with leading zeroes
  }

  Serial.println();
}





