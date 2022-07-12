#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/NegativeResponseCodes.h"
#include "../headers/ReadDTCInformationService.h"

// Variabile necesare pentru crearea cererilor
BYTE *DTCSM = NULL,
	*DTCSSRN, 
	*DTCSDRN = NULL, 
	*DTCEDRN = NULL, 
	*FGID = NULL,
	*DTCSAM = NULL,
	*DTCSVM = NULL, 
	*DTCSVAM = NULL, 
	*DTCRGI = NULL,
       	*UDDTCSSRN = NULL,
	*MEMYS = NULL;

DTCRecord* DTCMaskRecord = NULL;
DTCFormatIdentifierType* DTCFID = NULL;
WORD* DTCCount = NULL;
DTCItem* DTCs = NULL;

int RDTCI_RequestService(A_Data* msg, Bool suppress, BYTE sf) { 
	
	if (!msg)
		msg = (A_Data*)malloc(sizeof(A_Data));

	msg->Length = 2;

	if (sf == RNODTCBSM || sf == RDTCBSM || sf == RDTCSDBRN || sf == RDTCEDI) {
		if (!msg->data)
			msg->data = (BYTE*)malloc(3 * sizeof(BYTE));

		if (DTCSM && (sf == RNODTCBSM || sf == RDTCBSM))
			msg->data[2] = *DTCSM;

		if (DTCSDRN && (sf == RDTCSDBRN))
			msg->data[2] = *DTCSDRN;

		if (DTCEDRN && (sf == RDTCEDI))
			msg->data[2] = *DTCEDRN;

		if (FGID && (sf == RWWHOBDDTCWPS))
			msg->data[2] = *FGID;

		//msg->data[2] = 
		//	*DTCSM * (sf == RNODTCBSM || sf == RDTCBSM) +
		//      *DTCSDRN * (sf = RDTCSDBRN) + 
		//	*DTCEDRN * (sf == RDTCEDI) +
		//	*FGID * (sf == RWWHOBDDTCWPS); 
		msg->Length = 3;
	}

	if (sf == RDTCSSBDTC || sf == RDTCEDRBDN || sf == RDTCEDRBR || sf == RSIODTC) {

		msg->Length = 6 - (sf == RSIODTC || sf == RWWHOBDDTCBMR) + (sf == RUDMDTCSSBDTC || sf == RUDMDTCEDRBDN);

		if (!msg->data)
			msg->data = (BYTE*)malloc(msg->Length * sizeof(BYTE));

		if (sf != RWWHOBDDTCBMR && DTCMaskRecord) {
			msg->data[2] = DTCMaskRecord->DTCHB; 
			msg->data[3] = DTCMaskRecord->DTCMB;
			msg->data[4] = DTCMaskRecord->DTCLB;
		}

		if (sf == RWWHOBDDTCBMR && FGID && DTCSM && DTCSVM) {
			msg->data[2] = *FGID;
			msg->data[3] = *DTCSM;
			msg->data[4] = *DTCSVM;
		}

		if (sf != RSIODTC){
			if (DTCSSRN && (sf == RDTCSSI || sf == RDTCSSBDTC))
				msg->data[5] = *DTCSSRN;
			if (DTCEDRN && (sf == RDTCEDRBDN || sf == RDTCEDRBR || sf == RUDMDTCEDRBDN))
				msg->data[5] = *DTCEDRN;
			if (UDDTCSSRN && (sf == RUDMDTCEDRBDN))
				msg->data[5] = *UDDTCSSRN;
			//msg->data[5] = 
			//	*DTCSSRN * (sf == RDTCSSI || sf == RDTCSSBDTC) +
			//       	*DTCEDRN * (sf == RDTCEDRBDN || sf == RDTCEDRBR || sf == RUDMDTCEDRBDN) +
			//       	*UDDTCSSRN * (sf == RUDMDTCSSBDTC); 
		}

		if (MEMYS && (sf == RUDMDTCSSBDTC || sf == RUDMDTCEDRBDN))
			msg->data[6] = *MEMYS;
	}

	if (sf == RNODTCBSMR || sf == RDTCBSMR || sf == RUDMDTCBSM) {
		if (!msg->data)
			msg->data = (BYTE*)malloc(4 * sizeof(BYTE));

		if (DTCSVM && DTCSM && (sf == RNODTCBSMR || sf == RDTCBSMR)) {
			msg->data[2] = *DTCSVM;
			msg->data[3] = *DTCSM;
		}

		if (DTCSM && MEMYS && sf == RUDMDTCBSM) {
			msg->data[2] = *DTCSM;
			msg->data[3] = *MEMYS;
		}

		if (FGID && DTCRGI && sf == RDTCBSMR) {
			msg->data[2] = *FGID;
			msg->data[3] = *DTCRGI;
		}

		msg->Length = 4;

		//msg->data[2] = 
		//	DTCSeverityMask * (sf == RNODTCBSMR || sf == RDTCBSMR) +
		//       	DTCStatusMask * (sf == RUDMDTCBSM) + 
		//	FunctionalGroupIdentifier * (sf == RDTCBRGI);

		//msg->data[3] = 
		//	DTCStatusMask * (sf == RNODTCBSMR || sf == RDTCBSMR) +
		//       	MemorySelection * (sf == RUDMDTCBSM) + 
		//	DTCReadinessGroupIdentifier * (sf == RDTCBRGI);
	}

	if (sf == RDTCSSI || sf == RSUPDTC || sf == RFTFDTC || sf == RFCDTC || sf == RMRTFDTC || sf == RMRCDTC || sf == RDTCFDC || sf == RDTCWPS)
		if (!msg->data)
			msg->data = (BYTE*)malloc(2 * sizeof(BYTE));

	msg->data[0] = RDTCI;

	msg->data[1] = sf;

	return 0;
}

int RDTCI_ReceiveResponse(A_Data msg, BYTE sf) { 

	if (msg.data[0] != RDTCIPR)
		return 1;

	BYTE recv_sf = (msg.data[1] & 0x7f);

	if (recv_sf != sf)
		return 1;

	// Table 319
	if (sf == RNODTCBSM || sf == RNODTCBSMR) {
		//if (msg.Length < 6)
		//	return 2;
		SETBYTE(DTCSAM, msg.data[2]);
		if (!DTCFID)
			DTCFID = (DTCFormatIdentifierType*)malloc(sizeof(DTCFormatIdentifierType));
		*DTCFID = msg.data[3];
		if (!DTCCount)
			DTCCount = (WORD*)malloc(sizeof(WORD));
		*DTCCount = ((WORD)msg.data[4] << 8) + msg.data[5];
	}

	// Table 320
	if (sf == RDTCBSM || sf == RSUPDTC || sf == RFTFDTC || sf == RFCDTC || sf == RMRTFDTC || sf == RMRCDTC || sf == RDTCWPS) {
		SETBYTE(DTCSAM, msg.data[2]);
		DTCRecord Record;
		//if ((msg.Length - 3) & 3)
		//	return 2;
		for (unsigned int i = 3; i < msg.Length; i += 4) {
			Record.DTCHB = msg.data[i];
			Record.DTCMB = msg.data[i + 1];
			Record.DTCLB = msg.data[i + 2];
			DTCItem* newDTC = AddRecord(&DTCs, Record);
			SETBYTE(newDTC->DTC.SODTC, msg.data[i + 3]);
		}
	}

	// Table 321
	if (sf == RDTCSSI) {
		DTCRecord Record;
		//if ((msg.Length - 3) & 3)
		//	return 2;
		for (unsigned int i = 3; i < msg.Length; i += 4) {
			Record.DTCHB = msg.data[i];
			Record.DTCMB = msg.data[i + 1];
			Record.DTCLB = msg.data[i + 2];
			DTCItem* newDTC = AddRecord(&DTCs, Record);
			AddSnapshotRecord(&newDTC->snapshots, msg.data[i + 3]);
		}
	}

	// Table 322
	if (sf == RDTCSSBDTC) {
		DTCRecord Record;
		Record.DTCHB = msg.data[2];
		Record.DTCMB = msg.data[3];
		Record.DTCLB = msg.data[4];
		DTCItem* newDTC = AddRecord(&DTCs, Record);
		SETBYTE(newDTC->DTC.SODTC, msg.data[5]);
		DTCSnapshotItem* newSnapshot = AddSnapshotRecord(&newDTC->snapshots, msg.data[6]);
		newSnapshot->SnapshotNumberOfIdentifiers = msg.data[7];
		newSnapshot->DTCSnapshot.DIDBH = msg.data[8];
		newSnapshot->DTCSnapshot.DIDBL = msg.data[9];
		newSnapshot->DTCSnapshot.dataRecord.data = (BYTE*)malloc(msg.Length - 10);
		newSnapshot->DTCSnapshot.dataRecord.Length = msg.Length - 10;
		memcpy(newSnapshot->DTCSnapshot.dataRecord.data, msg.data + 10, msg.Length - 10);
	}

	// Table 323
	if (sf == RDTCSDBRN) {
		DTCRecord Record;
		if (msg.Length > 3) {
			Record.DTCHB = msg.data[3];
			Record.DTCMB = msg.data[4];
			Record.DTCLB = msg.data[5];
			DTCItem* newDTC = AddRecord(&DTCs, Record);
			DTCStoredDataItem* newStoredData = AddStoredDataRecord(&newDTC->storedData, msg.data[2]);
			SETBYTE(newDTC->DTC.SODTC, msg.data[6]);
			newStoredData->StoredDataNumberOfIdentifiers = msg.data[7];
			newStoredData->DTCStoredData.DIDBH = msg.data[8];
			newStoredData->DTCStoredData.DIDBL = msg.data[9];
			newStoredData->DTCStoredData.dataRecord.data = (BYTE*)malloc(msg.Length - 10);
			newStoredData->DTCStoredData.dataRecord.Length = msg.Length - 10;
			memcpy(newStoredData->DTCStoredData.dataRecord.data, msg.data + 10, msg.Length - 10);
		}
	}

	// Table 324
	if (sf == RDTCEDRBDN) {
		DTCRecord Record;
		Record.DTCHB = msg.data[2];
		Record.DTCMB = msg.data[3];
		Record.DTCLB = msg.data[4];
		DTCItem* newDTC = AddRecord(&DTCs, Record);
		SETBYTE(newDTC->DTC.SODTC, msg.data[5]);
		DTCExtDataItem* newExtData = AddExtRecord(&newDTC->extendedData, msg.data[6]);
		newExtData->DTCExtDataRecord.data = (BYTE*)malloc(msg.Length - 7);
		newExtData->DTCExtDataRecord.Length = msg.Length - 7;
		memcpy(newExtData->DTCExtDataRecord.data, msg.data + 7, msg.Length - 7);
	}

	// Table 325
	if (sf == RDTCBSMR || sf == RSIODTC) {
		SETBYTE(DTCSAM, msg.data[2]);
		DTCRecord Record;
		for (unsigned int i = 3; i < msg.Length; i += 6) {
			Record.DTCHB = msg.data[i + 2];
			Record.DTCMB = msg.data[i + 3];
			Record.DTCLB = msg.data[i + 4];
			DTCItem* newDTC = AddRecord(&DTCs, Record);
			SETBYTE(newDTC->DTC.SODTC, msg.data[i + 5]);
			SETBYTE(newDTC->severity, msg.data[i]);
			SETBYTE(newDTC->functionalUnit, msg.data[i + 1]);
		}
	}

	// Table 326
	if (sf == RDTCFDC) {
		DTCRecord Record;
		for (unsigned int i = 2; i < msg.Length; i += 4) {
			Record.DTCHB = msg.data[i];
			Record.DTCMB = msg.data[i + 1];
			Record.DTCLB = msg.data[i + 2];
			DTCItem* newDTC = AddRecord(&DTCs, Record);
			SETBYTE(newDTC->faultCounter, msg.data[i + 3]);
		}
	}

	// Table 327
	if (sf == RDTCEDRBR) {
		DTCRecord Record;
		Record.DTCHB = msg.data[3];
		Record.DTCMB = msg.data[4];
		Record.DTCLB = msg.data[5];
		DTCItem* newDTC = AddRecord(&DTCs, Record);
		SETBYTE(newDTC->DTC.SODTC, msg.data[6]);
		DTCExtDataItem* newExtData = AddExtRecord(&newDTC->extendedData, msg.data[2]);
		newExtData->DTCExtDataRecord.data = (BYTE*)malloc(msg.Length - 7);
		newExtData->DTCExtDataRecord.Length = msg.Length - 7;
		memcpy(newExtData->DTCExtDataRecord.data, msg.data + 7, msg.Length - 7);
	}

	// Table 328
	if (sf == RUDMDTCBSM) {
		SETBYTE(MEMYS, msg.data[2]);
		SETBYTE(DTCSAM, msg.data[3]);
		DTCRecord Record;
		for (unsigned int i = 4; i < msg.Length; i += 4) {
			Record.DTCHB = msg.data[i];
			Record.DTCMB = msg.data[i + 1];
			Record.DTCLB = msg.data[i + 2];
			DTCItem* newDTC = AddRecord(&DTCs, Record);
			SETBYTE(newDTC->DTC.SODTC, msg.data[i + 3]);
		}
	}

	// Table 329
	if (sf == RUDMDTCSSBDTC) {
		SETBYTE(MEMYS, msg.data[2]);
		DTCRecord Record;
		Record.DTCHB = msg.data[3];
		Record.DTCMB = msg.data[4];
		Record.DTCLB = msg.data[5];
		DTCItem* newDTC = AddRecord(&DTCs, Record);
		SETBYTE(newDTC->DTC.SODTC, msg.data[6]);
		DTCSnapshotItem* newUserSnapshot = AddSnapshotRecord(&newDTC->userSnapshots, msg.data[7]);
		newUserSnapshot->SnapshotNumberOfIdentifiers = msg.data[8];
		newUserSnapshot->DTCSnapshot.DIDBH = msg.data[9];
	       	newUserSnapshot->DTCSnapshot.DIDBL = msg.data[10];
		newUserSnapshot->DTCSnapshot.dataRecord.data = (BYTE*)malloc(msg.Length - 11);
		newUserSnapshot->DTCSnapshot.dataRecord.Length = msg.Length - 11;
		memcpy(newUserSnapshot->DTCSnapshot.dataRecord.data, msg.data + 11, msg.Length - 11);	
	}

	// Table 330
	if (sf == RUDMDTCEDRBDN) {
		SETBYTE(MEMYS, msg.data[2]);
		DTCRecord Record;
		Record.DTCHB = msg.data[3];
		Record.DTCMB = msg.data[4];
		Record.DTCLB = msg.data[5];
		DTCItem* newDTC = AddRecord(&DTCs, Record);
		SETBYTE(newDTC->DTC.SODTC, msg.data[6]);
		DTCExtDataItem* newExtData = AddExtRecord(&newDTC->extendedData, msg.data[7]);
		newExtData->DTCExtDataRecord.data = (BYTE*)malloc(msg.Length - 8);
		newExtData->DTCExtDataRecord.Length = msg.Length - 8;
		memcpy(newExtData->DTCExtDataRecord.data, msg.data + 8, msg.Length - 8);
	}

	// Table 331
	if (sf == RDTCEDI) {
		SETBYTE(DTCSAM, msg.data[2]);
		DTCRecord Record;
		for (unsigned int i = 4; i < msg.Length; i += 4) {
			Record.DTCHB = msg.data[i];
			Record.DTCMB = msg.data[i + 1];
			Record.DTCLB = msg.data[i + 2];
			DTCItem* newDTC = AddRecord(&DTCs, Record);
			SETBYTE(newDTC->DTC.SODTC, msg.data[i + 3]);
			AddExtRecord(&newDTC->extendedData, msg.data[3]);
		}
	}

	// Table 332
	if (sf == RWWHOBDDTCBMR) {
		SETBYTE(FGID, msg.data[2]);
		SETBYTE(DTCSAM, msg.data[3]);
		SETBYTE(DTCSVAM, msg.data[4]);
		if (!DTCFID)
			DTCFID = (DTCFormatIdentifierType*)malloc(sizeof(DTCFormatIdentifierType));
		*DTCFID = msg.data[5];
		DTCRecord Record;
		for (unsigned int i = 6; i < msg.Length; i += 5) {
			Record.DTCHB = msg.data[i + 1];
			Record.DTCMB = msg.data[i + 2];
			Record.DTCLB = msg.data[i + 3];
			DTCItem* newDTC = AddRecord(&DTCs, Record);
			SETBYTE(newDTC->severity, msg.data[i]);
			SETBYTE(newDTC->DTC.SODTC, msg.data[i + 4]);
		}
	}

	// Table 333
	if (sf == RWWHOBDDTCWPS) {
		SETBYTE(FGID, msg.data[2]);
		SETBYTE(DTCSAM, msg.data[3]);
		if (!DTCFID)
			DTCFID = (DTCFormatIdentifierType*)malloc(sizeof(DTCFormatIdentifierType));
		*DTCFID = msg.data[5];
		DTCRecord Record;
		for (unsigned int i = 5; i < msg.Length; i += 4) {
			Record.DTCHB = msg.data[i];
			Record.DTCMB = msg.data[i + 1];
			Record.DTCLB = msg.data[i + 2];
			DTCItem* newDTC = AddRecord(&DTCs, Record);
			SETBYTE(newDTC->DTC.SODTC, msg.data[i + 3]);
		}
	}
	
	// Table 334
	if (sf == RDTCBRGI) {
		SETBYTE(FGID, msg.data[2]);
		SETBYTE(DTCSAM, msg.data[3]);
		if (!DTCFID)
			DTCFID = (DTCFormatIdentifierType*)malloc(sizeof(DTCFormatIdentifierType));
		*DTCFID = msg.data[5];
		SETBYTE(DTCRGI, msg.data[5]);
		DTCRecord Record;
		for (unsigned int i = 6; i < msg.Length; i += 4) {
			Record.DTCHB = msg.data[i];
			Record.DTCMB = msg.data[i + 1];
			Record.DTCLB = msg.data[i + 2];
			DTCItem* newDTC = AddRecord(&DTCs, Record);
			SETBYTE(newDTC->DTC.SODTC, msg.data[i + 3]);
		}
	}

	return 0;
}

DTCExtDataItem* AddExtRecord(DTCExtDataItem** list, BYTE value) {
	DTCExtDataItem* item = *list;

	for (; item && item->DTCExtDataNumber != value; item = item->next);

	if (!item) {
		item = (DTCExtDataItem*)malloc(sizeof(DTCExtDataItem));
		item->DTCExtDataNumber = value;
		item->next = *list;
		*list = item;
	}

	return item;
}

DTCStoredDataItem* AddStoredDataRecord(DTCStoredDataItem** list, BYTE value) {
	DTCStoredDataItem* item = *list;

	for (; item && item->StoredDataNumber != value; item = item->next);

	if (!item) {
		item = (DTCStoredDataItem*)malloc(sizeof(DTCStoredDataItem));
		item->StoredDataNumber = value;
		item->next = *list;
		*list = item;
	}

	return item;
}

DTCSnapshotItem* AddSnapshotRecord(DTCSnapshotItem** list, BYTE value) {
	DTCSnapshotItem* item = *list;

	for (; item && item->SnapshotNumber != value; item = item->next);

	if (!item) {
		item = (DTCSnapshotItem*)malloc(sizeof(DTCSnapshotItem));
		item->SnapshotNumber = value;
		item->next = *list;
		*list = item;
	}

	return item;
}

DTCItem* AddRecord(DTCItem** list, DTCRecord record) {
	DTCItem* item = *list;

	for (; item && !(item->DTC.record.DTCHB == record.DTCHB && 
			 item->DTC.record.DTCMB == record.DTCMB && 
			 item->DTC.record.DTCLB == record.DTCLB); item = item->next);

	if (!item) {
		item = (DTCItem*)malloc(sizeof(DTCItem));
		item->DTC.record = record;
		item->DTC.SODTC = NULL;
		item->severity = NULL;
		item->functionalUnit = NULL;
		item->faultCounter = NULL;
		item->userSnapshots = NULL;
		item->snapshots = NULL;
		item->extendedData = NULL;
		item->storedData = NULL;
		item->next = *list;
		*list = item;
	}

	return item;
}

