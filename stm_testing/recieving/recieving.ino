// Use Serial2 for demonstration
HardwareSerial Serial2(2);

void setup() {
    // Initialize serial ports
    Serial.begin(115200);  // Debugging
    Serial2.begin(9600, SERIAL_8N1, 16, 17); // TX=GPIO16, RX=GPIO17

    Serial.println("Ready to read 11-byte array from Serial2.");
}

void loop() {
    if (Serial2.available() >= 11) {  // Check if at least 11 bytes are available
        uint8_t byteArray[11];        // Create an array to store the data
        Serial2.readBytes(byteArray, 11); // Read 11 bytes into the array

        // Print the received data for debugging
        for (int i = 0; i < 11; i++) {
            Serial.printf("0x ", byteArray[i]); // Print as hex
        }
        Serial.println();
    }
}