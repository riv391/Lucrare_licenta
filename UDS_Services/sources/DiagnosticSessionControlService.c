#include <stdlib.h>
#include "../headers/NegativeResponseCodes.h"
#include "../headers/DiagnosticSessionControlService.h"
	
WORD *P2Server_max = NULL, *P2_aux_Server_max = NULL;

int DSC_RequestService(A_Data* msg, Bool suppress, BYTE sf) {
	if (!msg)
		msg = (A_Data*)malloc(sizeof(A_Data));

	if (msg->data == NULL)
		msg->data = (BYTE*)malloc(2 * sizeof(BYTE));

	msg->data[0] = DSC;

	msg->data[1] = ((suppress << 7) | sf);

	msg->Length = 2;

	return 0;
}

int DSC_ReceiveResponse(A_Data msg, BYTE sf) {
	
	if (msg.data[0] != DSCPR)
		return 1;
	
	BYTE recv_sf = (msg.data[1] & 0x7f);

	if (recv_sf != sf)
		return 1;

	if (!P2Server_max)
		P2Server_max = (WORD*)malloc(sizeof(WORD));

	*P2Server_max = ((msg.data[2] << 8) | msg.data[3]);

	if (!P2_aux_Server_max)
		P2_aux_Server_max = (WORD*)malloc(sizeof(WORD));

	*P2_aux_Server_max = ((msg.data[4] << 8) | msg.data[5]); // Pentru NRC 0x78 Vezi Table29

	return 0;
}

//int SetSession(BYTE sf) {
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
