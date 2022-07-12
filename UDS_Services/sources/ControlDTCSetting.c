#include "../headers/NegativeResponseCodes.h"
#include "../headers/ControlDTCSettingService.h"
#include <string.h>

BYTE* DTCSCOR = NULL;

int RequestService(A_Data* msg, Bool suppress, BYTE sf) {
	if (!msg)
		msg = (A_Data*)malloc(sizeof(A_Data));


	unsigned int paramLength = 0;

	if (DTCSCOR != NULL)
		paramLength = strlen(DTCSCOR);
	
	if (msg->data == NULL)
		msg->data = (BYTE*)malloc((3 + paramLength) * sizeof(BYTE));

	msg->data[0] = CDTSC;

	msg->data[1] = ((suppress << 7) | sf);

	strcpy(*msg + 2, DTCSCOR);

	return 0;
}

int ReceiveResponse(A_Data msg, BYTE sf) {
	
	if (msg.data[0] != CDTSCPR)
		return 1;
	
	BYTE recv_sf = (msg.data[1] & 0x7f);

	if (recv_sf != sf)
		return 1;

	return 0;
}

int ControlDTCFunc(BYTE sf) {
	BYTE* request = NULL;
	if (RequestService(&request, False, sf))
		return 1;

	ISO_TP_Send_Request(request);

	if (!suppress) {

		BYTE* response = ISO_TP_Recv_Response();

		if (ReceiveResponse(response))
			return 1;
	}

	return 0;
}
