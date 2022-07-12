#include <stdlib.h>
#include <string.h>
#include "../headers/NegativeResponseCodes.h"
#include "../headers/ReadDataByIdentifierService.h"

DataIdentifier Identifiers[27] = {
	{0xf1, 0x80, {0, NULL}}, // Boot software identification
	{0xf1, 0x81, {0, NULL}}, // Application software identification
	{0xf1, 0x82, {0, NULL}}, // Application data identification
	{0xf1, 0x83, {0, NULL}}, // Boot software fingerprint
	{0xf1, 0x84, {0, NULL}}, // Application software fingerprint
	{0xf1, 0x85, {0, NULL}}, // Application data fingerprint
	{0xf1, 0x86, {0, NULL}}, // Active diagnostic session
	{0xf1, 0x87, {0, NULL}}, // Manufacturer spare part number
	{0xf1, 0x88, {0, NULL}}, // Manufacturer ECU software number
	{0xf1, 0x89, {0, NULL}}, // Manufacturer ECU software version
	{0xf1, 0x8a, {0, NULL}}, // Identifier of system supplier
	{0xf1, 0x8b, {0, NULL}}, // ECU manufacturing date
	{0xf1, 0x8c, {0, NULL}}, // ECU serial number
	{0xf1, 0x8d, {0, NULL}}, // Supported functional units
	{0xf1, 0x8e, {0, NULL}}, // Manufacturer kit assembly part number
	{0xf1, 0x90, {0, NULL}}, // Vehicle identification number (VIN)
	{0xf1, 0x92, {0, NULL}}, // System supplier ECU hardware number
	{0xf1, 0x93, {0, NULL}}, // System supplier ECU hardware version number
	{0xf1, 0x94, {0, NULL}}, // System supplier ECU software number
	{0xf1, 0x95, {0, NULL}}, // System supplier ECU software version number
	{0xf1, 0x96, {0, NULL}}, // Exhaust regulation/type approval number
	{0xf1, 0x97, {0, NULL}}, // System name / engine type
	{0xf1, 0x98, {0, NULL}}, // Repair shop code / tester serial number
	{0xf1, 0x99, {0, NULL}}, // Programming date
	{0xf1, 0x9d, {0, NULL}}, // ECU installation date
	{0xf1, 0x9e, {0, NULL}}, // ODX file
	{0x00, 0x00, {0, NULL}}
};

DataIdentifier* selectedIdentifier = NULL;

int RDBI_RequestService(A_Data* msg, Bool suppress, BYTE sf) {
	if (!msg)
		msg = (A_Data*)malloc(sizeof(A_Data));

	if (selectedIdentifier == NULL)
		return 1;

	msg->Length = 3;

	if (msg->data) {
		free(msg->data);
		msg->data = NULL;
	}

	msg->data = (BYTE*)malloc(msg->Length);

	msg->data[1] = selectedIdentifier->DIDBH;

	msg->data[2] = selectedIdentifier->DIDBL;
	
	msg->data[0] = RDBI;

	return 0;
}

int RDBI_ReceiveResponse(A_Data msg, BYTE sf) {
	
	if (msg.data[0] != RDBIPR)
		return 1;
	
	if (msg.Length < 3 || 
			msg.data[1] != selectedIdentifier->DIDBH || 
			msg.data[2] != selectedIdentifier->DIDBL)
		return 1;

	if (selectedIdentifier->dataRecord.data)
		free(selectedIdentifier->dataRecord.data);

	selectedIdentifier->dataRecord.data = (BYTE*)malloc(msg.Length - 3);

	memcpy(selectedIdentifier->dataRecord.data, msg.data + 3, msg.Length - 3);

	selectedIdentifier->dataRecord.Length = msg.Length - 3;

	return 0;
}

