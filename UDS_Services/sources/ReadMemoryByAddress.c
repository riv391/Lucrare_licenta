#include "../headers/NegativeResponseCodes.h"
#include "../headers/ReadMemoryByAddressService.h"

BYTE *MA = NULL, *MS = NULL, dataRec = NULL;

// ALFID = Table 195
int RequestService(A_Data* msg, BYTE ALFID) {
	if (!msg)
		msg = (A_Data*)malloc(sizeof(A_Data));


	if (MS == NULL || BYTE_HIGH(ALFID) != LENGTH(MS))
		return 1;

	if (MA == NULL || BYTE_LOW(ALFID) != LENGTH(MA))
		return 1;

	unsigned int length = 2 + BYTE_HIGH(ALFID) + BYTE_LOW(ALFID);

	if (msg->data == NULL)
		msg->data = (BYTE*)malloc(length * sizeof(BYTE));

	msg->data[0] = RMBA;

	msg->data[1] = ALFID;

	memcpy(*msg + 2, MA, BYTE_LOW(ALFID));

	memcpy(*msg + 2 + BYTE_LOW(ALFID), MS, BYTE_HIGH(ALFID));

	return 0;
}

int ReceiveResponse(A_Data msg) { 
	if (msg.data[0] != RMBAPR)
		return 1;
	
	if (dataRec != NULL) {
		free(dataRec);
		dataRec = NULL;
	}

	if (MS == NULL)
		return 1;

	// Vezi exemplul de la tabelul 200
	dataRec = (BYTE*)malloc( * sizeof(BYTE)); // Tine cont de lungimea din MS

	memcpy(dataRec, msg + 1, );

	return 0;
}
