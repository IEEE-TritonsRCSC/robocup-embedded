/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    gpio.h
 * @brief   This file contains all the function prototypes for
 *          the gpio.c file
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "helpers.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/**
 * @brief Enable clocks for the GPIO ports used by the application.
 */
void enableGPIOClocks();
/**
 * @brief Configure the GPIO pins used to drive the motors.
 *
 * @param GPIO_InitStruct Pointer to a GPIO init structure to populate.
 */
void configureMotorPins(GPIO_InitTypeDef* GPIO_InitStruct);
/**
 * @brief Configure the GPIO pin used for the key/button input.
 *
 * @param GPIO_InitStruct Pointer to a GPIO init structure to populate.
 */
void configureKeyPin(GPIO_InitTypeDef* GPIO_InitStruct);
/**
 * @brief Configure the GPIO pin used for the green status LED.
 *
 * @param GPIO_InitStruct Pointer to a GPIO init structure to populate.
 */
void configureGreenLED(GPIO_InitTypeDef *GPIO_InitStruct);
/**
 * @brief Configure the GPIO pin used for the red status LED.
 *
 * @param GPIO_InitStruct Pointer to a GPIO init structure to populate.
 */
void configureRedLED(GPIO_InitTypeDef *GPIO_InitStruct);
/**
 * @brief Configure and initialize all application GPIO pins.
 *
 * @param GPIO_InitStruct Pointer to a GPIO init structure to populate.
 */
void configureAndInitializePins(GPIO_InitTypeDef *GPIO_InitStruct);
/**
 * @brief Set the initial output level for the green status LED.
 */
void configureGreenLEDOutputLevel();
/**
 * @brief Set the initial output level for the red status LED.
 */
void configureRedLEDOutputLevel();
/**
 * @brief Set the initial output level for the motor control pins.
 */
void configureMotorPinsOutputLevel();
/**
 * @brief Set the initial output levels for all configured output pins.
 */
void configurePinsOutputLevel();
/**
 * @brief Set output levels and initialize all application GPIO pins.
 *
 * @param GPIO_InitStruct Pointer to a GPIO init structure to populate.
 */
void configureOutputLevelsAndInitializePins(GPIO_InitTypeDef *GPIO_InitStruct);
/**
 * @brief Initialize all configured GPIO peripherals and pins.
 */
void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

