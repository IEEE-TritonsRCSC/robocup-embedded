/*
 * pid.h
 *
 *  Created on: Jan 28, 2024
 *      Author: ryan
 */

#ifndef INC_PID_H_
#define INC_PID_H_

// TODO: Update with better values
#define PID_P_GAIN = 0.2;
#define PID_D_GAIN = 0.2;
#define PID_I_GAIN = 0.2;


uint16_t update_pid(pid_state* pid_state, uint16_t speed_expected, uint16_t speed_actual);

#endif /* INC_PID_H_ */
