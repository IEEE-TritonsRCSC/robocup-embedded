#define NUM_ACTUATORS 2 // kicker, chipper

#define KICKER_INDEX 0
#define CHIPPER_INDEX 1

#define KICKER_DELAY 100 // keep kicker pin high for delay in ms
#define CHIPPER_DELAY 100 // keep chipper pin high for delay in ms
#define ACTUATOR_ON true

#define KICKER_GPIO_PORT 0
#define KICK_PIN 0
#define CHIPPER_GPIO_PORT 0
#define CHIP_PIN 0

#define BALL_SENSOR_GPIO_PORT 0
#define BALL_SENSOR_PIN 0
#define BALL_DETECTION_DELAY 50 // delay in each ball detection poll in ms 50ms = 20Hz

#define HEARTBEAT_LED_BLINK_DELAY 500 // Heartbeat LED: Toggle every 500ms (1Hz blink)
#define HEARTBEAT_LED_GPIO_PORT 0
#define HEARTBEAT_LED_PIN 0

/**
 * @brief stores activation status for the kicker, dribbler, and chipper
 */
typedef struct ActuatorCommand {
   bool actuatorStatus[NUM_ACTUATORS];
} ActuatorCommand_t;
