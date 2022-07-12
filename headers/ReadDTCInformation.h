#ifndef READDTCINFORMATION_H_
#define READDTCINFORMATION_H_
#include "header.h"

extern SimplePanel RDTCIRequests[16];
extern SimplePanel RDTCIResponses[17];
extern ConfigPanel RDTCIPanel[25];
extern ServiceParam _RDTCI;

int _DTCStatusMaskRequest();
int _DTCMaskRecordRequest();
int _DTCSnapshotRecordNumberRequest();
int _DTCStoredDataRecordNumberRequest();
int _DTCExtDataRecordNumberRequest();
int _DTCExtDataRecordNumberLabelRequest();
int _DTCSeverityMaskRecordRequest();
int _MemorySelectionRequest();
int _UserDefDTCSnapshotRecordNumberRequest();
int _FunctionalGroupIdentifierRequest();
int _DTCReadinessGroupIdentifierRequest();

int _DTCStatusMaskUpdate();
int _DTCMaskRecordUpdate();
int _DTCSnapshotRecordNumberUpdate();
int _DTCStoredDataRecordNumberUpdate();
int _DTCExtDataRecordNumberUpdate();
int _DTCExtDataRecordNumberLabelUpdate();
int _DTCSeverityMaskRecordUpdate();
int _MemorySelectionUpdate();
int _UserDefDTCSnapshotRecordNumberUpdate();
int _FunctionalGroupIdentifierUpdate();
int _DTCReadinessGroupIdentifierUpdate();

int __DTCStatusAvailabilityMaskUpdate();
int __DTCSeverityAvailabilityMaskUpdate();
int __DTCFormatIdentifierUpdate();
int __DTCCountUpdate();
int __DTCRecordUpdate();
int __DTCStatusUpdate();
int __DTCSnapshotRecordNumberUpdate();
int __DTCDataIdentifierUpdate();
int __DTCSnapshotRecordNumberOfIdentifiersUpdate();
int __DTCSnapshotRecordUpdate();
int __DTCStoredDataRecordNumberUpdate();
int __DTCStoredDataRecordNumberOfIdentifiersUpdate();
int __DTCExtDataRecordNumberUpdate();
int __DTCExtDataRecordUpdate();
int __DTCSeverityUpdate();
int __DTCFunctionalUnitUpdate();
int __DTCFaultDetectionCounterUpdate();
int __FunctionalGroupIdentifierUpdate();
int __UserDefDTCSnapshotRecordNumberUpdate();
int __DTCReadinessGroupIdentifierUpdate();
int __MemorySelectionUpdate();

void updateRDTCIResponse1();
void updateRDTCIResponse2();
void updateRDTCIResponse3();
void updateRDTCIResponse4();
void updateRDTCIResponse5();
void updateRDTCIResponse6();
void updateRDTCIResponse7();
void updateRDTCIResponse8();
void updateRDTCIResponse9();
void updateRDTCIResponse10();
void updateRDTCIResponse11();
void updateRDTCIResponse12();
void updateRDTCIResponse13();
void updateRDTCIResponse14();
void updateRDTCIResponse15();
void updateRDTCIResponse16();

int CreateRDCTIRequests();

int CreateRDTCIResponse();

int CreateRDTCIPanel();

#endif
