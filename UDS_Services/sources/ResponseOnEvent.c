#include "../headers/NegativeResponseCodes.h"
#include "../headers/ResponseOnEventService.h"
#include <string.h>

// Foarte multe detalii...

BYTE *ETR = NULL, *STRTR = NULL, EWT = 0x00;

int RequestService(A_Data* msg, Bool suppress, BYTE sf) {
	if (!msg)
		msg = (A_Data*)malloc(sizeof(A_Data));

	
	unsigned int length = 0;
	
	if (ETR != NULL)
		length += strlen(ETR);
	if (STRTR != NULL)
		length += strlen(STRTR);

	if (msg->data == NULL)
		msg->data = (BYTE*)malloc((4 + length) * sizeof(BYTE));

	msg->data[2] = EWT;

	msg->data[0] = ER;

	msg->data[1] = ((suppress << 7) | sf);

	msg->data[3] = '\0';

	if (ETR != NULL)
		strcpy(*msg + 3, ETR);

	if (STRTR != NULL)
		strcpy(*msg + strlen(*msg), STRTR);

	return 0;
}

int ReceiveResponse(A_Data msg, BYTE sf) {
	
	BYTE NOIE, // numberOfActivatedEvents
	     *EVOAE; // eventTypeOfActiveEvent Table 146

	if (msg.data[0] != ERPR)
		return 1;
	
	BYTE recv_sf = (msg.data[1] & 0x7f);

	if (recv_sf != sf)
		return 1;

	NOIE = msg.data[2];

	// Table 145 si 146
	if (sf == RAE) {
		
	} else {
		EWT = msg.data[3];

	}

	return 0;
}

// Table 149 si 150
int SetupResponseOnEvent(BYTE sf) {
}

// Table 151 si 152
int StartResponseOnEvent(BYTE sf) {

}

// Table 153
int ReadDTCInformation(BYTE sf) {

}

// Table 154
int ResponseOnEvent(BYTE sf) {

}
