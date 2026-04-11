#define NUM_ACTUATORS 3 // kicker, dribbler, chipper
#define KICKER_INDEX 0
#define DRIBBLER_INDEX 1
#define CHIPPER_INDEX 2

/**
 * @brief stores activation status for the kicker, dribbler, and chipper
 */
typedef struct ActuatorCommand {
   bool actuatorStatus[NUM_ACTUATORS];
};
typedef struct ActuatorCommand_t;