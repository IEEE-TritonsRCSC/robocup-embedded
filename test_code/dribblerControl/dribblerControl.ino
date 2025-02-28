#include <Arduino.h>
#include "driver/mcpwm.h"

#define ESC_GPIO 18  // GPIO pin connected to ESC signal wire

bool flag = false;

void setup() {
    Serial.begin(115200);
    
    // Configure MCPWM for ESC control (50Hz frequency)
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, ESC_GPIO);

    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;  // 50 Hz for ESC (20ms period)
    pwm_config.cmpr_a = 5;  // Start with minimum throttle (5% duty)
    pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

    Serial.println("ESC Initialized. Waiting 3 seconds...");
    delay(3000);  // Give ESC time to initialize
}

// Function to set ESC throttle (0-100%)
void setESCThrottle(float throttle) {
    if (throttle < 0) throttle = 0;
    if (throttle > 100) throttle = 100;

    // Convert throttle % to pulse width (1000µs to 2000µs)
    float pulseWidth = 1000 + (throttle / 100.0) * 1000;  
    float dutyCycle = (pulseWidth / 20000.0) * 100.0;  // Convert to percentage (20ms period)

    Serial.printf("Throttle: %.1f%%, Pulse Width: %.1fµs, Duty Cycle: %.2f%%\n", throttle, pulseWidth, dutyCycle);
    
    // Set duty cycle
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, dutyCycle);
}


void loop() {
    if (!flag) {
      Serial.println("Starting Motor...");

      // Ensure ESC is armed with minimum throttle
      setESCThrottle(0);
      delay(5000);

      setESCThrottle(1);
      delay(100);
      setESCThrottle(2);
      delay(100);
      setESCThrottle(3);

      flag = true;
    }
}
