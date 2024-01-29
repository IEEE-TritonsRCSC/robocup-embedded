/*
 * pid.h
 *
 *  Created on: Jan 28, 2024
 *      Author: ryan
 */

#ifndef INC_PID_H_
#define INC_PID_H_

uint16_t update_pid(pid_state* pid_state, uint16_t speed_expected, uint16_t speed_actual);

#endif /* INC_PID_H_ */
