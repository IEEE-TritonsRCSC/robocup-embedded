
#include "MotorControl.h"

ACAN2517FD can(MCP2517_CS, SPI, MCP2517_INT);

/**
   @note motor indices 0-3 are wheel motors
   @note motor index 4 is the dribbler motor
*/
Moteus* motors[NUM_MOTORS] = {nullptr};

/**
   @brief angle of wheel relative to the front to back axis in radians
   @note order: FrontLeft, FrontRight, BackRight, FrontLeft
   @note front angle is 20 degrees and back angle is 60 degrees
*/
float angles[NUM_WHEELS] = {
   0.349066,
   -0.349066,
   1.0472,
   -1.0472,
};

/**
   @brief should the velocity of the motor be inverted rotation
*/
bool invert[NUM_WHEELS] = {
   true,
   false,
   false,
   true
};


void setup() {
  pinMode(KICKER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {}
  Serial.println(F("started"));

  SPI.begin();
   
  // Don't touch anything below

  // Run CAN-FD at 1 Mbit/s for both arbitration and data.
  ACAN2517FDSettings settings(
      ACAN2517FDSettings::OSC_20MHz, CANFD_BITRATE, DataBitRateFactor::x1);

  // Keep the driver buffers small to fit on memory-constrained boards.
  settings.mArbitrationSJW = 2;
  settings.mDriverTransmitFIFOSize = 1;
  settings.mDriverReceiveFIFOSize = 2;

  const uint32_t errorCode = can.begin(settings, [] { can.isr(); });
  while (errorCode != 0) {
    Serial.print(F("CAN error 0x"));
    Serial.println(errorCode, HEX);
    delay(1000);
  }

   for (int i=0;i<TEST_NUM_MOTORS;i++) {
      motors[i] = new Moteus(can, [i]() {
         Moteus::Options options;
         options.id = i+1;
         return options;
      }());
      // Clear any faults
      motors[i]->BeginStop();
   }

   // Don't touch anything above

}

void loop() {
   stop(motors, KICKER_PIN);
   //turn(motors,TURN_SPEED);
   //dash(motors,angles,invert,DASH_POWER,DASH_ANGLE);
}
