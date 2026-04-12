
/**
 * @brief stores command to trigger solenoid actuators
 */
typedef struct {
	/**
	 * command to actuate kicker
	 */
	bool kick;
	/**
	 * command to actuate chipper
	 */
	bool chip;
} SolenoidTrigger_t;
