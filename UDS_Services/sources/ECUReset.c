#include <stdlib.h>
#include "../headers/NegativeResponseCodes.h"
#include "../headers/ECUResetService.h"

BYTE *powerDownTime = NULL;

int ECUR_RequestService(A_Data* msg, Bool suppress, BYTE sf) {
	if (!msg)
		msg = (A_Data*)malloc(sizeof(A_Data));

	if (!msg->data)
		msg->data = (BYTE*)malloc(2 * sizeof(BYTE));

	msg->data[0] = ER;

	msg->data[1] = ((suppress << 7) | sf);

	msg->Length = 2;

	return 0;
}

int ECUR_ReceiveResponse(A_Data msg, BYTE sf) {
	
	if (msg.data[0] != ERPR)
		return 1;
	
	BYTE recv_sf = (msg.data[1] & 0x7f);

	if (recv_sf != sf)
		return 1;

	if (msg.Length > 2 && sf == ERPSD && msg.data[2] < 0xff) {
		if (!powerDownTime)
			powerDownTime = (BYTE*)malloc(sizeof(BYTE));

		*powerDownTime = msg.data[2];
	}
	
	return 0;
}

//int ResetECU(BYTE sf) {
//
//	BYTE* request = NULL;
//	if (RequestService(&request, False, sf))
//		return 1;
//
//	ISO_TP_Send_Request(request);
//	BYTE* response = ISO_TP_Recv_Response();
//
//	if (ReceiveResponse(response))
//		return 1;
//
//	return 0;
//}
