#include "velocityConversions.h"

using namespace std;

void getVelocityArray(array<int, 4>& wheel_speeds, double heading, double vx, double vy, double rotV) {
    // Takes heading, absolute velocity, theta, and rotational velocity
    // rotational velocity as input parameters and returns a byte array.
    
    array<double, 4> wheel_speeds_double;

    // constant
    constexpr double two_pi = 2 * M_PI;

    // difference between current direction (heading) and desired direction (theta)
    // double relativeTheta = fmod((theta - heading + 2 * two_pi), two_pi);

    // add back in vx/vy relative to global heading later
    // double vx = absV * sin(relativeTheta);
    // double vy = absV * cos(relativeTheta);


    // front right wheel 
    wheel_speeds_double[0] = 
        rotV * (sin(FR_WHEEL_ANGLE)*FR_X - cos(FR_WHEEL_ANGLE)*FR_Y) 
        + vx * cos(FR_WHEEL_ANGLE)
        + vy * sin(FR_WHEEL_ANGLE);
    
    wheel_speeds_double[0] /= WHEEL_RADIUS;

    // back right wheel
    wheel_speeds_double[1] = 
        rotV * (sin(BR_WHEEL_ANGLE)*BR_X - cos(BR_WHEEL_ANGLE)*BR_Y) 
        + vx * cos(BR_WHEEL_ANGLE)
        + vy * sin(BR_WHEEL_ANGLE);

    wheel_speeds_double[1] /= WHEEL_RADIUS;
    
    // back left wheel
    wheel_speeds_double[2] = 
        rotV * (sin(BL_WHEEL_ANGLE)*BL_X - cos(BL_WHEEL_ANGLE)*BL_Y) 
        + vx * cos(BL_WHEEL_ANGLE)
        + vy * sin(BL_WHEEL_ANGLE);

    wheel_speeds_double[2] /= WHEEL_RADIUS;

    // front left wheel
    wheel_speeds_double[3] = 
        rotV * (sin(FL_WHEEL_ANGLE)*FL_X - cos(FL_WHEEL_ANGLE)*FL_Y) 
        + vx * cos(FL_WHEEL_ANGLE)
        + vy * sin(FL_WHEEL_ANGLE);

    wheel_speeds_double[3] /= WHEEL_RADIUS;

    for (int i = 0; i < 4; i++) {
      if (wheel_speeds_double[i] > MAX_VELOCITY) {
        wheel_speeds_double[i] = MAX_VELOCITY;
      } else if (wheel_speeds_double[i] < -MAX_VELOCITY) {
        wheel_speeds_double[i] = -MAX_VELOCITY;
      }
    }

    for (int i = 0; i < 4; i++) {
        wheel_speeds_double[i] *= RESCALE_FACTOR;
        wheel_speeds[i] = (int)wheel_speeds_double[i];        
    }
}

void valuesToBytes(array<int, 4>& wheel_speeds, array<uint8_t, 8>& wheel_speeds_byte) {
    
    for (int i = 0; i < 4; i++) {
        wheel_speeds_byte[(i * 2)] = (wheel_speeds[i] >> 8 & 0xff); 
        wheel_speeds_byte[(i * 2 + 1)] = (wheel_speeds[i] & 0xff);
    }

}

void action_to_byte_array(array<uint8_t, 8>& wheel_speeds_byte) {

    array<int, 4> wheel_speeds;

    getVelocityArray(wheel_speeds, 0, 0, 1, 0.5);
    valuesToBytes(wheel_speeds, wheel_speeds_byte);
}