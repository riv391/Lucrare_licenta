#include "../headers/NegativeResponseCodes.h"
#include "../headers/CommunicationControlService.h"

BYTE CTP = 0x00;
WORD *NIN = NULL;

int RequestService(A_Data* msg, Bool suppress, BYTE sf) {
	if (!msg)
		msg = (A_Data*)malloc(sizeof(A_Data));


	unsigned char len = (4 + ((sf == ERXDTXWEAI || sf == ERXTXWEAI) && NIN != NULL) * 2) * sizeof(BYTE);

	if (msg->data == NULL)
		msg->data = (BYTE*)malloc(len);

	msg->data[len - 1] = '\0';

	msg->data[0] = CC;

	msg->data[1] = ((suppress << 7) | sf);

	msg->data[2] = CTP;

	// Este conditionat de CTP vezi Table 53
	if ((sf == ERXDTXWEAI || sf == ERXTXWEAI) && NIN != NULL) {
		msg->data[3] = (*NIN >> 0xff);
		msg->data[4] = (BYTE)(*NIN);
	}

	return 0;
}

int ReceiveResponse(A_Data msg, BYTE sf) {
	
	if (msg.data[0] != CCPR)
		return 1;
	
	BYTE recv_sf = (msg.data[1] & 0x7f);

	if (recv_sf != sf)
		return 1;

	return 0;
}


