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
#include "main.h"
#include "can.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;

uint32_t TxMailbox;

uint8_t TxData[8];
uint8_t RxData[8];
/* USER CODE BEGIN PV */

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
int main(void)
{
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
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  //can and motors setup

  if (HAL_CAN_Start(&hcan1) != HAL_OK)
  {
	  Error_Handler();
  }

  TxHeader.DLC = 8;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.StdId = 0x200;
  TxHeader.TransmitGlobalTime = DISABLE;

  HAL_GPIO_TogglePin(Motor_Port, Motor1_Pin);
  HAL_GPIO_TogglePin(Motor_Port, Motor2_Pin);
  HAL_GPIO_TogglePin(Motor_Port, Motor3_Pin);
  HAL_GPIO_TogglePin(Motor_Port, Motor4_Pin);

  //forward(1000, 3000);
  backward(1000, 3000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

	 HAL_Delay(500);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/*
if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY)!= HAL_OK){
	  Error_Handler();
}

TxData[0] = 0b0001111101000000 >> 8;
  TxData[1] = 0b0001111101000000;
  TxData[2] = 0b0001111101000000 >> 8;
  TxData[3] = 0b0001111101000000;
  TxData[4] = 0b0001111101000000 >> 8;
  TxData[5] = 0b0001111101000000;
  TxData[6] = 0b0001111101000000 >> 8;
  TxData[7] = 0b0001111101000000;

  if(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0){
	  		if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
	  	  	{
	  			Error_Handler ();
	  	  		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
	  	  	}
	  	  	else{
	  	  		HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
	  	  	}
	  	  }
	  	  else{
	  	  	HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
	  	}

*/

void forward(int motorSpeed, int runDuration){
	TxData[0] = motorSpeed >> 8;
	TxData[1] = motorSpeed;
	TxData[2] = (-1 * motorSpeed) >> 8;
	TxData[3] = (-1 * motorSpeed);
	TxData[4] = motorSpeed >> 8;
	TxData[5] = motorSpeed;
	TxData[6] = (-1 * motorSpeed) >> 8;
	TxData[7] = (-1 * motorSpeed);

	int i = 0;
	while(i < runDuration){
		HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	    HAL_Delay(0.5);
	    i++;
	}
}

void backward(int motorSpeed, int runDuration){
	TxData[0] = (-1 * motorSpeed) >> 8;
	TxData[1] = (-1 * motorSpeed);
	TxData[2] = motorSpeed >> 8;
	TxData[3] = motorSpeed;
	TxData[4] = (-1 * motorSpeed) >> 8;
	TxData[5] = (-1 * motorSpeed);
	TxData[6] = motorSpeed >> 8;
	TxData[7] = motorSpeed;

	int i = 0;
	while(i < runDuration){
		HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	    HAL_Delay(0.5);
	    i++;
	}
}

void turn(int motor1Speed, int motor2Speed, int motor3Speed, int motor4Speed, int runDuration){
	TxData[0] = motor1Speed >> 8;
	TxData[1] = motor1Speed;
	TxData[2] = motor2Speed >> 8;
	TxData[3] = motor2Speed;
	TxData[4] = motor3Speed >> 8;
	TxData[5] = motor3Speed;
	TxData[6] = motor4Speed >> 8;
	TxData[7] = motor4Speed;

	int i = 0;
	while(i < runDuration){
		HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
		HAL_Delay(0.5);
		i++;
	}
}

void kick(int kickDuration){

}

void charge(int chargeDuration){

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
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
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
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
