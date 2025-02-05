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

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "can.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "PID.h"

#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_RX_BUFFER_SIZE 9   // set to the size we want to limit receive messages to
#define UART_TX_BUFFER_SIZE 12  // set to the size we want to limit send messages to
#define HEADER_BYTE_1 0xCA
#define HEADER_BYTE_2 0xFE
#define KICK 0x14
#define REDUCTION_RATIO 36.0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

// CAN variables
CAN_TxHeaderTypeDef canTxHeader;
CAN_RxHeaderTypeDef canRxHeader;
uint32_t canTxMailbox;
uint8_t CAN_TxData[8];
uint8_t CAN_RxData[8];

// PID feedback variables
volatile uint8_t motor_idx;
volatile uint16_t angle_data[4];
volatile float speed_data[4];
volatile float torque_current_data[4];
volatile float targetSpeeds[4];
PID_TypeDef motor_pid[4];

volatile float Kp1 = 1;
volatile float Ki1 = 0.4;
volatile float Kd1 = 0;
volatile float Kp2 = 1;
volatile float Ki2 = 0.4;
volatile float Kd2 = 0;
volatile float Kp3 = 1;
volatile float Ki3 = 0.4;
volatile float Kd3 = 0;
volatile float Kp4 = 1;
volatile float Ki4 = 0.4;
volatile float Kd4 = 0;

// UART setup
uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE]; // buffer that stores in an array of characters user inputs, aka a string
uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];

volatile uint8_t rx_byte = 0;
volatile int header1_flag = 0;
volatile int header2_flag = 0;

volatile int timeout;  // timeout for safety mechanism to shutoff robot

volatile int kickFlag; // flag for kicking

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */

int main(void) {
	/* USER CODE BEGIN 1 */
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_CAN1_Init();
	MX_TIM1_Init();
	MX_USART2_UART_Init();
	/* USER CODE BEGIN 2 */

	// Motor setup
	HAL_GPIO_TogglePin(MOTOR_PORT, MOTOR1_PIN);
	HAL_GPIO_TogglePin(MOTOR_PORT, MOTOR2_PIN);
	HAL_GPIO_TogglePin(MOTOR_PORT, MOTOR3_PIN);
	HAL_GPIO_TogglePin(MOTOR_PORT, MOTOR4_PIN);

	// CAN setup
	canTxHeader.DLC = 8;
	canTxHeader.IDE = CAN_ID_STD;
	canTxHeader.RTR = CAN_RTR_DATA;
	canTxHeader.StdId = 0x200;
	canTxHeader.TransmitGlobalTime = DISABLE;

	// PID Setup
	for (int i = 0; i < 4; ++i) {
		speed_data[i] = 0;
	}
	pid_init(&motor_pid[0], 9999, 1000, 20, 0, Kp1, Ki1, Kd1);
	pid_init(&motor_pid[1], 9999, 1000, 20, 0, Kp2, Ki2, Kd2);
	pid_init(&motor_pid[2], 9999, 1000, 20, 0, Kp3, Ki3, Kd3);
	pid_init(&motor_pid[3], 9999, 1000, 20, 0, Kp4, Ki4, Kd4);

	for (int i = 0; i < 4; ++i) {
		targetSpeeds[i] = 0;
	}

	// Start program with LEDs off
	HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, LED_OFF);
	HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, LED_OFF);

	HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		if (kickFlag == 1) {               // Triggers a kick
			kick(20);
			kickFlag = 0;
		}

		if (timeout >= 500) {             // Safety timeout when UART disconnects
			for (int i = 0; i < 4; ++i) {
				targetSpeeds[i] = 0;
			}
		}

		 for (int i = 0; i < 4; ++i) {                          // PID control loop
			 motor_pid[i].target = targetSpeeds[i];
			 pid_calculate(&motor_pid[i],speed_data[i]);
		 }

		 setMotorSpeeds((motor_pid[0].output), (motor_pid[1].output), (motor_pid[2].output), (motor_pid[3].output));


//	  int feedbackSize = sizeof(speed_data);
//	  uint8_t feedbackBuffer[(feedbackSize+2)];
//	  feedbackBuffer[0] = 0xB0;
//	  feedbackBuffer[1] = 0xBA;
//
//	  for (int i = 0; i < (sizeof(speed_data) / sizeof(float)); i++)
//	  {
//		  uint8_t *floatBytes = (uint8_t *)&speed_data[i];
//		  feedbackBuffer[2 + (i * 4) + 0] = floatBytes[0];
//		  feedbackBuffer[2 + (i * 4) + 1] = floatBytes[1];
//		  feedbackBuffer[2 + (i * 4) + 2] = floatBytes[2];
//		  feedbackBuffer[2 + (i * 4) + 3] = floatBytes[3];
//	  }
//
//	  HAL_UART_Transmit_DMA(&huart2, feedbackBuffer, sizeof(feedbackBuffer));
		timeout++;
		HAL_Delay(10);
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		/* USER CODE END 3 */
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	//HAL_GPIO_TogglePin(LED_GREEN_PORT,LED_GREEN_PIN);
	if (hcan == &hcan1) {
		HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &canRxHeader, CAN_RxData);

		if (canRxHeader.StdId == 0x201)
			motor_idx = 0;
		if (canRxHeader.StdId == 0x202)
			motor_idx = 1;
		if (canRxHeader.StdId == 0x203)
			motor_idx = 2;
		if (canRxHeader.StdId == 0x204)
			motor_idx = 3;

		angle_data[motor_idx] = (uint16_t) (CAN_RxData[0] << 8 | CAN_RxData[1]);
		speed_data[motor_idx] = ((int16_t) (CAN_RxData[2] << 8 | CAN_RxData[3])
				/ REDUCTION_RATIO); // angular velocity in rpm
		torque_current_data[motor_idx] = (CAN_RxData[4] << 8 | CAN_RxData[5]);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	/*
	 * Called when uart_rx_buffer is full
	 */

	if (!header1_flag) {
		if (rx_byte == HEADER_BYTE_1) {
			// first header byte received

			header1_flag = 1;
			HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
			return;
		}
	} else {

		if (header2_flag) {
			// full message received

			timeout = 0;

			targetSpeeds[0] = (int16_t)((uart_rx_buffer[0] << 8) | uart_rx_buffer[1]);
			targetSpeeds[1] = (int16_t)((uart_rx_buffer[2] << 8) | uart_rx_buffer[3]);
			targetSpeeds[2] = (int16_t)((uart_rx_buffer[4] << 8) | uart_rx_buffer[5]);
			targetSpeeds[3] = (int16_t)((uart_rx_buffer[6] << 8) | uart_rx_buffer[7]);

			/*
			 * Previously |targetSpeeds[i]| <= 500
			 */

			if (uart_rx_buffer[9] == KICK) {
				 kickFlag = 1;
			}

			for (int i = 0; i < UART_RX_BUFFER_SIZE; ++i) {
				uart_rx_buffer[i] = 0;
			}

			header1_flag = header2_flag = 0;

			HAL_GPIO_TogglePin(LED_GREEN_PORT, LED_GREEN_PIN);
			HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
			return;

		} else if (rx_byte == HEADER_BYTE_2) {
			// second header byte received

			header2_flag = 1;
			HAL_UART_Receive_IT(&huart2, uart_rx_buffer, UART_RX_BUFFER_SIZE);
			return;
		} else {
			// first header byte received but not followed by second header byte

			header1_flag = 0;
			HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
			return;
		}
	}
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 6;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void setMotorSpeeds(int16_t ms1, int16_t ms2, int16_t ms3, int16_t ms4) {
	uint8_t h1 = ms1 >> 8;
	uint8_t l1 = ms1;
	uint8_t h2 = ms2 >> 8;
	uint8_t l2 = ms2;
	uint8_t h3 = ms3 >> 8;
	uint8_t l3 = ms3;
	uint8_t h4 = ms4 >> 8;
	uint8_t l4 = ms4;
	runMotors(h1, l1, h2, l2, h3, l3, h4, l4);
}

void runMotors(unsigned char motorOneHigh, unsigned char motorOneLow,
		unsigned char motorTwoHigh, unsigned char motorTwoLow,
		unsigned char motorThreeHigh, unsigned char motorThreeLow,
		unsigned char motorFourHigh, unsigned char motorFourLow) {
	//speed can be 16 bits, split into high and low bytes
	CAN_TxData[0] = motorOneHigh; //high byte for speed, shifted 8 because only buffer is only 8 bits
	CAN_TxData[1] = motorOneLow;       //low bytes for speed
	CAN_TxData[2] = motorTwoHigh;
	CAN_TxData[3] = motorTwoLow;
	CAN_TxData[4] = motorThreeHigh;
	CAN_TxData[5] = motorThreeLow;
	CAN_TxData[6] = motorFourHigh;
	CAN_TxData[7] = motorFourLow;
	HAL_CAN_AddTxMessage(&hcan1, &canTxHeader, CAN_TxData, &canTxMailbox);
}

void kick(int kickDuration) {
	HAL_GPIO_WritePin(KICKER_PORT, KICKER_PIN, GPIO_PIN_SET);
	HAL_Delay(kickDuration);
	HAL_GPIO_WritePin(KICKER_PORT, KICKER_PIN, GPIO_PIN_RESET);
}

/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
