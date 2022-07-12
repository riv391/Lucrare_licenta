#ifndef ECURESETSERVICE_H_
#define ECURESETSERVICE_H_
#include "General_Types.h"
	
	static const BYTE ER = 0x11; 

	static const BYTE ERPR = 0x51; // ECUReset Response SID Table 35

	extern BYTE *powerDownTime;

	// Table 34
	typedef enum {
		HR = 0x01,	// HardReset
		KOFFONR = 0x02,	// keyOffOnReset
		SR = 0x03,	// softReset
		ERPSD = 0x04,	// enableRapidPowerShutDown
		DRPSD = 0x05	// disableRapidPowerShutDown
	} Subfunction;

	int ECUR_RequestService(A_Data*, Bool, BYTE);

	int ECUR_ReceiveResponse(A_Data, BYTE);

#endif
