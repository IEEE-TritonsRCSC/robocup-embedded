// Use Serial2 for demonstration
HardwareSerial leSerial(2);

#define TX 17 // TX=GPIO17
#define RX 16 // RX=GPIO16

#define HEADER_BYTE_1 0xCA
#define HEADER_BYTE_2 0xFE

void setup() {
    // Initialize serial ports
    Serial.begin(115200);  // Debugging
    leSerial.begin(115200, SERIAL_8N1, RX, TX);

    Serial.println("Ready to read 11-byte array from Serial2.");
}

void loop() {

    // Read from UART2 -- properly aligns messages with 0xCA 0xFE header
    if (leSerial.available() >= 11) {
      if (leSerial.read() == HEADER_BYTE_1 && leSerial.read() == HEADER_BYTE_2) {
          uint8_t byteArray[9];
          leSerial.readBytes(byteArray, 9);

          // Print the received data for debugging
          Serial.printf("0x%02X 0x%02X ", HEADER_BYTE_1, HEADER_BYTE_2);
          for (int i = 0; i < 9; i++) {
            Serial.printf("0x%02X ", byteArray[i]); // Print as hex
          }
          Serial.println();
      }
    }
}
