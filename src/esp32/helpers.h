#ifndef HELPERS_H
#define HELPERS_H

#include "globals.h"

// Robot
#define FRONT_ANGLE 30		// angle of front wheels (deg)
#define BACK_ANGLE 60		// angle of back wheels (deg)
#define rad_robot 0.086F	// robot radius (m) (from center to wheel contact point)
#define rad_wheel 0.025F 	// wheel radius (m)

// Parsers
void handleNewChar(char c);
void parseMsg(char *msg);
void parseCommand(char *command, char *parameters);

// Executors
void execute_stop();
void execute_turn(float angular_speed);
void execute_dash(float power, float dir);
void execute_skick(float power);
void execute_kick();
void execute_catch();
void command_velocity(float u, float v, float w);
void send_pid_update(uint8_t wheel, float kp, float ki, float kd);
void send_header_config(uint8_t header1, uint8_t header2);

// Hardware Controllers
void setDribbler(float power);
void prepare_and_send_motor_command();

#endif
