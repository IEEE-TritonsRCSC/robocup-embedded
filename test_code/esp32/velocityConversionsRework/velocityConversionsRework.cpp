#include "velocityConversionsRework.h"

// Work In Progress...

/* 
 * Takes heading, absolute vleocity, theta, and rotation velocity as input 
 * params. Returns a byte array consisting of individual wheel velocities.
 * Respective order: top right, bottom right, bottom left, top left.
*/

// See if we can make int* wheel_speeds short* instead
void getVelocityArray(int[] wheel_speeds, double heading, 
                             double absVel, double theta, double rotVel) {

    double wheel_speeds_double[4];

    // current - desired orientation. aka, heading - theta.
    double relAngleOffset = fmod((theta - heading + (2 * twoPI)), twoPI);

    // determine x/y velocities relative to current orientation (heading).
    double velX = absV * sin(angularOffset);
    double velY = absV * cos(relativeTheta);

    // set wheel velocities based on desired angle
    for (int i = 0; i < 4; i++) {

        // apply formula
        wheel_speeds_double[i] = (velX - rotVel * posY[i]) * cos(wheelAngles[i])
                               + (velY + rotVel * posX[i]) * sin(wheelAngles[i]);

        // m/s to RPM conversions
        wheel_speeds_double[i] /= MS_TO_RPM;

        // account for gear ratio
        wheel_speeds_double[i] *= 36;
    }

    // rescaling such that no wheel velocity exceeds our max rpm
    double rescale = 1;
    for (int i = 0; i < 4; i++) {
        rescale = max(rescale, abs(wheel_speeds_double[i] / maxRPM);
    }

    for (int i = 0; i < 4; i++) {
        wheel_speeds_double[i] /= rescale;
        wheel_speeds[i] = (int)wheel_speeds_double[i];        
    }

}

void valuesToBytes(int[] wheel_speeds, uint8_t[] wheel_speeds_byte) {
    
    for (int i = 0; i < 4; i++) {
        wheel_speeds_byte[(i * 2)] = (wheel_speeds[i] >> 8 & 0xff); 
        wheel_speeds_byte[(i * 2 + 1)] = (wheel_speeds[i] & 0xff);
    }

}

void getWheelVelocities(int[] wheel_speeds, 
                        proto_simulation_RobotMoveCommand& action) {

    proto_simulation_MoveLocalVelocity& local_v = action.command.local_velocity;

    double absV = sqrt(local_v.forward * local_v.forward + local_v.left * local_v.left);
    double theta = atan2(local_v.forward, local_v.left); // Using forward as y, left as x
    double rotV = local_v.angular;

    getVelocityArray(wheel_speeds, 0, absV, theta, rotV);
}

void action_to_byte_array(uint8_t[] wheel_speeds_byte,
                          proto_simulation_RobotMoveCommand& action) {

    array<int, 4> wheel_speeds;
    getWheelVelocities(wheel_speeds, action);
    valuesToBytes(wheel_speeds, wheel_speeds_byte);
}
