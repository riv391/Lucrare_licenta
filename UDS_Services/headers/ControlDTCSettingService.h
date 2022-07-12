#ifndef CONTROLDTCSETTINGSERVICE_H_
#define CONTROLDTCSETTINGSERVICE_H_
#include "General_Types.h"
	
	static const BYTE CDTSC = 0x85;	// ControlDTCSetting Request SID Table 127

	static const BYTE CDTSCPR = 0xC5;	// ControlDTCSetting Response SID Table 129

	// Table 128
	typedef enum {
		ON = 0x01,	// on
		OFF = 0x02	// off
	} Subfunction;

#endif


