// Define pins
#define xTX 17
#define xRX 16

#include <Arduino.h>

TaskHandle_t SenderHandle = NULL;
TaskHandle_t ReceiverHandle = NULL;
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

// Sender task: Runs on Core 0
void Sender(void *pvParameters) {
    Serial.println("\nHello from Sender! (Core 0)");
    while (1) {
        espSerial.write(msg.data(), msg.size());
        readSentData();
        vTaskDelay(pdMS_TO_TICKS(2000));  // Delay for 1 second
    }
}

// Receiver task: Runs on Core 1
void Receiver(void *pvParameters) {
    Serial.println("Hello from Receiver! (Core 1)");
    while (1) {
        if (received_interrupt) {
            readReceivedData();
            received_interrupt = false;
        }
    }
}

void setup() {

    Serial.begin(115200);
    espSerial.begin(115200, SERIAL_8N1, xRX, xTX);

    pinMode(xRX, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(xRX),
                    handleInterrupt,
                    FALLING);
    delay(1000);  // let things intialize

    // Create Sender task on Core 0
    xTaskCreatePinnedToCore(
        Sender,           // Task function
        "Sender",         // Task name
        1000,             // Stack size (in words)
        NULL,             // Task input parameter
        1,                // Priority (higher = more important)
        &SenderHandle,    // Task handle
        0                 // Core 0
    );

    delay(1000);

    // Create Receiver task on Core 1
    xTaskCreatePinnedToCore(
        Receiver,         // Task function
        "Receiver",       // Task name
        1000,             // Stack size (in words)
        NULL,             // Task input parameter
        3,                // Priority
        &ReceiverHandle,  // Task handle
        1                 // Core 1
    );
}

void loop() {
    // The loop is empty because tasks run independently.
}


void IRAM_ATTR handleInterrupt() {
    received_interrupt = true;
}

inline void readReceivedData() {

    Serial.printf("\nRECEIVER (%d) ", receiveCount++);
  
    unsigned char receivedMsg[11] = {0};  // Buffer for received data

    // Wait until we have at least 11 bytes in the buffer
    uint32_t startTime = millis();
    while (espSerial.available() < 11) {
        if (millis() - startTime > 100) { // Timeout after 100ms
            Serial.print("Timeout waiting for data.");
            return;
        }
    }

    // Read exactly 11 bytes
    espSerial.readBytes(receivedMsg, 11);

    // Print received data as hex
    for (int i = 0; i < 11; i++) {
        Serial.printf("%02X ", receivedMsg[i]);
    }
  
}

inline void readSentData() {

    Serial.printf("\nSENDER (%d) ", transmitCount++);
    for (int i = 0; i < 11; i++) {
        Serial.printf("%02X ", msg.data()[i]); // Print as uppercase hex with leading zeroes
    }

}
