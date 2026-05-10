/**
 * This file is to make the onboard LED easier to control
 * This has functions and defines for more readable usage
 */
#pragma once

// BEGIN INCLUDES
#include "Arduino.h"
// END INCLUDES

// BEGIN DEFINES
#define LED_ON LOW
#define LED_OFF HIGH
// END DEFINES

// BEGIN FUNCTION DECLARATION
/**
 * @brief set the pin `LED_BUILTIN` to `OUTPUT` mode
 * this initializes the LED to power it and make it blink
 */
void initLED();

/**
 * @brief initializes the onboard LED to off
 */
void initLEDoff();

/**
 * @brief initializes the onboard LED to on
 */
void initLEDon();

/**
 * @brief turn the onboard LED on
 */
void LEDon();

/**
 * @brief turn the onboard LED off
 */
void LEDoff();

/**
 * @brief blink the onboard LED with a blocking delay for a specified duration with a specified number of blinks. The number of blinks is 1 by default
 * @param duration how long the LED stays on and off in milliseconds (0-65535)
 * @param numBlinks how many times the LED should blink. Defaults to 1
 * @note blocking means that all processes stop during the delay
 */
void blocking_blink(const unsigned short duration, const unsigned short numBlinks = 1);
// END FUNCTION DECLARATION