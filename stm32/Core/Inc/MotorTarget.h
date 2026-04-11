#ifndef NUM_MOTORS
#define NUM_MOTORS 5
#endif

struct MotorTarget {
   float velocities[NUM_MOTORS];
};
typedef struct MotorTarget MotorTarget_t;