#include "../headers/NegativeResponseCodes.h"
#include "../headers/ReadScalingDataByIdentifierService.h"

int RequestService(A_Data* msg, WORD dataIdentifier) { 
	if (!msg)
		msg = (A_Data*)malloc(sizeof(A_Data));

	if (msg->data == NULL)
		msg->data = (BYTE*)malloc(3 * sizeof(BYTE));

	msg->data[0] = RSDBI;

	msg->data[1] = (dataIdentifier >> 0xff);

	msg->data[2] = (dataIdentifier & 0xff);

	return 0;
}

int ReceiveResponse(A_Data msg, WORD dataIdentifier) { 
	if (msg.data[0] != RSDBIPR)
		return 1;

	if ((msg.data[1] << 0xff | msg.data[2]) != dataIdentifier)
		return 1;

	BYTE scalingByte = msg.data[3];

	// Table 208 pentru restul definitiilor

	return 0;
	
}
