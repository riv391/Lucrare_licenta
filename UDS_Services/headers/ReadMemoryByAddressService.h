#ifndef READMEMORYBYADDRESSSERVICE_H_
#define READMEMORYBYADDRESSSERVICE_H_
#include "General_Types.h"

	static const BYTE RMBA = 0x23; // ReadMemoryByAddress Request SID
	static const BYTE RMBAPR = 0x63; // ReadMemoryByAddress Response SID

	extern BYTE *MA, *MS, *dataRec; // Memory Address si Memory Size
#endif
