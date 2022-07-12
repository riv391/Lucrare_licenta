#ifndef SECURITYACCESSSERVICE_H_
#define SECURITYACCESSSERVICE_H_
#include "General_Types.h"

#define ISRSD(sf) (sf == 0x01 || (sf >= 0x03 && sf <= 0x05) || (sf >= 0x07 && sf <=0x41))
#define ISSK(sf) (sf == 0x02 || (sf >= 0x04 && sf <= 0x06) || (sf >= 0x08 && sf <= 0x42))

	static const BYTE SA = 0x27;	 	// SecurityAccess Request SID Table 40

	static const BYTE SAPR = 0x67;	// SecurityAccess Response SID Table 44

	extern BYTE *securityAcessDataRecord, *securityKey, *securitySeed;

#endif
