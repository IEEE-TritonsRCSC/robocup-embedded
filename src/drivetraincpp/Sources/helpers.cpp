#include "main.h"
#include "can.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "helpers.h"
#define DRIBBLE_SPEED 1500

DrivetrainState::DrivetrainState()
	: can(nullptr),
	  uart(nullptr),
	  canHeader1(canTxHeader1, FRAME_LENGTH, CAN_ID_STD, CAN_RTR_DATA, CANTXHEADER1_STDID, DISABLE),
	  canHeader2(canTxHeader2, FRAME_LENGTH, CAN_ID_STD, CAN_RTR_DATA, CANTXHEADER2_STDID, DISABLE),
	  canTxMailbox(0),
	  canTxData{0},
	  can2TxData{0},
	  canRxData{0},
	  motor_idx(0),
	  angle_data{0},
	  speed_data{0},
	  torque_current_data{0},
	  targetSpeeds{0},
	  uart_rx_buffer{0},
	  uart_tx_buffer{0},
	  rx_byte(0),
	  header1_flag(0),
	  header2_flag(0),
	  timeout(0),
	  dribble_flag(0),
	  dribble_speed(0),
	  motorPins{MOTOR1_PIN, MOTOR2_PIN, MOTOR3_PIN, MOTOR4_PIN, MOTOR5_PIN},
	  motor1_gains(MOTOR1_GAINS),
	  motor2_gains(MOTOR2_GAINS),
	  motor3_gains(MOTOR3_GAINS),
	  motor4_gains(MOTOR4_GAINS),
	  motor1PID(MAX_OUTPUT, INTEGRAL_LIMIT, DEADBAND, 0, motor1_gains),
	  motor2PID(MAX_OUTPUT, INTEGRAL_LIMIT, DEADBAND, 0, motor2_gains),
	  motor3PID(MAX_OUTPUT, INTEGRAL_LIMIT, DEADBAND, 0, motor3_gains),
	  motor4PID(MAX_OUTPUT, INTEGRAL_LIMIT, DEADBAND, 0, motor4_gains),
	  motor_pids{motor1PID, motor2PID, motor3PID, motor4PID}
{
}

void updateDribblerSpeedFromFlag(int dribble_flag, int16_t* dribble_speed)
{
    if (dribble_flag)
    { // Turns dribbling on/off
        *dribble_speed = DRIBBLE_SPEED;
    }
    else
    {
        *dribble_speed = 0;
    }
}

void applySafetyTimeoutToTargetSpeeds(int timeout, volatile float *targetSpeeds)
{
    if (timeout >= TIMEOUT_DELAY)
    { // Safety timeout when UART disconnects
        for (int i = 0; i < 4; ++i)
        {
            targetSpeeds[i] = 0;
        }
    }
}

void updateMotorPidLoop(PID_Data *motor_pids, volatile float *targetSpeeds, volatile float *speed_data)
{
    for (int i = 0; i < 4; ++i)
    { // PID control loop
        motor_pids[i].setTarget(targetSpeeds[i]);
        motor_pids[i].pidCalculate(speed_data[i]);
    }
}

void setupMotors(uint16_t* motorPins)
{
    // Motor setup
    for (int i = 0; i < 5; i++)
    {
        HAL_GPIO_TogglePin(MOTOR_PORT, motorPins[i]);
    }
}

void turnLEDsOff()
{
    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, LED_OFF);
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, LED_OFF);
}


void handleCanRxFifo0(DrivetrainState *state, CAN_HandleTypeDef *hcan)
{
	//HAL_GPIO_TogglePin(LED_GREEN_PORT,LED_GREEN_PIN);
	if (hcan == state->can) {
		HAL_CAN_GetRxMessage(state->can, CAN_RX_FIFO0, &state->canRxHeader, state->canRxData);

		if (state->canRxHeader.StdId == 0x201)
			state->motor_idx = 0;
		if (state->canRxHeader.StdId == 0x202)
			state->motor_idx = 1;
		if (state->canRxHeader.StdId == 0x203)
			state->motor_idx = 2;
		if (state->canRxHeader.StdId == 0x204)
			state->motor_idx = 3;

		// angle of the motor (0 - 8191 corresponding with 0 - 360 deg)
		state->angle_data[state->motor_idx] = (uint16_t)(state->canRxData[0] << 8 | state->canRxData[1]);

		// speed of the motor in rpm
		state->speed_data[state->motor_idx] = ((int16_t)(state->canRxData[2] << 8 | state->canRxData[3]));

		// torque current of the motor
		state->torque_current_data[state->motor_idx] = (state->canRxData[4] << 8 | state->canRxData[5]);
	}
}

void handleUartRxComplete(DrivetrainState *state, UART_HandleTypeDef *huart)
{
	(void)huart;
	/*
	 * Called when uart_rx_buffer is full
	 */

	if (!state->header1_flag) {
		if (state->rx_byte == HEADER_BYTE_1) {
			// first header byte received

			state->header1_flag = 1;
			HAL_UART_Receive_IT(state->uart, const_cast<uint8_t *>(&state->rx_byte), 1);
			return;
		}
	} else {

		if (state->header2_flag) {
			// full message received

			state->timeout = 0;

			state->targetSpeeds[0] = (int16_t)((state->uart_rx_buffer[0] << 8) | state->uart_rx_buffer[1]);
			state->targetSpeeds[1] = (int16_t)((state->uart_rx_buffer[2] << 8) | state->uart_rx_buffer[3]);
			state->targetSpeeds[2] = (int16_t)((state->uart_rx_buffer[4] << 8) | state->uart_rx_buffer[5]);
			state->targetSpeeds[3] = (int16_t)((state->uart_rx_buffer[6] << 8) | state->uart_rx_buffer[7]);

			/*
			 * Previously |targetSpeeds[i]| <= 500
			 */

			if (state->uart_rx_buffer[FRAME_LENGTH] == DRIBBLE_ON)
			{
				state->dribble_flag = 1;
			}
			else
			{
				state->dribble_flag = 0;
			}

			for (int i = 0; i < UART_RX_BUFFER_SIZE; ++i) {
				state->uart_rx_buffer[i] = 0;
			}

			HAL_GPIO_TogglePin(LED_GREEN_PORT, LED_GREEN_PIN);

			state->header1_flag = 0;
			state->header2_flag = 0;
			HAL_UART_Receive_IT(state->uart, const_cast<uint8_t *>(&state->rx_byte), 1);
			return;

		} else if (state->rx_byte == HEADER_BYTE_2) {
			// second header byte received

			state->header2_flag = 1;
			HAL_UART_Receive_IT(state->uart, state->uart_rx_buffer, UART_RX_BUFFER_SIZE);
			return;
		} else {
			// first header byte received but not followed by second header byte

			state->header1_flag = 0;
			HAL_UART_Receive_IT(state->uart, const_cast<uint8_t *>(&state->rx_byte), 1);
			return;
		}
	}
}

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

void setMotorSpeeds(DrivetrainState *state, int16_t speedCommands[5]) {
	uint8_t highBytes[5] = {0};
	uint8_t lowBytes[5] = {0};

	for (int i=0;i<NUM_MOTORS;i++) {
		highBytes[i] = speedCommands[i] >> 8;
		lowBytes[i] = speedCommands[i];
	}
	for (int i=0;i<8;i++) {
		if (i % 2 == 0) {
			state->canTxData[i] = highBytes[i / 2];
		} else {
			state->canTxData[i] = lowBytes[i / 2];
		}
	}
	state->can2TxData[0] = highBytes[4];
	state->can2TxData[1] = lowBytes[4];

	HAL_CAN_AddTxMessage(state->can, state->canHeader1.getTxHeaderPointer(), state->canTxData, &state->canTxMailbox);
	HAL_CAN_AddTxMessage(state->can, state->canHeader2.getTxHeaderPointer(), state->can2TxData, &state->canTxMailbox);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

