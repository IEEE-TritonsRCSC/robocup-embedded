#include <Arduino.h>
#include "driver/mcpwm.h"

#define SERVO_GPIO 18  // GPIO pin connected to servo

void setup() {
    Serial.begin(115200);
    
    // Configure MCPWM unit 0, timer 0, operator 0
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, SERVO_GPIO);

    // Initialize MCPWM with 50Hz frequency (20ms period)
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;  // 50 Hz for servo control
    pwm_config.cmpr_a = 0;  // Initial duty cycle
    pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
}

// Function to set servo angle (0° to 180°)
void setServoAngle(float angle) {
    // Convert angle to pulse width (500-2500µs)
    float pulseWidth = 500 + (angle / 180.0) * 2000;  // Scale 0-180° to 500-2500µs
    float dutyCycle = (pulseWidth / 20000.0) * 100.0;  // Convert to percentage (20ms period)
    
    Serial.printf("Angle: %.1f°, Pulse Width: %.1fµs, Duty Cycle: %.2f%%\n", angle, pulseWidth, dutyCycle);
    
    // Set duty cycle
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, dutyCycle);
}

void loop() {

    for (int angle = 0; angle <= 180; angle += 45) {  // Move from 0° to 180°
        
        setServoAngle(angle);
        delay(1000);
    }
    for (int angle = 180; angle >= 0; angle -= 45) {  // Move back from 180° to 0°
        setServoAngle(angle);
        delay(1000);
    }
}
