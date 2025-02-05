//#include "velocityConversions.h"
#include <Arduino.h>
#include "PID.h"
#include "testing.h"

PID pid1(10,0,0,1000,8000);
PID pid2(10,0,0,1000,8000);
PID pid3(10,0,0,1000,8000);
PID pid4(10,0,0,1000,8000);


uint8_t wheel1[] = { 0x11, 0x11, 0x0a, 0xbc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t wheel2[] = {0x11, 0x11, 0x00, 0x00, 0x0a, 0xbc, 0x00, 0x00, 0x00, 0x00};
uint8_t wheel3[] = {0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x0a, 0xbc, 0x00, 0x00};
uint8_t wheel4[] = {0x11, 0x11, 0x0a, 0xbc, 0x00, 0x00, 0x00, 0x00, 0x0a, 0xbc};
uint8_t wheelAll[] = {0x11, 0x11, 0x0b, 0xb8, 0x0b, 0xb8, 0x0b, 0xb8, 0x0b, 0xb8};

// uint8_t rmpArray[] = rmpArrayToHex(motorSpeed);
// uint8_t velocities[] = [0x11, 0x11, rmpArray[0], rmpArray[1]];
// print(velocities);


void moveCommands(HardwareSerial& espSerial, char input){
  switch (input) {
    case 'w':
      sendToEmbedded(espSerial,forwards);
      break;
    case 'a':
      sendToEmbedded(espSerial, left);
      break;
    case 's':
      sendToEmbedded(espSerial,backwards);
      break;
    case 'd':
      sendToEmbedded(espSerial,right);
      break;
    case 'q':
      sendToEmbedded(espSerial,counterclockwise);
      break;
    case 'e':
      sendToEmbedded(espSerial,clockwise);
      break;
    default:
      sendToEmbedded(espSerial, stop);
      break;
  }

}


//sends preped messages to embedded
void sendToEmbedded(HardwareSerial& espSerial, const uint8_t * message){
  espSerial.write(message, 12);
  espSerial.flush();
}


