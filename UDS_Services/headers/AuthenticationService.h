#ifndef AUTHENTICATIONSERVICE_H_
#define AUTHENTICATIONSERVICE_H_
#include "General_Types.h"
	
	static const BYTE ARS = 0x29;		// Authentication Request SID Table 65

	static const BYTE ARDPR = 0x69;	// ECUReset Response SID Table 77

	// Table 74
	typedef enum {
		DA,	// deAuthenticate
		VCU,	// verifyCertificateUnidirectional
		VCB,	// veryfyCertificateBidirectional
		POWN,	// proofOfOwnership
		TC,	// transmitCertificate
		RCFA,	// requestChallengeForAuthentication
		VPOWNU,	// verifyProofOfOwnershipUnidirectional
		VPOWNB,	// verifyProofOfOwnershipBidirectional
		AC	// authenticationConfiguration
	} Subfunction;

#endif

