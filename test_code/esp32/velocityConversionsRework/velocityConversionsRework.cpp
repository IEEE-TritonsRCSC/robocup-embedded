#include "velocityConversionsRework.h"

// WIP...

// Try to put all of these constexprs anywhere but inside the functio
/* 
 * Takes heading, absolute vleocity, theta, and rotation velocity as input 
 * params. Returns a byte array consisting of individual wheel velocities.
 * Respective order: top right, bottom right, bottom left, top left.
*/

// See if we can make int* wheel_speeds short* instead
// wheel_speeds is returned
void getVelocityArray(std::array<int, 4>& wheel_speeds, double heading, 
                             double absV, double theta, double rotV) {

    std::array<double, 4> wheel_speeds_double;

    // current - desired orientation. aka, heading - theta.
    double relativeAngleOffset = fmod((theta - heading + (2 * twoPI)), twoPI);

    // determine x/y velocities relative to current orientation (heading).
    double velX = absV * sin(relativeAngleOffset);
    double velY = absV * cos(relativeAngleOffset);

    // set wheel velocities based on desired angle
    for (int i = 0; i < 4; i++) {

        // apply formula
        wheel_speeds_double[i] = (velX - rotV * posY[i]) * cos(wheelAngles[i])
                               + (velY + rotV * posX[i]) * sin(wheelAngles[i]);

        // m/s to RPM conversions
        wheel_speeds_double[i] /= MS_TO_RPM;

        // account for gear ratio
        wheel_speeds_double[i] *= 36;
    }

    // rescaling such that no wheel velocity exceeds our max rpm
    double rescale = 1;
    for (int i = 0; i < 4; i++) {
        rescale = std::max(rescale, std::abs(wheel_speeds_double[i] / maxRPM));
    }

    for (int i = 0; i < 4; i++) {
        wheel_speeds_double[i] /= rescale;
        wheel_speeds[i] = (int)wheel_speeds_double[i];        
    }

}

void valuesToBytes(std::array<int, 4>& wheel_speeds, std::array<uint8_t, 8>& wheel_speeds_byte) {
    
    for (int i = 0; i < 4; i++) {
        wheel_speeds_byte[(i * 2)] = (wheel_speeds[i] >> 8 & 0xff); 
        wheel_speeds_byte[(i * 2 + 1)] = (wheel_speeds[i] & 0xff);
    }

}

void getWheelVelocities(std::array<int, 4>& wheel_speeds, _RobotMoveCommand& action) {

    _MoveLocalVelocity& local_v = action.command.local_velocity;

    double absV = sqrt(local_v.forward * local_v.forward + local_v.left * local_v.left);
    double theta = atan2(local_v.forward, local_v.left); // Using forward as y, left as x
    double rotV = local_v.angular;

    getVelocityArray(wheel_speeds, 0, absV, theta, rotV);
}

void action_to_byte_array(std::array<uint8_t, 8>& wheel_speeds_byte, _RobotMoveCommand& action) {

    std::array<int, 4> wheel_speeds;
    getWheelVelocities(wheel_speeds, action);
    valuesToBytes(wheel_speeds, wheel_speeds_byte);
}
