#ifndef DIAGNOSTICSESSIONCONTROLSERVICESERVICE_H_
#define DIAGNOSTICSESSIONCONTROLSERVICESERVICE_H_
#include "General_Types.h"

	static const BYTE DSC = 0x10;

	static const BYTE DSCPR = 0x50; // DiagnosticSessionControl Response SID Table 26

	// Table 25
	typedef enum {
		DS = 0x01,		// defaultSession
		PRGS = 0x02,		// ProgrammingSession
		EXTDS = 0x03,		// extendedDiagnosticsSession
		SSDS = 0x04		// safetySystemDiagnosticSession
	} Subfunction;

	extern WORD *P2Server_max, *P2_aux_Server_max;

	int DSC_RequestService(A_Data*, Bool, BYTE);

	int DSC_ReceiveResponse(A_Data, BYTE);

#endif
