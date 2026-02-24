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
#define HAL_DELAY 10

static DrivetrainState state;

int main(void) {
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_CAN1_Init();
	MX_TIM1_Init();
	MX_UART4_Init();

	state.can = &hcan1;
	state.uart = &huart4;

	setupMotors(state.motorPins);

	turnLEDsOff();

	HAL_UART_Receive_IT(state.uart, const_cast<uint8_t *>(&state.rx_byte), 1);


	/* Infinite loop */
	while (1) {

		updateDribblerSpeedFromFlag(state.dribble_flag, &state.dribble_speed);

		//applySafetyTimeoutToTargetSpeeds(state.timeout, state.targetSpeeds);
		
		updateMotorPidLoop(state.motor_pids, state.targetSpeeds, state.speed_data);
		

		int16_t speedCommands[NUM_MOTORS] = {
				static_cast<int16_t>(state.motor_pids[0].getOutput()),
				static_cast<int16_t>(state.motor_pids[1].getOutput()),
				static_cast<int16_t>(state.motor_pids[2].getOutput()),
				static_cast<int16_t>(state.motor_pids[3].getOutput()),
				state.dribble_speed
		};
		
		setMotorSpeeds(&state, speedCommands);
		 
		state.timeout++;
		HAL_Delay(HAL_DELAY);
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	handleCanRxFifo0(&state, hcan);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	handleUartRxComplete(&state, huart);
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
