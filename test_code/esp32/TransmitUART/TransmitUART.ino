// Define pins
#define TX 17
#define RX 16

// UART setup
HardwareSerial espSerial(2);

int count;

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200, SERIAL_8N1, RX, TX);

  count = 0;
}

void loop() {
  std::array<unsigned char, 11> msg = {0xca, 0xfe, 0x00, 0x64, 0x00, 0x64, 0x00, 0x64, 0x00, 0x64, 0x11};
  
  espSerial.write(msg.data(), msg.size());

  Serial.printf("(%d) ", count);
  for (int i = 0; i < 11; i++) {
      Serial.printf("%02X ", msg.data()[i]); // Print as uppercase hex with leading zeroes
  }
  Serial.println();

  // espSerial.flush();
  count++;

  delay(1000);

}
