#ifndef LINKCONTROLSERVICE_H_
#define LINKCONTROLSERVICE_H_
#include "General_Types.h"

	static const LC = 0x87;

	static const LCPR = 0xc7;

	extern BYTE *linkRecord, LCMI;

	typedef enum {
		VMTWFP = 0x01,	// verifyModeTransitionWithFixedParameter
		VMTWSP = 0x02,	// verifyModeTransitionWithSpecificParameter
		TM = 0x03	// transitionMode
	} Subfunction;

#endif
