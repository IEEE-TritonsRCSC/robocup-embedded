#include "globals.h"
#include <math.h>

// Robot physical constants
const float FRONT_ANGLE = 60.0;
const float BACK_ANGLE = 45.0;
const float rad_wheel = 0.03;   // Example radius
const float rad_robot = 0.09;   

void blink() {
  digitalWrite(2, HIGH);
  delay(10);
  digitalWrite(2,LOW);
}

void parseMsg(char *msg) {
    char cmd_type[15];
    int n_read = 0;
    
    // Use your RELEVANT_FORMAT to detect commands for this robot ID
    if (sscanf(msg, RELEVANT_FORMAT, cmd_type, &n_read) == 1) {
        char* params = msg + n_read;
        
        if (strcmp(cmd_type, "stop") == 0) {
            memset(&current_cmd, 0, sizeof(current_cmd));
        } 
        else if (cmd_type[0] == 'm') { // Move: m <vel_u> <vel_v> <vel_w>
            float u, v, w;
            if (sscanf(params, " %f %f %f", &u, &v, &w) == 3) {
                calculateKinematics(u, v, w);
            }
        }
        else if (cmd_type[0] == 'k') { // Kick
            current_cmd.kick = 1;
        }
        else if (cmd_type[0] == 'c') { // Chip
            current_cmd.chip = 1;
        }
        else if (cmd_type[0] == 'd') { // Dribbler: d <power>
            float power;
            if (sscanf(params, " %f", &power) == 1) {
                current_cmd.velocities[4] = power; // Dribbler is motor 5
            }
        }
    }
}

void calculateKinematics(float vx, float vy, float wz) {
    float cosF = cosf(FRONT_ANGLE * M_PI/180.0);
    float sinF = sinf(FRONT_ANGLE * M_PI/180.0);
    
    // Standard 4-wheel omni kinematics [4]
    current_cmd.velocities[0] = (-vx * sinF) + (vy * -cosF) + (wz * rad_robot); // FR
    current_cmd.velocities[1] = (vx * sinF) + (vy * -cosF) + (wz * rad_robot);  // BR
    current_cmd.velocities[2] = (vx * sinF) + (vy * cosF) + (wz * rad_robot);   // BL
    current_cmd.velocities[3] = (-vx * sinF) + (vy * cosF) + (wz * rad_robot);  // FL

    // Note: STM32 now handles "kick" and "chip" as event bits, 
    // so we don't clear them here until we actually transmit.
}