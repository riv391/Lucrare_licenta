#ifndef READDATABYIDENTIFIERSERVICE_H_
#define READDATABYIDENTIFIERSERVICE_H_
#include "General_Types.h"

	static const BYTE RDBI = 0x22; // ReadDataByIdentifier Request ID
	static const BYTE RDBIPR = 0x62; // ReadDataByIdentifier Response ID

	//typedef struct DataIdentifierItem {
	//	DataIdentifier identifier;
	//	struct DataIdentifierItem* next;
	//} DataIdentifierItem;

	extern DataIdentifier Identifiers[27];

	extern DataIdentifier* selectedIdentifier;

	//extern size_t numberOfIdentifiers;

	int RDBI_RequestService(A_Data*, Bool, BYTE);

	int RDBI_ReceiveResponse(A_Data, BYTE);
#endif
