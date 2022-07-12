/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include "../headers/CAN_Types.h"
#include "../headers/ISO_TP.h"
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
CAN_HandleTypeDef hcan;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */
uint8_t ledMask = 1;
uint32_t txMailBox;
CAN_TxHeaderTypeDef txHeader;
CAN_RxHeaderTypeDef rxHeader;
uint8_t txData[64], rxData[64];
uint8_t busyFlag = 0;
UART_HandleTypeDef* currentInterface = NULL;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
static void TP_Init(void);
static int IsConfigured(void);
static void ConfigureFormat(Format*);
static void ConfigureInfo(N_AI*);
static void SendUart(size_t);
static void ResetTimer();
static void SetIDE(unsigned char);
static void SetId(unsigned int);
static void SetRTR(unsigned char);
static int IsExtended();
static int IsRemote();
static void myDelay(BYTE, DelayType);
static void SetLeds(unsigned char);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void SetLeds(unsigned char mask) {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, (mask & 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (mask & 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, (mask & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	if (currentState == Idle) {
		if (GPIO_Pin == GPIO_PIN_5 || GPIO_Pin == GPIO_PIN_6) {
			ledMask >>= (GPIO_Pin == GPIO_PIN_5);
			ledMask <<= (GPIO_Pin == GPIO_PIN_6);
			ledMask = (ledMask > 4) ? 1 : ledMask;
			ledMask = (ledMask < 1) ? 4 : ledMask;
			SetLeds(ledMask);
		}
	}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (currentState == Idle) {

		currentState = Config;
		SetLeds(4);

		currentInterface = huart;
		ManageUARTRequest(Size);
	}

	if (huart == &huart1) {
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uartRx, 1024);
		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
	}
	if (huart == &huart2) {
		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uartRx, 1024);
		__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
	}
}

int RecvMessage(TP_Handler* TPHandler, BYTE** msg){

	__HAL_TIM_SET_PRESCALER(&htim1, 64000);
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	htim1.Instance->EGR |= 1;
	htim1.Instance->EGR &= (uint32_t)(~1);

	while(!busyFlag) {
		if (__HAL_TIM_GET_COUNTER(&htim1) > 1000) {
					busyFlag = 0;
					currentState = Idle;
					SetLeds(0);
					return 1;
		}
	}

	__HAL_TIM_SET_PRESCALER(&htim1, 64000);
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	htim1.Instance->EGR |= 1;
	htim1.Instance->EGR &= (uint32_t)(~1);

	TPHandler->ConnectionFormat.DLC = rxHeader.DLC;
	TPHandler->ConnectionFormat.RTR = rxHeader.RTR;
	TPHandler->ConnectionFormat.IDE = rxHeader.IDE;

	if (msg) {
		if (!(*msg))
			*msg = (BYTE*)malloc(CAN_DL[TPHandler->ConnectionFormat.DLC] * sizeof(BYTE));
		MEMCPY(*msg, rxData, TPHandler->ConnectionFormat.DLC * sizeof(BYTE));
	}

	busyFlag = 0;

	return 0;
}

int SendMessage(BYTE* msg, Format* format){

	if (msg) {

		txHeader.DLC = format->DLC;
		txHeader.RTR = format->RTR;
		txHeader.IDE = format->IDE;

		MEMCPY(txData, msg, CAN_DL[txHeader.DLC] * sizeof(BYTE));

		if (__HAL_TIM_GET_COUNTER(&htim1) > 1000)
			return 1;

		while (HAL_CAN_AddTxMessage(&hcan, &txHeader, txData, &txMailBox));
	}

	return 0;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {

	while (busyFlag);

	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData))
		Error_Handler();

	if (currentState == Idle) {
		currentState = Receiving;
		setLeds(1);
	}

	busyFlag = 1;
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
  MX_CAN_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  TP_Init();
  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_TIM_Base_Start(&htim1);

  currentState = Idle;

  MessageBuffer.data = NULL;
  MessageBuffer.Length = 0;

  txHeader.ExtId = (uint32_t)(-1);
  txHeader.IDE = (uint32_t)(-1);
  txHeader.RTR = (uint32_t)(-1);
  txHeader.StdId = (uint32_t)(-1);
  txHeader.TransmitGlobalTime = DISABLE;

  SetLeds(ledMask);

  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uartRx, 99);
  __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uartRx, 99);
  __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  StartDiagnosis();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 16;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  CAN_FilterTypeDef confilterconfig;

  confilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  confilterconfig.FilterBank = 10;
  confilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  confilterconfig.FilterIdHigh = 0;
  confilterconfig.FilterIdLow = 0x0000;
  confilterconfig.FilterMaskIdHigh = 0;
  confilterconfig.FilterMaskIdLow = 0x0000;
  confilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  confilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
  confilterconfig.SlaveStartFilterBank = 13;

  HAL_CAN_ConfigFilter(&hcan, &confilterconfig);

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_DISABLE;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  if (HAL_TIM_SlaveConfigSynchro(&htim1, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA5 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */

void
TP_Init() {
	DelayFor = myDelay;
	RecvFromDataLink = RecvMessage;
	SendToDataLink = SendMessage;
	validConfiguration = IsConfigured;
	isExtended = IsExtended;
	isRemote = IsRemote;
	configureFormat = ConfigureFormat;
	configureInfo = ConfigureInfo;
	sendUart = SendUart;
	resetTimer = ResetTimer;
	setIDE = SetIDE;
	setIdentifier = SetId;
	setRTR = SetRTR;
	setLeds = SetLeds;
}

int
IsConfigured() {
	return (((txHeader.IDE == CAN_ID_STD && txHeader.StdId != (uint32_t)(-1)) ||
			(txHeader.IDE == CAN_ID_EXT && txHeader.ExtId != (uint32_t)(-1))) &&
			txHeader.RTR != (uint32_t)(-1));
}

void
ConfigureFormat(Format* format) {
	switch (currentState) {
		case Receiving: {
			format->DLC = rxHeader.DLC;
			format->IDE = (rxHeader.IDE == CAN_ID_EXT);
			format->RTR = (rxHeader.RTR == CAN_RTR_REMOTE);
			break;
		}
		case Sending: {
			format->DLC = 8;
			format->IDE = (txHeader.IDE == CAN_ID_EXT);
			format->RTR = (txHeader.RTR == CAN_RTR_REMOTE);
			break;
		}
		default:
			break;
	}
}

void
ConfigureInfo(N_AI* AddressInfo) {
	switch (currentState) {
		case Receiving: {
			if (rxHeader.IDE == CAN_ID_STD) {
				AddressInfo->SA = rxHeader.StdId;
				AddressInfo->TA = rxHeader.StdId;
			}
			if (rxHeader.IDE == CAN_ID_EXT) {
				AddressInfo->SA = rxHeader.ExtId;
				AddressInfo->TA = rxHeader.ExtId;
			}
			break;
		}
		case Sending: {
			if (txHeader.IDE == CAN_ID_STD)
				  AddressInfo->SA = rxHeader.StdId;

			if (txHeader.IDE == CAN_ID_EXT)
				  AddressInfo->SA = rxHeader.ExtId;
			break;
		}
		default:
			break;
	}
}

void
SendUart(size_t timeout) {
	if (currentInterface) {
		if (currentInterface == &huart1) {
				MEMCPY(uartRx, MessageBuffer.data, MessageBuffer.Length);
				uartRx[MessageBuffer.Length] = '\r';
				uartRx[MessageBuffer.Length + 1] = '\n';
				MessageBuffer.Length += 2;
				HAL_UART_Transmit(currentInterface, uartRx, MessageBuffer.Length, timeout);
		}
		if (currentInterface == &huart2)
				HAL_UART_Transmit(currentInterface, MessageBuffer.data, MessageBuffer.Length, timeout);
	}
}
void
ResetTimer() {
	__HAL_TIM_SET_PRESCALER(&htim1, 64000);
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	htim1.Instance->EGR |= 1;
	htim1.Instance->EGR &= (uint32_t)(~1);
}

void
SetIDE(unsigned char parameter) {
	if (parameter)
		txHeader.IDE = CAN_ID_EXT;
	else
		txHeader.IDE = CAN_ID_STD;
}

void
SetId(unsigned int id) {
	if (txHeader.IDE == CAN_ID_STD)
		txHeader.StdId = id;
	if (txHeader.IDE == CAN_ID_EXT)
		txHeader.ExtId = id;
}

void
SetRTR(unsigned char parameter) {
	if (parameter)
		txHeader.RTR = CAN_RTR_REMOTE;
	else
		txHeader.RTR = CAN_RTR_DATA;
}

int
IsExtended() {
	return (txHeader.IDE == CAN_ID_EXT);
}

int
IsRemote() {
	return (txHeader.RTR == CAN_RTR_REMOTE);
}

void
myDelay(BYTE per, DelayType prescaler) {
	__HAL_TIM_SET_PRESCALER(&htim1, (prescaler != micro) * 64000 + (prescaler == micro) * 64);
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	htim1.Instance->EGR |= 1;
	htim1.Instance->EGR &= (uint32_t)(~1);
	while (__HAL_TIM_GET_COUNTER(&htim1) < per);
}
/* USER CODE END 4 */

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

