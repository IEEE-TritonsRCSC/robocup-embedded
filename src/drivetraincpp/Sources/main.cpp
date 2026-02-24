/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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

#include <string.h>
#include "main.h"
#include "can.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "PID_Data.h"
#include "PID_Gains.h"
#include "CanHeader.h"
#include "helpers.h"


#define REDUCTION_RATIO 36.0

static DrivetrainState state;

int main(void) {
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	initializePeripherals();

	state.can = &hcan1;
	state.uart = &huart4;

	setupMotors(state.motorPins);

	turnLEDsOff();

	HAL_UART_Receive_IT(state.uart, const_cast<uint8_t *>(&state.rx_byte), 1);


	/* Infinite loop */
	while (1) {

		updateDribblerSpeedFromFlag(&state);

		//applySafetyTimeoutToTargetSpeeds(state.timeout, state.targetSpeeds);
		
		updateMotorPidLoop(&state);

		updateMotorCommandsTimeoutAndDelay(&state);
	}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
extern "C" void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
