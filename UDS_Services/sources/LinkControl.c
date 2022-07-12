#include "../headers/NegativeResponseCodes.h"
#include "../headers/LinkControlService.h"
#include <string.h>

BYTE *linkRecord = NULL, LCMI = 0x00;

int RequestService(A_Data* msg, Bool suppress, BYTE sf) {
	if (!msg)
		msg = (A_Data*)malloc(sizeof(A_Data));

	unsigned int length = 3 + (sf == VMTWFP);

	if (sf == VMTWSP && linkRecord != NULL)
		length += strlen(linkRecord);
	
	if (msg->data == NULL)
		msg->data = (BYTE*)malloc(length * sizeof(BYTE));

	msg->data[length - 1] = '\0';

	msg->data[0] = LC;

	msg->data[1] = ((suppress << 7) | sf);

	if (sf == VMTWFP)
		msg->data[2] = LCMI;
	
	if (sf == VMTWSP)
		strcpy(*msg + 2, linkRecord);
	
	return 0;
}

int ReceiveResponse(A_Data msg, BYTE sf) {
	
	if (msg.data[0] != LCPR)
		return 1;
	
	BYTE recv_sf = (msg.data[1] & 0x7f);

	if (recv_sf != sf)
		return 1;

	return 0;
}

// Table 178
void LinkControlFunc(BYTE sf) {
	BYTE* request = NULL;

	// Ar trebui setate variabilele globale, nu stiu unde exact

	if (RequestService(&request, False, sf))
		return 1;

	ISO_TP_Send_Request(request);
	BYTE* response = ISO_TP_Recv_Response();

	if (ReceiveResponse(response))
		return 1;

	free(request);

	request = NULL;

	if (RequestService(&request, True, TM))
		return 1;

	return 0;
}
