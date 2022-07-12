#include "../headers/NegativeResponseCodes.h"
#include "../headers/SecurityAccessService.h"

BYTE *securityAcessDataRecord = NULL, *securityKey = NULL, *securitySeed = NULL;

int RequestService(A_Data* msg, Bool suppress, BYTE sf) {
	if (!msg)
		msg = (A_Data*)malloc(sizeof(A_Data));

	
	if (ISRSD(sf) && securityAcessDataRecord != NULL) {
		msg->data = (char*)malloc(strlen(securityAcessDataRecord) + 3);
		strcpy(*msg + 2, securityAcessDataRecord);
	}

	if (ISSK(sf) && securityKey != NULL) {
		msg->data = (char*)malloc(strlen(securityKey) + 3);
		strcpy(*msg + 2, securityKey);
	}

	if (msg->data == NULL)
		msg->data = (BYTE*)malloc((3 + strlen(paramLength)) * sizeof(BYTE));

	msg->data[0] = SA;

	msg->data[1] = ((suppress << 7) | sf);

	return 0;
}

int ReceiveResponse(A_Data msg, BYTE sf) {
	
	if (msg.data[0] != SAPR)
		return 1;
	
	BYTE recv_sf = (msg.data[1] & 0x7f);

	if (recv_sf != sf)
		return 1;

	unsigned int seedLength = msg.Length - 2;

	if (seedLength > 0) {
		if (securitySeed != NULL) {
			free(securitySeed);
			securitySeed = NULL;
		}
		securitySeed = (char*)malloc(seedLength);
		strcpy(securitySeed, msg + 2);
	}
	
	return 0;
}
