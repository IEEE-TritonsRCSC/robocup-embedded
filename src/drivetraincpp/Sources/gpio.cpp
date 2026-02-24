/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    gpio.c
 * @brief   This file provides code for the configuration
 *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif

void configureMotorPins(GPIO_InitTypeDef* GPIO_InitStruct) {
	GPIO_InitStruct->Pin = MOTOR1_PIN | MOTOR2_PIN | MOTOR3_PIN | MOTOR4_PIN | MOTOR5_PIN;
	GPIO_InitStruct->Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct->Pull = GPIO_NOPULL;
	GPIO_InitStruct->Speed = GPIO_SPEED_FREQ_LOW;
}

void enableGPIOClocks() {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
}

void configureKeyPin(GPIO_InitTypeDef* GPIO_InitStruct)
{
	GPIO_InitStruct->Pin = KEY_PIN;
	GPIO_InitStruct->Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct->Pull = GPIO_NOPULL;
}

void configureGreenLED(GPIO_InitTypeDef* GPIO_InitStruct) {
	GPIO_InitStruct->Pin = LED_GREEN_PIN;
	GPIO_InitStruct->Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct->Pull = GPIO_NOPULL;
	GPIO_InitStruct->Speed = GPIO_SPEED_FREQ_LOW;
}

void configureRedLED(GPIO_InitTypeDef *GPIO_InitStruct) {
	GPIO_InitStruct->Pin = LED_RED_PIN;
	GPIO_InitStruct->Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct->Pull = GPIO_NOPULL;
	GPIO_InitStruct->Speed = GPIO_SPEED_FREQ_LOW;
}

void configureGreenLEDOutputLevel() {
	HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);
}

void configureRedLEDOutputLevel() {
	HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
}

void configureMotorPinsOutputLevel()
{
	HAL_GPIO_WritePin(MOTOR_PORT, MOTOR1_PIN | MOTOR2_PIN | MOTOR3_PIN | MOTOR4_PIN | MOTOR5_PIN, GPIO_PIN_RESET);
}

void configurePinsOutputLevel() {
	configureGreenLEDOutputLevel();
	configureRedLEDOutputLevel();
	configureMotorPinsOutputLevel();
}

void configureAndInitializePins(GPIO_InitTypeDef *GPIO_InitStruct) {
	configureMotorPins(GPIO_InitStruct);
	HAL_GPIO_Init(MOTOR_PORT, GPIO_InitStruct);

	configureKeyPin(GPIO_InitStruct);
	HAL_GPIO_Init(KEY_PORT, GPIO_InitStruct);

	configureGreenLED(GPIO_InitStruct);
	HAL_GPIO_Init(LED_GREEN_PORT, GPIO_InitStruct);

	configureRedLED(GPIO_InitStruct);
	HAL_GPIO_Init(LED_RED_PORT, GPIO_InitStruct);
}

void configureOutputLevelsAndInitializePins(GPIO_InitTypeDef *GPIO_InitStruct) {
	configurePinsOutputLevel();
	configureAndInitializePins(GPIO_InitStruct);
}

void MX_GPIO_Init(void) {

	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	enableGPIOClocks();

	configureOutputLevelsAndInitializePins(&GPIO_InitStruct);
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
