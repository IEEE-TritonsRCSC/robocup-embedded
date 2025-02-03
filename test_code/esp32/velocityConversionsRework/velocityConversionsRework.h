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
double[] wheelAngles = {ANGLE_TR, ANGLE_BR, ANGLE_BL, ANGLE_TL} // 1234

// position of each wheel relative to center
double[] posX = { d * sin(M_PI / 6), d * (-sin(M_PI / 6)),   // 1, 2
                         d * (-sin(M_PI / 6)), d * sin(M_PI / 6) };    // 3, 4
double[] posY = { d * cos(M_PI / 6), d * cos(M_PI / 6),      // 1, 2
                         d * (-cos(M_PI / 6)), d * (-cos(M_PI / 6)) }; // 3, 4


void getVelocityArray(int[] wheel_speeds, double heading, double absV,
                      double theta, double rotV);

void valuesToBytes(int[] wheel_speeds, uint8_t[] wheel_speeds_byte);

void getWheelVelocities(int[] wheel_speeds, 
                        proto_simulation_RobotMoveCommand& action);

void action_to_byte_array(uint8_t[] wheel_speeds_byte,
                          proto_simulation_RobotMoveCommand& action);
