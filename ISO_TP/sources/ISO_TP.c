#include <stdlib.h>
#include "../headers/CAN_Types.h"
#include "../headers/PCI.h"
#include "../headers/Receiver.h"
#include "../headers/Sender.h"
#include "../headers/ISO_TP.h"

TPState currentState = Idle;
N_Data MessageBuffer;
BYTE uartRx[1024];
int (*validConfiguration)() = 0;
int (*isExtended)() = 0;
int (*isRemote)() = 0;
void (*configureFormat)(Format*) = 0;
void (*configureInfo)(N_AI*) = 0;
void (*sendUart)(size_t timeout) = 0;
void (*resetTimer)() = 0;
void (*setIDE)(unsigned char) = 0;
void (*setIdentifier)(unsigned int) = 0;
void (*setRTR)(unsigned char) = 0;
void (*setLeds)(unsigned char) = 0;

const BYTE config[] = "Config: ";
const BYTE error[] = "Error";

void StartDiagnosis() {
	while (1)
	{
		switch (currentState) {
		  case Receiving: {
			  Format format;
			  if (configureFormat)
				  configureFormat(&format);
			  //format.RTR = rxHeader.RTR;
			  //format.IDE = rxHeader.IDE;
			  N_AI AddressInfo;
			  if (configureInfo)
				  configureInfo(&AddressInfo);
			  StartReceiving(format, AddressInfo, &MessageBuffer);
			  //HAL_UART_Transmit_IT(&huart1, uartTx, MessageBuffer.Length);
			  if (sendUart)
				  sendUart(1000);
			  //HAL_UART_Transmit(&huart1, MessageBuffer.data, MessageBuffer.Length, 1000);
			  free(MessageBuffer.data);
			  MessageBuffer.data = NULL;
			  currentState = Idle;
			  if (setLeds)
				  setLeds(0);
			  break;
		  }
		  case Sending: {

			  Format format;
			  format.DLC = 8;
			  //format.RTR = txHeader.RTR;
			  //format.IDE = txHeader.IDE;
			  N_AI AddressInfo;
			  if (configureInfo)
				  configureInfo(&AddressInfo);
			  //AddressInfo.TA = txHeader.StdId;

			  //__HAL_TIM_SET_PRESCALER(&htim1, 55000);
			  //__HAL_TIM_SET_COUNTER(&htim1, 0);
			  //htim1.Instance->EGR |= 1;
			  //htim1.Instance->EGR &= (uint32_t)(~1);
			  if (resetTimer)
				  resetTimer();

			  StartSender(format, AddressInfo, &MessageBuffer);
			  currentState = Idle;
			  if (setLeds)
				  setLeds(0);
			  break;
		  }
		  default:
			  break;
		}
	}
}

void ManageUARTRequest(size_t Size) {
	
	int i;

	size_t size = Size - (uartRx[Size - 1] == '\r');

	for (i = 8; size > 12 && i > 0; --i)
		if (config[i - 1] != uartRx[i - 1])
			break;

	if (MessageBuffer.data) {
		free(MessageBuffer.data);
		MessageBuffer.data = NULL;
	}

	if (i) {

		if (MessageBuffer.data) {
			free(MessageBuffer.data);
			MessageBuffer.data = NULL;
		}

		if (validConfiguration && validConfiguration()) {

			currentState = Sending;
			if (setLeds)
				setLeds(2);

			MessageBuffer.Length = size;
			MessageBuffer.data = (BYTE*)malloc(size * sizeof(BYTE));
			MEMCPY(MessageBuffer.data, uartRx, size);

			//memcpy(MessageBuffer.data, uartRx, Size * sizeof(BYTE));
		} else {
			MessageBuffer.Length = 13;
			MessageBuffer.data = (BYTE*)malloc(MessageBuffer.Length);
			MEMCPY(MessageBuffer.data, error, 5);
			MessageBuffer.data[5] = ':';
			MessageBuffer.data[6] = ' ';
			MEMCPY(MessageBuffer.data + 7, config, 6);
			if (sendUart)
				sendUart(1000);
			currentState = Idle;
			if (setLeds)
				setLeds(0);
			//HAL_UART_Transmit(&huart1, MessageBuffer.data, MessageBuffer.Length, 1000);
			if (MessageBuffer.data) {
				free(MessageBuffer.data);
				MessageBuffer.data = NULL;
			}
		}
	} else {
		currentState = Config;
		if (setLeds)
			setLeds(4);

		unsigned int tempID = 0x0;

		for (i = 8; uartRx[i] >= '0' && uartRx[i] <= '9' && i < size; tempID = (tempID * 10) + (uartRx[i] - '0'), ++i);

		if (size == i + 4 && uartRx[i + 2] == ';')
		{

			if (setIDE)
				setIDE(uartRx[i + 1] == '1');

			if (setIdentifier)
				setIdentifier(tempID);

			if (setRTR)
				setRTR(uartRx[i + 3] == '1');

			char tempValue [11];
			for (i = 0; tempID; tempValue[i] = (tempID % 10) + '0', tempID /= 10, ++i);
			MessageBuffer.Length = i + 12;
			MessageBuffer.data = (BYTE*)malloc(MessageBuffer.Length);
			for (; i > 0; MessageBuffer.data[MessageBuffer.Length - 4 - i] = tempValue[i - 1], --i);
			for (i = 0; i < 8; MessageBuffer.data[i] = config[i], ++i);
			MessageBuffer.data[MessageBuffer.Length - 4] = ';';
			MessageBuffer.data[MessageBuffer.Length - 3] = '0' + (isExtended && isExtended());
			MessageBuffer.data[MessageBuffer.Length - 2] = ';';
			MessageBuffer.data[MessageBuffer.Length - 1] = '0' + (isRemote && isRemote());

		} else {
			MessageBuffer.Length = 5;
			MessageBuffer.data = (BYTE*)malloc(MessageBuffer.Length);
			MEMCPY(MessageBuffer.data, error, 5);
		}

		if (validConfiguration && !validConfiguration()) {
			if (MessageBuffer.data) {
				free(MessageBuffer.data);
				MessageBuffer.data = NULL;
			}
			MessageBuffer.Length = 5;
			MessageBuffer.data = (BYTE*)malloc(MessageBuffer.Length);
			MEMCPY(MessageBuffer.data, error, 5);
		}

		//HAL_UART_Transmit_IT(&huart1, uartTx, length);
		if (sendUart)
			sendUart(1000);
		currentState = Idle;
		if (setLeds)
			setLeds(0);
		if (MessageBuffer.data) {
			free(MessageBuffer.data);
			MessageBuffer.data = NULL;
		}
	}
}
