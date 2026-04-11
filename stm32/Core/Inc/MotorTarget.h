#ifndef NUM_MOTORS
#define NUM_MOTORS 5
#endif

/**
 * @brief struct for 5 target velocities for each motor from an ESP32 command
 * 
 * @note `velocities` is 5 elements, so the motor target queue size is 20 bytes 
 * (4 byte float * 5 motors = 20 bytes)
 */
typedef struct MotorTarget {
   float velocities[NUM_MOTORS];
}; MotorTarget_t;