// Work In Progress...
#include<cmath>
#include<array>
#include<iostream>
#include "ssl_simulation_robot_control.pb.h"

constexpr double twoPI = 2 * M_PI;
constexpr double maxRPM = 15000;
constexpr double d = 0.13;
constexpr double r = 0.05;

constexpr double ANGLE_TR = -M_PI/3;
constexpr double ANGLE_BR = M_PI/6;
constexpr double ANGLE_BL = -M_PI/6;
constexpr double ANGLE_TL = M_PI/3;

constexpr double MS_TO_RPM = r / 60;

// wheel angles RELATIVE TO THE X AXIS
std::array<double, 4> wheelAngles = {ANGLE_TR, ANGLE_BR, ANGLE_BL, ANGLE_TL}; // 1234

// position of each wheel relative to center
std::array<double, 4> posX = { d * sin(M_PI / 6), d * (-sin(M_PI / 6)),   // 1, 2
                         d * (-sin(M_PI / 6)), d * sin(M_PI / 6) };    // 3, 4
std::array<double, 4> posY = { d * cos(M_PI / 6), d * cos(M_PI / 6),      // 1, 2
                         d * (-cos(M_PI / 6)), d * (-cos(M_PI / 6)) }; // 3, 4


void getVelocityArray(std::array<int, 4>& wheel_speeds, double heading, double absV,
                      double theta, double rotV);

void valuesToBytes(std::array<int, 4>& wheel_speeds, std::array<uint8_t, 8>& wheel_speeds_byte);

void getWheelVelocities(std::array<int, 4>& wheel_speeds, _RobotMoveCommand& action);

void action_to_byte_array(std::array<uint8_t, 8>& wheel_speeds_byte, _RobotMoveCommand& action);
