#include <ctype.h>

typedef struct pid_state_struct {
	uint16_t previous_error;
	uint16_t sum_error;
} pid_history;



uint8_t update_pid(pid_state* pid_state, uint16_t speed_expected, uint16_t speed_actual) {
	uint16_t ctrl_err = speed_expected - speed_actual;
	uint16_t err_delta = ctrl_err - previous_error;
	pid_state->previous_error = ctrl_err;
	pid_state->sum_error += ctrl_err;
	return PID_P_GAIN * ctrl_err + PID_D_GAIN * err_delta + PID_I_GAIN * pid_state->sum_error;
}

void update_pid_arr(pid_state* pid_states, ) {

}
