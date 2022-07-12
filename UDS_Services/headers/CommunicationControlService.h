#ifndef COMMUNICATIONCONTROLSERVICE_H_
#define COMMUNICATIONCONTROLSERVICE_H_
#include "General_Types.h"
	
	static const BYTE CC = 0x28;		// CommunicationControl Request SID Table 53

	static const BYTE CCPR = 0x68; 	// CommunicationControl Response SID Table 56

	extern BYTE CTP;
	extern WORD *NIN;

	// Table 54
	typedef enum {
		ERXTX,		// enableRxAndTx
		ERXDTX,		// enableRxAndDisableTx
		DRXETX,		// disableRxAndEnableTx
		DRXTX,		// disableRxAndTx
		ERXDTXWEAI,	// enableRxAndDisableTxWithEnhancedAddressInformation
		ERXTXWEAI	// enableRxAndTxWithEnhancedAddressInformation
	} Subfunction;

#endif

