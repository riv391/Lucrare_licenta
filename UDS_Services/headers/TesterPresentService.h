#ifndef TESTERPRESENTSERVICE_H_
#define TESTERPRESENTSERVICE_H_
#include "General_Types.h"
	
	static const BYTE TP = 0x3e;		// TesterPresent Request SID Table 119

	static const BYTE TPPR = 0x7e;	// TesterPresent Response SID Table 121

	// Table 120
	typedef enum {
		ZSUBF = 0x00	// zeroSubFunction
	} Subfunction;

#endif

