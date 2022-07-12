#ifndef READDTCINFORMATIONSERVICE_H_
#define READDTCINFORMATIONSERVICE_H_
#include "General_Types.h"
	
	#define SETBYTE(pointer, value) {\
		if (!pointer)\
			pointer = (BYTE*)malloc(sizeof(BYTE));\
		*(pointer) = value;\
	}

	static const BYTE RDTCI = 0x19;
	static const BYTE RDTCIPR = 0x59;

	typedef enum {
		RNODTCBSM = 0x01,	// reportNumberOfDTCByStatusMask
		RDTCBSM = 0x02,		// reportDTCByStatusMask
		RDTCSSI = 0x03,		// reportDTCSnapshotIdentification
		RDTCSSBDTC = 0x04,	// reportDTCSnapshotRecordByDTCNumber
		RDTCSDBRN = 0x05,	// reportDTCStoredDataByRecordNumber
		RDTCEDRBDN = 0x06,	// reportDTCExtDataRecordByDTCNumber
		RNODTCBSMR = 0x07,	// reportNumberOfDTCBySeverityMaskRecord
		RDTCBSMR = 0x08,	// reportDTCBySeverityMaskRecord
		RSIODTC = 0x09,		// reportSeverityInformationOfDTC
		RSUPDTC = 0x0a,		// reportSupportedDTC
		RFTFDTC = 0x0b,		// reportFirstTestFailedDTC
		RFCDTC = 0x0c,		// reportFirstConfirmedDTC
		RMRTFDTC = 0x0d,	// reportMostRecentTestFailedDTC
		RMRCDTC = 0x0e,		// reportMostRecentConfirmedDTC
		RDTCFDC = 0x14,		// reportDTCFaultDetectionCounter
		RDTCWPS = 0x15,		// reportDTCWithPermanentStatus
		RDTCEDRBR = 0x16,	// reportDTCExtDataRecordByRecordNumber
		RUDMDTCBSM = 0x17,	// reportUserDefMemoryDTCByStatusMask
		RUDMDTCSSBDTC = 0x18,	// reportUserDefMemoryDTCSnapshotRecordByDTCNumber
		RUDMDTCEDRBDN = 0x19,	// reportUserDefMemoryDTCExtDataRecordByDTCNumber
		RDTCEDI = 0x1a,		// reportDTCExtendedDataRecordIdentification
		RWWHOBDDTCBMR = 0x42,	// reportWWHOBDDTCByMaskRecord
		RWWHOBDDTCWPS = 0x55,	// reportWWHOBDDTCWithPermanentStatus
		RDTCBRGI = 0x56		// reportDTCInformationByDTCReadinessGroupIdentifier
	} Subfunction;

	typedef enum {
		SAE_J2012_F0 = 0x00, // SAE_J2012-DA_DTCFormat_00
		ISO_14229 = 0x01, // ISO_14229-1_DTCFormat
		SAE_J1939 = 0x02, // SAE_J1939-73_DTCFormat
		ISO_11992 = 0x03, // ISO_11992-4DTCFormat
		SAE_J2012_F4 = 0x04  // SAE_J2012-DA_DTCFormat_04
	} DTCFormatIdentifierType;

	typedef struct {
		BYTE DTCHB, DTCMB, DTCLB;
	} DTCRecord;

	typedef struct {
		DTCRecord record;
		BYTE* SODTC;
	} DTCAndStatusRecord;

	typedef struct {
		DTCRecord record;
		BYTE DTCFDC;
	} DTCFaultDetectionCounterRecord;

	typedef struct {
		BYTE DTCS, DTCFU;
		DTCAndStatusRecord StatusAndRecord;
	} DTCAndSeverityRecord;

	typedef struct DTCSnapshotItem {
		DataIdentifier DTCSnapshot;
		BYTE SnapshotNumber;
		BYTE SnapshotNumberOfIdentifiers;
		struct DTCSnapshotItem* next;
	} DTCSnapshotItem;

	typedef struct DTCStoredDataItem {
		DataIdentifier DTCStoredData;
		BYTE StoredDataNumber;
		BYTE StoredDataNumberOfIdentifiers;
		BYTE UserDefRecordNumber;
		struct DTCStoredDataItem* next;
	} DTCStoredDataItem;

	typedef struct DTCExtDataItem {
		BYTE DTCExtDataNumber;
		A_Data DTCExtDataRecord;
		struct DTCExtDataItem* next;
	} DTCExtDataItem;

	typedef struct DTCItem {
		DTCAndStatusRecord DTC;
		BYTE *severity, *functionalUnit, *faultCounter;
		DTCSnapshotItem* userSnapshots;
		DTCSnapshotItem* snapshots;
		DTCStoredDataItem* storedData;
		DTCExtDataItem* extendedData;
		struct DTCItem* next;
	} DTCItem;

	extern BYTE *DTCSM,
		*DTCSSRN, 
		*DTCSDRN, 
		*DTCEDRN, 
		*FGID,
		*DTCSAM,
		*DTCSVM, 
		*DTCSVAM, 
		*DTCRGI,
		*UDDTCSSRN,
		*MEMYS;
		
	extern DTCRecord* DTCMaskRecord;
	extern DTCFormatIdentifierType* DTCFID;
	extern WORD* DTCCount;
	extern DTCItem* DTCs;

	int RDTCI_RequestService(A_Data*, Bool, BYTE); 
	int RDTCI_ReceiveResponse(A_Data, BYTE); 
	DTCExtDataItem* AddExtRecord(DTCExtDataItem**, BYTE);
	DTCStoredDataItem* AddStoredDataRecord(DTCStoredDataItem**, BYTE);
	DTCSnapshotItem* AddSnapshotRecord(DTCSnapshotItem**, BYTE);
	DTCItem* AddRecord(DTCItem**, DTCRecord);
#endif
