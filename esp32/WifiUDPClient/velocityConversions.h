#include<cmath>
#include<array>
#include<iostream>
#include "ssl_simulation_robot_control.pb.h"

void getVelocityArray(std::array<int, 4>& wheel_speeds, double heading, double absV,
                      double theta, double rotV);

void valuesToBytes(std::array<int, 4>& wheel_speeds, std::array<unsigned char, 8>& wheel_speeds_byte);

void getWheelVelocities(std::array<int, 4>& wheel_speeds, 
                        proto_simulation_RobotMoveCommand& action);

void action_to_byte_array(std::array<unsigned char, 8>& wheel_speeds_byte,
                          proto_simulation_RobotMoveCommand& action);
