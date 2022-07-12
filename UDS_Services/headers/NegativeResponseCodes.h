#ifndef NEGATIVERESPONSECODESSERVICE_H_
#define NEGATIVERESPONSECODESSERVICE_H_
	
	typedef enum {
		SFNS = 0x12,	// SubFunctionNotSupported
		IMLOIF = 0x13,	// incorrectMessageLengthOrInvalidFormat
		CNC = 0x22,	// conditionNotCorrect
		RSE = 0x24,	// requestSequenceError
		ROOR = 0x31,	// requestOutOfRange
		SAD = 0x33,	// securityAccessDenied
		IK = 0x35,	// invalidKey
		ENOA = 0x36,	// exceededNumberOfAttempts
		RTDNE = 0x37,	// requiredTimeDelayNotExpired
		RTL = 0x14,	// responseTooLong
		AR = 0x34,	// authenticationRequired
		GPF = 0x72,	// generalProgrammingFailure
		UDNA = 0x70,	// uploadDownloadNotAccepted
		TDS = 0x71,	// transferDataSuspended
		WBSC = 0x73,	// wrongBlockSequenceCounter
		VTH = 0x92,	// voltageTooHigh
		VTL = 0x93,	// voltageTooLow
		SDVF = 0x3a	// secureDataVerificationFailed
	} Negative_Response;

#endif
