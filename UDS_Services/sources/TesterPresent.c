#include "../headers/NegativeResponseCodes.h"
#include "../headers/TesterPresentService.h"

int RequestService(A_Data* msg, Bool suppress, BYTE sf) {
	if (!msg)
		msg = (A_Data*)malloc(sizeof(A_Data));

	if (msg->data == NULL)
		msg->data = (BYTE*)malloc(3 * sizeof(BYTE));

	msg->data[2] = '\0';

	msg->data[0] = TP;

	msg->data[1] = ((suppress << 7) | sf);

	return 0;
}

int ReceiveResponse(A_Data msg, BYTE sf) {
	
	if (msg.data[0] != TPPR)
		return 1;
	
	BYTE recv_sf = (msg.data[1] & 0x7f);

	if (recv_sf != sf)
		return 1;

	return 0;
}

int TesterFunc(BYTE sf) {
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
