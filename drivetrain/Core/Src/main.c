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
#include <string.h>
#include "PID.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

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
#define NUM_MOTORS 4

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//CAN variables
CAN_TxHeaderTypeDef canTxHeader;
CAN_RxHeaderTypeDef canRxHeader;
uint32_t canTxMailbox;
uint8_t CAN_TxData[8];
uint8_t CAN_RxData[8];
static volatile uint8_t motor_idx;
static volatile uint16_t angle_data[4];
static volatile int16_t speed_data[4];
static volatile float torque_current_data[4];

//UART variables
unsigned char runMotorHeader = 0x01;
unsigned char dribblerHeader = 0x02;
uint8_t uart_rx_buffer[9]; //buffer that stores in an array of characters user inputs, aka a string
uint8_t uart_tx_buffer[33];
int ms_to_listen = 4000;  //set the number of ms we keep the uart line in receive mode for

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  HAL_UART_Receive_IT(&huart2, uart_rx_buffer, 8);
}

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
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  //Motor setup
  HAL_GPIO_TogglePin(Motor_Port, Motor1_Pin);
  HAL_GPIO_TogglePin(Motor_Port, Motor2_Pin);
  HAL_GPIO_TogglePin(Motor_Port, Motor3_Pin);
  HAL_GPIO_TogglePin(Motor_Port, Motor4_Pin);

  //CAN setup
  HAL_CAN_Start(&hcan1); //start CAN
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING); // Activate CAN receive interrupt for encoder data
  canTxHeader.DLC = 8;
  canTxHeader.IDE = CAN_ID_STD;
  canTxHeader.RTR = CAN_RTR_DATA;
  canTxHeader.StdId = 0x200;
  canTxHeader.TransmitGlobalTime = DISABLE;

  // Structure to store PID data and pointer to PID structure.
  // Prepare one for each motor.
  struct pid_controller pid_controllers[NUM_MOTORS];
  pid_t pid[NUM_MOTORS];

  // Inputs, outputs, and setpoints for each motor
  float motor_input[NUM_MOTORS] = {0}; // from encoder
  float motor_output[NUM_MOTORS] = {0}; // to motor
  float desired_speed[NUM_MOTORS] = {0};

  // Control loop gains for each motor
  float kp[NUM_MOTORS] = {0};
  float ki[NUM_MOTORS] = {0};
  float kd[NUM_MOTORS] = {0};

  for (int i = 0; i < NUM_MOTORS; ++i) {

      pid[i] = pid_create(&pid_controllers[i], &motor_input[i], &motor_output[i], &desired_speed[i], kp[i], ki[i], kd[i]);

      // Set controller output limits for each motor
      pid_limits(&pid_controllers[i], 0, 200);

      // Enable auto-computation for each motor
      pid_auto(&pid_controllers[i]);
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */
  }
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

void forward(int motorSpeed, int runDuration){          //speed can be 16 bits, split into high and low bytes
	CAN_TxData[0] = (-motorSpeed) >> 8;  //high byte for speed, shifted 8 because only buffer is only 8 bits
	CAN_TxData[1] = (-motorSpeed);       //low bytes for speed
	CAN_TxData[2] = (motorSpeed) >> 8;
	CAN_TxData[3] = (motorSpeed);
	CAN_TxData[4] = (-motorSpeed) >> 8;
	CAN_TxData[5] = (-motorSpeed);
	CAN_TxData[6] = (motorSpeed) >> 8;
	CAN_TxData[7] = (motorSpeed);

	int i = 0;
	while(i < runDuration){
		HAL_CAN_AddTxMessage(&hcan1, &canTxHeader, CAN_TxData, &canTxMailbox);
		HAL_Delay(0.5);
		i++;
	}
}

void runMotors(unsigned char motorOneHigh, unsigned char motorOneLow, unsigned char motorTwoHigh, unsigned char motorTwoLow, unsigned char motorThreeHigh, unsigned char motorThreeLow, unsigned char motorFourHigh, unsigned char motorFourLow){          //speed can be 16 bits, split into high and low bytes
	CAN_TxData[0] = motorOneHigh;  //high byte for speed, shifted 8 because only buffer is only 8 bits
	CAN_TxData[1] = motorOneLow;       //low bytes for speed
	CAN_TxData[2] = motorTwoHigh;
	CAN_TxData[3] = motorTwoLow;
	CAN_TxData[4] = motorThreeHigh;
	CAN_TxData[5] = motorThreeLow;
	CAN_TxData[6] = motorFourHigh;
	CAN_TxData[7] = motorFourLow;

	int i = 0;
	while(i < 1000){
		HAL_CAN_AddTxMessage(&hcan1, &canTxHeader, CAN_TxData, &canTxMailbox);
	    HAL_Delay(0.5);
	    i++;
	}
}

void runMotorsFloats(float motorOne, float motorTwo, float motorThree, float motorFour) {
  // Convert float values to two bytes (high byte and low byte) for each motor
  uint8_t motorOneHigh = (uint8_t)((uint16_t)motorOne >> 8);
  uint8_t motorOneLow = (uint8_t)motorOne;
  uint8_t motorTwoHigh = (uint8_t)((uint16_t)motorTwo >> 8);
  uint8_t motorTwoLow = (uint8_t)motorTwo;
  uint8_t motorThreeHigh = (uint8_t)((uint16_t)motorThree >> 8);
  uint8_t motorThreeLow = (uint8_t)motorThree;
  uint8_t motorFourHigh = (uint8_t)((uint16_t)motorFour >> 8);
  uint8_t motorFourLow = (uint8_t)motorFour;

  // Call the original runMotors function with the converted values
  runMotors(motorOneHigh, motorOneLow, motorTwoHigh, motorTwoLow, motorThreeHigh, motorThreeLow, motorFourHigh, motorFourLow);
}

void dribble() {

}

void noDribble(){

}

void kick(int kickDuration){

}

void charge(int chargeDuration){

}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    if(hcan == &hcan1) {
        HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &canRxHeader, CAN_RxData);

        if(canRxHeader.StdId == 0x201) motor_idx = 0;
        if(canRxHeader.StdId == 0x202) motor_idx = 1;
        if(canRxHeader.StdId == 0x203) motor_idx = 2;
        if(canRxHeader.StdId == 0x203) motor_idx = 3;

        angle_data[motor_idx] = (uint16_t)(CAN_RxData[0]<<8 | CAN_RxData[1]);
        speed_data[motor_idx] = (int16_t)(CAN_RxData[2]<<8 | CAN_RxData[3]); // originally rpm
        torque_current_data[motor_idx] = (CAN_RxData[4]<<8 | CAN_RxData[5])*5.f/16384.f;
    }
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
