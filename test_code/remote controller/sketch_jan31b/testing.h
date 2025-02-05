#ifndef testing_h
#define testing_h

constexpr uint8_t forwards[] = {0xCA, 0xFE, 0x0a, 0xbc, 0x0a, 0xbc, 0xf5, 0x44, 0xf5, 0x44, 0x00};
constexpr uint8_t backwards[] = {0xCA, 0xFE, 0xf5, 0x44, 0xf5, 0x44, 0x0a, 0xbc, 0x0a, 0xbc, 0x00};
constexpr uint8_t left[] = {0xCA, 0xFE, 0xf5, 0x44, 0x0a, 0xbc, 0xf5, 0x44, 0x0a, 0xbc, 0x00};
constexpr uint8_t right[] = {0xCA, 0xFE, 0x0a, 0xbc, 0xf5, 0x44, 0x0a, 0xbc, 0xf5, 0x44, 0x00};
constexpr uint8_t clockwise[] = {0xCA, 0xFE, 0x0a, 0xbc, 0x0a, 0xbc, 0x0a, 0xbc, 0x0a, 0xbc, 0x00};
constexpr uint8_t counterclockwise[] = {0xCA, 0xFE, 0xf5, 0x44, 0xf5, 0x44, 0xf5, 0x44, 0xf5, 0x44, 0x00};
constexpr uint8_t stop[] = {0xCA, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
constexpr uint8_t kick[] = {0xCA, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

void moveCommands(HardwareSerial& espSerial, char input);
void sendToEmbedded(HardwareSerial& espSerial, const uint8_t * message);

#endif