#ifndef RESPONSEONEVENTSERVICE_H_
#define RESPONSEONEVENTSERVICE_H_
#include "General_Types.h"

	static const ROE = 0x86;

	static const ROEPR = 0xc6;

	extern BYTE *ETR, *STRTR, EWT; // Table 138

	typedef enum {
		// Table 139
		DNSE = 0x00,	// doNotStoreEvent
		SE = 0x01,	// storeEvent

		// Table 140
		STPROE = 0x00,		// control stopResponseOnEvent
		ONDTCS = 0x01,		// setup onDRCStatusChange
		OCODID = 0x03,		// setup onChangeOfDataIdentifier
		RAE = 0x04,		// control reportActivatedEvents
		STRTROE = 0x05,		// control startResponseEvent
		CLRROE = 0x06,		// control clearResponseOnEvent
		OCOV = 0x07,		// setup onComparisonOfValues
		RMRDOSC = 0x08,		// control reportMostRecentDtcOnStatusChange
		RDRIODSC = 0x09		// control reportDRCRecordInformationOnDtcStatusChange
	}Subfunction;

#endif
