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
#include "pid.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define UART_RX_BUFFER_SIZE 9  //set to the size we want to limit receive messages to
#define UART_TX_BUFFER_SIZE 33  //set to the size we want to limit send messages to
#define RUN_MOTOR_HEADER 0x01
#define DRIBBLER_HEADER 0x02
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//CAN variables
CAN_TxHeaderTypeDef canTxHeader;
CAN_RxHeaderTypeDef canRxHeader;
uint32_t canTxMailbox;
uint8_t CAN_TxData[8];
uint8_t CAN_RxData[8];
volatile uint8_t motor_idx;
volatile uint16_t angle_data[4];
volatile int16_t speed_data[4];
volatile float torque_current_data[4];
int16_t motorCurrents[4];
volatile int16_t targetSpeeds[4];
PID_TypeDef motor_pid[4];

//UART setup
uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE]; //buffer that stores in an array of characters user inputs, aka a string
uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];
uint8_t headers[] = {RUN_MOTOR_HEADER, DRIBBLER_HEADER};
int ms_to_listen = 4000;  //set the number of ms we keep the uart line in receive mode for

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

/*
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (uart_rx_buffer[0] == headers[0]){
		for (int i = 0; i<8; i++){
			motorCurrents[i] = uart_rx_buffer[i+1];
		}
		uint8_t feedback[] = {0x01, 0x01, 0x01, 0x01, (speed_data[0] >> 8), (speed_data[0] & 0xff), (speed_data[1] >> 8), (speed_data[1] & 0xff), (speed_data[2] >> 8), (speed_data[2] & 0xff),(speed_data[3] >> 8), (speed_data[3] & 0xff)};
		HAL_UART_Transmit(&huart2, feedback, sizeof(feedback), 1000);
	}
	HAL_UART_Receive_IT(&huart2, uart_rx_buffer, sizeof(uart_rx_buffer));
}*/


int main(void)
{
  /* USER CODE BEGIN 1 */
	//uint8_t uart_rx_buffer[uart_rx_buffer_size]; //buffer that stores in an array of characters user inputs, aka a string
	//uint8_t uart_tx_buffer[uart_tx_buffer_size];
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
  canTxHeader.DLC = 8;
  canTxHeader.IDE = CAN_ID_STD;
  canTxHeader.RTR = CAN_RTR_DATA;
  canTxHeader.StdId = 0x200;
  canTxHeader.TransmitGlobalTime = DISABLE;

  //PID Setup
  for (int i = 0; i < 4; i++) {
        speed_data[i] = 100;
  }
  pid_init(&motor_pid[0],10000,5000,20,0,0.05,0,0);
  //pid_init(&motor_pid[1],10000,5000,20,0,0,0,0);
  //pid_init(&motor_pid[2],10000,5000,20,0,0,0,0);
  //pid_init(&motor_pid[3],10000,5000,20,0,0,0,0);
  /*
  for(int i=0; i<4; i++)
  {
	  pid_init(&motor_pid[i],10000,1000,50,0,1.1,0.3,0); //sets max output to 9999, integral windup to 1000, PID deadzone to 20 (setpoints below this won't work), Kp=1.5, Ki=0.3, Kd=0
  }*/


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //runMotors(0x03, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  //uint8_t feedback[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  //HAL_UART_Receive_IT(&huart2, uart_rx_buffer, UART_RX_BUFFER_SIZE);   //Uncomment to start the UART interrupt tests
  for (int i = 0; i < 4; i++) {
	  /*
	  if (i%2 == 0){
		  targetSpeeds[i] = -100;
	  }
	  else{*/
	  targetSpeeds[i] = 100;
	  motor_pid[i].output = 1000;
	  //}
  }

  //HAL_UART_Receive_IT(&huart2, uart_rx_buffer, sizeof(uart_rx_buffer));
  while (1)
  {
	  /*
	  for(int i=0; i<4; i++){
		  motor_pid[i].target = targetSpeeds[i];
	      pid_calculate(&motor_pid[i],speed_data[i]);
	  }*/
	  //setMotorSpeeds(-(motor_pid[0].output),(motor_pid[1].output),-(motor_pid[2].output),(motor_pid[3].output));
	  //setMotorSpeeds((motor_pid[0].output),0,0,0);
	  //setMotorSpeeds((motor_pid[0].output),0,0,0);
	  //HAL_Delay(10);
	  /*
	  for (int i = 0; i<4; i++){
		  motorCurrents[i] = (int)motor_pid[i].output;
		  if (motorCurrents[i] > 10000){
			  motorCurrents[i] = 10000;
		  }
		  else if (motorCurrents[i] < -10000){
			  motorCurrents[i] = -10000;
		  }
	  }
	  setMotorSpeeds(motorCurrents[0],motorCurrents[1],motorCurrents[2],motorCurrents[3]);  //some tests for just running motors with values*/
	  //HAL_Delay(1000);
	  //setMotorSpeeds(1000,1000,1000,1000);
	  //HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	  //HAL_Delay(1);
	  //HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);  //LED and delay looping


	  uint8_t feedback[] = {0x01, 0x01, 0x01, 0x01, (speed_data[0] >> 8), (speed_data[0] & 0xff), (speed_data[1] >> 8), (speed_data[1] & 0xff), (speed_data[2] >> 8), (speed_data[2] & 0xff),(speed_data[3] >> 8), (speed_data[3] & 0xff)};
	  HAL_UART_Transmit(&huart2, feedback, sizeof(feedback), 1000);
	  /*
	   //UART Receive code, uncomment this block and comment out other parts to test UART
	  HAL_UART_Receive(&huart2, uart_rx_buffer, UART_RX_BUFFER_SIZE, 1000);
	  if (uart_rx_buffer[0] == headers[0]){
		  uint8_t feedback[] = {0x01, 0x01, 0x01, 0x01, (speed_data[0] >> 8), (speed_data[0] & 0xff), (speed_data[1] >> 8), (speed_data[1] & 0xff), (speed_data[2] >> 8), (speed_data[2] & 0xff),(speed_data[3] >> 8), (speed_data[3] & 0xff)};
		  //runMotors(uart_rx_buffer[1], uart_rx_buffer[2], uart_rx_buffer[3], uart_rx_buffer[4], uart_rx_buffer[5], uart_rx_buffer[6], uart_rx_buffer[7], uart_rx_buffer[8]);
		  HAL_UART_Transmit(&huart2, feedback, sizeof(feedback), 1000);
	  }*/
	  HAL_Delay(1);

	  //runMotors(motorCurrents[0], motorCurrents[1], motorCurrents[2], motorCurrents[3], motorCurrents[4], motorCurrents[5], motorCurrents[6], motorCurrents[7]);  //for testing UART interrupts
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */
  }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	//HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
    if(hcan == &hcan1) {
        HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &canRxHeader, CAN_RxData);

        if(canRxHeader.StdId == 0x201) motor_idx = 0;
        if(canRxHeader.StdId == 0x202) motor_idx = 1;
        if(canRxHeader.StdId == 0x203) motor_idx = 2;
        if(canRxHeader.StdId == 0x204) motor_idx = 3;

        angle_data[motor_idx] = (uint16_t)(CAN_RxData[0]<<8 | CAN_RxData[1]);
        speed_data[motor_idx] = (int16_t)(CAN_RxData[2]<<8 | CAN_RxData[3]); // originally rpm
        torque_current_data[motor_idx] = (CAN_RxData[4]<<8 | CAN_RxData[5]);
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
void forward(int motorSpeed){          //speed can be 16 bits, split into high and low bytes
	CAN_TxData[0] = (-motorSpeed) >> 8;  //high byte for speed, shifted 8 because only buffer is only 8 bits
	CAN_TxData[1] = (-motorSpeed);       //low bytes for speed
	CAN_TxData[2] = (motorSpeed) >> 8;
	CAN_TxData[3] = (motorSpeed);
	CAN_TxData[4] = (-motorSpeed) >> 8;
	CAN_TxData[5] = (-motorSpeed);
	CAN_TxData[6] = (motorSpeed) >> 8;
	CAN_TxData[7] = (motorSpeed);

	HAL_CAN_AddTxMessage(&hcan1, &canTxHeader, CAN_TxData, &canTxMailbox);
}

void setMotorSpeeds(int16_t ms1, int16_t ms2, int16_t ms3, int16_t ms4){
	uint8_t h1 = ms1 >> 8;
	uint8_t l1 = ms1;
	uint8_t h2 = ms2 >> 8;
	uint8_t l2 = ms2;
	uint8_t h3 = ms3 >> 8;
	uint8_t l3 = ms3;
	uint8_t h4 = ms4 >> 8;
	uint8_t l4 = ms4;
	runMotors(h1,l1,h2,l2,h3,l3,h4,l4);
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
	HAL_CAN_AddTxMessage(&hcan1, &canTxHeader, CAN_TxData, &canTxMailbox);

	/*
	int i = 0;
	while(i < 100000){
		HAL_CAN_AddTxMessage(&hcan1, &canTxHeader, CAN_TxData, &canTxMailbox);
	    HAL_Delay(0.5);
	    i++;
	}*/
}

void dribble() {

}

void noDribble(){

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
