/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */

void forward(int motorSpeed, int duration);
void backward(int motorSpeed, int duration);
void left(int motorSpeed, int duration);
void right(int motorSpeed, int duration);
void setMotorSpeeds(int16_t ms1, int16_t ms2, int16_t ms3, int16_t ms4);
void runMotors(unsigned char motorOneHigh, unsigned char motorOneLow, unsigned char motorTwoHigh, unsigned char motorTwoLow, unsigned char motorThreeHigh, unsigned char motorThreeLow, unsigned char motorFourHigh, unsigned char motorFourLow);
void kick(int kickDuration);
void Error_Handler(void);

void dribble();
void noDribble();

/* USER CODE END EFP */

/* USER CODE BEGIN Private defines */

#define KEY_PIN GPIO_PIN_10
#define KEY_PORT GPIOD
#define LED_GREEN_PIN GPIO_PIN_14
#define LED_GREEN_PORT GPIOF
#define LED_RED_PIN GPIO_PIN_11
#define LED_RED_PORT GPIOE

/*
 * motor 1 = front right motor
 * motor 2 = back right motor
 * motor 3 = back left motor
 * motor 4 = front left motor
 */

#define MOTOR1_PIN GPIO_PIN_2
#define MOTOR2_PIN GPIO_PIN_3
#define MOTOR3_PIN GPIO_PIN_4
#define MOTOR4_PIN GPIO_PIN_5
#define MOTOR_PORT GPIOH

#define KICKER_PIN GPIO_PIN_1
#define KICKER_PORT GPIOC

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
