// Define pins
#define TX 17
#define RX 16
#define MESSAGE_LENGTH 11

// UART setup
HardwareSerial espSerial(2);

int count = 0;
bool fast = true;
uint8_t fast_msg[MESSAGE_LENGTH] = {0xca, 0xfe, 0x03, 0xe8, 0x03, 0xe8, 0x03, 0xe8, 0x03, 0xe8, 0x01};
uint8_t slow_msg[MESSAGE_LENGTH] = {0xca, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0, 0x00, 0x00, 0x00};

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200, SERIAL_8N1, RX, TX);
}

void loop() {

  count++;
  Serial.printf("(%d) ", count);


  if (fast) {
    espSerial.write(fast_msg, MESSAGE_LENGTH);

    
    for (int i = 0; i < 11; i++) {
      Serial.printf("%02x ", fast_msg[i]); // Print as uppercase hex with leading zeroes
    }

    Serial.println();
    
    fast = false;
    delay(1000);

  } else {
    espSerial.write(slow_msg, MESSAGE_LENGTH);

    
    for (int i = 0; i < 11; i++) {
      Serial.printf("%02x ", slow_msg[i]); // Print as uppercase hex with leading zeroes
    }
    Serial.println();
    
    fast = true;
    delay(1000);
  }
  
}