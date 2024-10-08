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
void forward(int motorSpeed, int duration);
void backward(int motorSpeed, int duration);
void left(int motorSpeed, int duration);
void right(int motorSpeed, int duration);
void setMotorSpeeds(int16_t ms1, int16_t ms2, int16_t ms3, int16_t ms4);
void runMotors(unsigned char motorOneHigh, unsigned char motorOneLow, unsigned char motorTwoHigh, unsigned char motorTwoLow, unsigned char motorThreeHigh, unsigned char motorThreeLow, unsigned char motorFourHigh, unsigned char motorFourLow);
void kick(int kickDuration);
void Error_Handler(void);

/* USER CODE BEGIN EFP */
#define KEY_Pin GPIO_PIN_10
#define KEY_GPIO_Port GPIOD
#define LED1_Pin GPIO_PIN_14
#define LED1_GPIO_Port GPIOF
#define LED2_Pin GPIO_PIN_11
#define LED2_GPIO_Port GPIOE

#define Motor1_Pin GPIO_PIN_2
#define Motor2_Pin GPIO_PIN_3
#define Motor3_Pin GPIO_PIN_4
#define Motor4_Pin GPIO_PIN_5
#define Motor_Port GPIOH

#define Kicker_Pin GPIO_PIN_1
#define Kicker_Port GPIOC
/* USER CODE BEGIN Private defines */

void dribble() ;
void noDribble() ;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
