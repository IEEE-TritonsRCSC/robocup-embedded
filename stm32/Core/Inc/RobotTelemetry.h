/**
 * @brief stores telemetry data for the robot to send to ESP32
 */
typedef struct {
  float velocities[5];      // Actual velocity from motor IDs 1-5
  uint8_t faults[5];        // Fault codes from motor IDs 1-5
  float battery_voltage;    // Average bus voltage from controllers
  bool ball_sensed;         // Data from the SensorTask
  bool last_kick_ok;        // Feedback from ActuatorTask
} RobotTelemetry_t;