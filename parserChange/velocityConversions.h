#include<cmath>
#include<array>
#include<iostream>
#include<climits>

// wheel constants
constexpr double WHEEL_RADIUS = 0.02425;
constexpr double GEAR_RATIO = 36;
constexpr double MAX_RPM = 15000;
constexpr double MAX_VELOCITY = 175;
constexpr double RESCALE_FACTOR = MAX_RPM/MAX_VELOCITY;

// front right wheel constants
constexpr double FR_X = 0.09 * cos(M_PI/6);
constexpr double FR_Y = 0.09 * sin(M_PI/6);
constexpr double FR_WHEEL_ANGLE = -(M_PI/3);

// back right wheel constants
constexpr double BR_X = 0.09 * cos(-(M_PI/3));
constexpr double BR_Y = 0.09 * sin(-(M_PI/3));
constexpr double BR_WHEEL_ANGLE = -5 * (M_PI/6);

// back left wheel constants
constexpr double BL_X = 0.09 * cos(-4 * (M_PI/6));
constexpr double BL_Y = 0.09 * sin(-4 * (M_PI/6));
constexpr double BL_WHEEL_ANGLE = 5 * (M_PI/6);

// front left wheel constants
constexpr double FL_X = 0.09 * cos(5 * (M_PI/6));
constexpr double FL_Y = 0.09 * sin(5 * (M_PI/6));
constexpr double FL_WHEEL_ANGLE = M_PI/3;

void getVelocityArray(std::array<int, 4>& wheel_speeds, double heading, double absV,
                      double theta, double rotV);

void valuesToBytes(std::array<int, 4>& wheel_speeds, std::array<unsigned char, 8>& wheel_speeds_byte);

void action_to_byte_array(std::array<unsigned char, 8>& wheel_speeds_byte);
