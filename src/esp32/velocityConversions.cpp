#include "velocityConversions.h"

using namespace std;

void getVelocityArray(array<int, 4>& wheel_speeds, double heading, double absV,
                      double theta, double rotV) {
    // Takes heading, absolute velocity, theta, and rotational velocity
    // rotational velocity as input parameters and returns a byte array.
    
    array<double, 4> wheel_speeds_double;

    // constants
    double twoPI = 2 * M_PI;
    double maxRPM = 15000;
    double d = 0.13;
    double r = 0.05;

    // difference between current direction (heading) and desired direction (theta)
    double relativeTheta = fmod((theta - heading + 2 * twoPI), twoPI);

    /*
        I THINK vx AND vy SHOULD POSSIBLY BE SWITCHED
            e.g. vx = absV * sin(relativeTheta)
                 vy = absV * cos(relativeTheta)
        BECAUSE THETA IS RELATIVE TO UP/NORTH
    */
    // double vx = absV * cos(relativeTheta);
    // double vy = absV * sin(relativeTheta);

    double vx = absV * sin(relativeTheta);
    double vy = absV * cos(relativeTheta);


    // angle of each wheel relative to the x-axis
    array<double, 4> B = {-M_PI/6, M_PI/6, 5*M_PI/6, 7*M_PI/6};
    
    // position of each wheel relative to center
    array<double, 4> y = {d*cos(M_PI/6), d*cos(M_PI/6), d*(-cos(M_PI/6)), d*(-cos(M_PI/6))};
    array<double, 4> x = {d*sin(M_PI/6), d*(-sin(M_PI/6)), d*(-sin(M_PI/6)), d*sin(M_PI/6)};

    // set wheel velocities based on desired angle
    for (int i = 0; i < 4; i++) {

        // apply formula
        wheel_speeds_double[i] = (vx - rotV * y[i]) * cos(B[i]) + (vy + rotV * x[i]) * sin(B[i]);

        // convert from m/s to RPM
        wheel_speeds_double[i] /= r / 60;

        // account for gear ratio
        wheel_speeds_double[i] *= 36;
    }

    // rescale so that no wheel velocity exceeds our max RPM
    double rescale = 1;

    for (int i = 0; i < 4; i++) {
        rescale = max(rescale, abs(wheel_speeds_double[i]) / maxRPM);
    }

    for (int i = 0; i < 4; i++) {
        wheel_speeds_double[i] /= rescale;
        wheel_speeds[i] = (int)wheel_speeds_double[i];        
    }
}

void valuesToBytes(array<int, 4>& wheel_speeds, array<uint8_t, 8>& wheel_speeds_byte) {
    
    for (int i = 0; i < 4; i++) {
        wheel_speeds_byte[(i * 2)] = (wheel_speeds[i] >> 8 & 0xff); 
        wheel_speeds_byte[(i * 2 + 1)] = (wheel_speeds[i] & 0xff);
    }

}

void getWheelVelocities(array<int, 4>& wheel_speeds, 
                        proto_simulation_RobotMoveCommand& action) {

    proto_simulation_MoveLocalVelocity& local_v = action.command.local_velocity;

    double absV = sqrt(local_v.forward * local_v.forward + local_v.left * local_v.left);
    double theta = atan2(local_v.forward, local_v.left); // Using forward as y, left as x
    double rotV = local_v.angular;

    getVelocityArray(wheel_speeds, 0, absV, theta, rotV);
}

void action_to_byte_array(array<uint8_t, 8>& wheel_speeds_byte,
                          proto_simulation_RobotMoveCommand& action) {

    array<int, 4> wheel_speeds;
    getWheelVelocities(wheel_speeds, action);
    valuesToBytes(wheel_speeds, wheel_speeds_byte);
}
