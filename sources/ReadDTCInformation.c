#include <stdlib.h>
#include "../headers/ReadDTCInformation.h"
#include "../UDS_Services/headers/ReadDTCInformationService.h"

#define NOTVALID(label) gtk_label_set_text(GTK_LABEL(label), "Not a valid option!");
#define VALID(label) gtk_label_set_text(GTK_LABEL(label), NULL);

#define NEWLIST(element, name, selection) {\
	element.elements = (SimpleElement*)malloc(3 * sizeof(SimpleElement));\
	element.elements[0].widget = gtk_label_new(name);\
	element.elements[0].xPos = 0, element.elements[0].yPos = 0;\
	element.elements[1].widget = gtk_list_box_new();\
	element.elements[1].xPos = 0, element.elements[1].yPos = 1;\
	element.elements[2].widget = NULL;\
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(element.elements[1].widget), selection);\
	if (selection == GTK_SELECTION_SINGLE)\
		g_signal_connect(G_OBJECT(element.elements[1].widget), "row-selected", G_CALLBACK(UpdateListEvent), NULL);\
	CREATEELEMENT(element)\
}

#define NEWLABEL(element, name) {\
	element.elements = (SimpleElement*)malloc(3 * sizeof(SimpleElement));\
	element.elements[0].widget = gtk_label_new(name);\
	element.elements[0].xPos = 0, element.elements[0].yPos = 0;\
	element.elements[1].widget = gtk_label_new(NULL);\
	element.elements[1].xPos = 1, element.elements[1].yPos = 0;\
	element.elements[2].widget = NULL;\
	CREATEELEMENT(element);\
}

#define NEWENTRY(element, name) {\
	element.elements = (SimpleElement*)malloc(4 * sizeof(SimpleElement));\
	element.elements[0].widget = gtk_label_new(name);\
	element.elements[0].xPos = 0, element.elements[0].yPos = 0;\
	element.elements[1].widget = gtk_entry_new();\
	element.elements[1].xPos = 1, element.elements[1].yPos = 0;\
	element.elements[2].widget = gtk_label_new(NULL);\
	element.elements[2].xPos = 2, element.elements[2].yPos = 0;\
	element.elements[3].widget = NULL;\
	gtk_entry_set_max_length(GTK_ENTRY(element.elements[1].widget), 2);\
	gtk_entry_set_placeholder_text(GTK_ENTRY(element.elements[1].widget), "XX");\
	CREATEELEMENT(element);\
}

#define NEWCOMBO(element, name, event) {\
	element.elements = (SimpleElement*)malloc(4 * sizeof(SimpleElement));\
	element.elements[0].widget = gtk_label_new(name);\
	element.elements[0].xPos = 0, element.elements[0].yPos = 0;\
	element.elements[1].widget = gtk_combo_box_text_new();\
	element.elements[1].xPos = 1, element.elements[1].yPos = 0;\
	element.elements[2].widget = gtk_label_new(NULL);\
	element.elements[2].xPos = 2, element.elements[2].yPos = 0;\
	element.elements[3].widget = NULL;\
	if (event == True)\
		g_signal_connect(G_OBJECT(element.elements[1].widget), "changed", G_CALLBACK(UpdateComboEvent), NULL);\
	CREATEELEMENT(element);\
}

#define SETENTRYTEXT(element, value) {\
	GtkEntryBuffer* buffer = gtk_entry_get_buffer(GTK_ENTRY(element.elements[1].widget));\
	if (!value)\
		gtk_entry_buffer_delete_text(buffer, 0, -1);\
	else {\
		sprintf(buff, "%x", *value);\
		gtk_entry_buffer_set_text(buffer, buff, strlen(buff));\
	}\
}

#define SETLABELTEXT(element, value) {\
	if (!value)\
		sprintf(buff, "Not set");\
	else\
		sprintf(buff, "%x", *value);\
	gtk_label_set_text(GTK_LABEL(element.elements[1].widget), buff);\
}

#define EMPTYLIST(element) {\
	GtkWidget* tempWidget = gtk_widget_get_first_child(GTK_WIDGET(element.elements[1].widget));\
	while (tempWidget) {\
		gtk_list_box_remove(GTK_LIST_BOX(element.elements[1].widget), GTK_WIDGET(tempWidget));\
		tempWidget = gtk_widget_get_first_child(GTK_WIDGET(element.elements[1].widget));\
	}\
}

#define GETSELECTEDRECORD(item, element) {\
	unsigned int status;\
	item = DTCs;\
	if (!item) {\
		sprintf(buff, "No DTCs available");\
		if (CHECKWIDGET(element.elements[1].widget, GTK_TYPE_LABEL))\
			gtk_label_set_text(GTK_LABEL(element.elements[1].widget), buff);\
		else { \
			if (CHECKWIDGET(element.elements[1].widget, GTK_TYPE_LIST_BOX))\
				gtk_list_box_append(GTK_LIST_BOX(element.elements[1].widget), gtk_label_new(buff));\
		}\
		return 0;\
	}\
	GetSelectedValueFromList(&status, __DTCRecord.elements[1].widget);\
	if (status != (unsigned int)(-1)) {\
		DTCRecord record;\
		record.DTCHB = status >> 16;\
		record.DTCMB = (status & 0xff00) >> 8;\
		record.DTCLB = status & 0xff;\
		item = AddRecord(&DTCs, record);\
	}\
}

#define GETSELECTEDSNAPSHOT(snapshot, item, element) {\
	unsigned int status;\
	if (!item->snapshots) {\
		sprintf(buff, "No snapshots available");\
		if (CHECKWIDGET(element.elements[1].widget, GTK_TYPE_LABEL))\
			gtk_label_set_text(GTK_LABEL(element.elements[1].widget), buff);\
		else { \
			if (CHECKWIDGET(element.elements[1].widget, GTK_TYPE_LIST_BOX))\
				gtk_list_box_append(GTK_LIST_BOX(element.elements[1].widget), gtk_label_new(buff));\
		}\
		return 0;\
	}\
	GetSelectedValueFromList(&status, __DTCSnapshotRecordNumber.elements[1].widget);\
	BYTE snapshotNumber = (BYTE)status;\
	snapshot = item->snapshots;\
	for (; snapshot && snapshotNumber != snapshot->SnapshotNumber; snapshot = snapshot->next);\
}

#define GETSELECTEDEXTENDED(extended, item, element) {\
	unsigned int status;\
	if (!item->extendedData) {\
		sprintf(buff, "No extended data available");\
		if (CHECKWIDGET(element.elements[1].widget, GTK_TYPE_LABEL))\
			gtk_label_set_text(GTK_LABEL(element.elements[1].widget), buff);\
		else { \
			if (CHECKWIDGET(element.elements[1].widget, GTK_TYPE_LIST_BOX))\
				gtk_list_box_append(GTK_LIST_BOX(element.elements[1].widget), gtk_label_new(buff));\
		}\
		return 0;\
	}\
	GetSelectedValueFromList(&status, __DTCExtDataRecordNumber.elements[1].widget);\
	BYTE extendedNumber = (BYTE)status;\
	extended = item->extendedData;\
	for (; extended && extendedNumber != extended->DTCExtDataNumber; extended = extended->next);\
}

unsigned int aux;

PanelElement _DTCStatusMask;
PanelElement _DTCMaskRecord;
PanelElement _DTCSnapshotRecordNumber;
PanelElement _DTCStoredDataRecordNumber;
PanelElement _DTCExtDataRecordNumber;
PanelElement _DTCExtDataRecordNumberLabel;
PanelElement _DTCSeverityMaskRecord;
PanelElement _MemorySelection;
PanelElement _UserDefDTCSnapshotRecordNumber;
PanelElement _FunctionalGroupIdentifier;
PanelElement _DTCReadinessGroupIdentifier;

PanelElement __DTCStatusAvailabilityMask;
PanelElement __DTCSeverityAvailabilityMask;
PanelElement __DTCFormatIdentifier;
PanelElement __DTCCount;
PanelElement __DTCRecord;
PanelElement __DTCStatus;
PanelElement __DTCSnapshotRecordNumber;
PanelElement __DTCDataIdentifier;
PanelElement __DTCSnapshotRecordNumberOfIdentifiers;
PanelElement __DTCSnapshotRecord;
PanelElement __DTCStoredDataRecordNumber;
PanelElement __DTCStoredDataRecordNumberOfIdentifiers;
PanelElement __DTCExtDataRecordNumber;
PanelElement __DTCExtDataRecord;
PanelElement __DTCSeverity;
PanelElement __DTCFunctionalUnit;
PanelElement __DTCFaultDetectionCounter;
PanelElement __FunctionalGroupIdentifier;
PanelElement __UserDefDTCSnapshotRecordNumber;
PanelElement __DTCReadinessGroupIdentifier;
PanelElement __MemorySelection;

SimplePanel RDTCIRequests[16] = {0};
SimplePanel RDTCIResponses[17] = {0};

ConfigPanel RDTCIPanel[25] = {
	{NULL, &RDTCIRequests[0], &RDTCIResponses[0]},
	{NULL, &RDTCIRequests[0], &RDTCIResponses[1]},
	{NULL, &RDTCIRequests[6], &RDTCIResponses[2]},
	{NULL, &RDTCIRequests[1], &RDTCIResponses[3]},
	{NULL, &RDTCIRequests[2], &RDTCIResponses[4]},
	{NULL, &RDTCIRequests[3], &RDTCIResponses[5]},
	{NULL, &RDTCIRequests[4], &RDTCIResponses[0]},
	{NULL, &RDTCIRequests[4], &RDTCIResponses[6]},
	{NULL, &RDTCIRequests[5], &RDTCIResponses[6]},
	{NULL, &RDTCIRequests[6], &RDTCIResponses[1]},
	{NULL, &RDTCIRequests[6], &RDTCIResponses[1]},
	{NULL, &RDTCIRequests[6], &RDTCIResponses[1]},
	{NULL, &RDTCIRequests[6], &RDTCIResponses[1]},
	{NULL, &RDTCIRequests[6], &RDTCIResponses[1]},
	{NULL, &RDTCIRequests[6], &RDTCIResponses[7]},
	{NULL, &RDTCIRequests[6], &RDTCIResponses[1]},
	{NULL, &RDTCIRequests[7], &RDTCIResponses[8]},
	{NULL, &RDTCIRequests[8], &RDTCIResponses[9]},
	{NULL, &RDTCIRequests[9], &RDTCIResponses[10]},
	{NULL, &RDTCIRequests[10], &RDTCIResponses[11]},
	{NULL, &RDTCIRequests[11], &RDTCIResponses[12]},
	{NULL, &RDTCIRequests[12], &RDTCIResponses[13]},
	{NULL, &RDTCIRequests[13], &RDTCIResponses[14]},
	{NULL, &RDTCIRequests[14], &RDTCIResponses[15]},
	{NULL, NULL, NULL}
};

ServiceParam _RDTCI = {
	"ReadDTCInformation",
	RDTCI_RequestService,
	RDTCI_ReceiveResponse,
	{
		{"reportNumberOfDTCByStatusMask", RNODTCBSM, &RDTCIPanel[0]},
		{"reportDTCByStatusMask", RDTCBSM, &RDTCIPanel[1]},
		{"reportDTCSnapshotIdentification", RDTCSSI, &RDTCIPanel[2]},
		{"reportDTCSnapshotRecordByDTCNumber", RDTCSSBDTC, &RDTCIPanel[3]},
		{"reportDTCStoredDataByRecordNumber", RDTCSDBRN, &RDTCIPanel[4]},
		{"reportDTCExtDataRecordByDTCNumber", RDTCEDRBDN, &RDTCIPanel[5]},
		{"reportNumberOfDTCBySeverityMaskRecord", RNODTCBSMR, &RDTCIPanel[6]},
		{"reportDTCBySeverityMaskRecord", RDTCBSMR, &RDTCIPanel[7]},
		{"reportSeverityInformationOfDTC", RSIODTC, &RDTCIPanel[8]},
		{"reportSupportedDTC", RSUPDTC, &RDTCIPanel[9]},
		{"reportFirstTestFailedDTC", RFTFDTC, &RDTCIPanel[10]},
		{"reportFirstConfirmedDTC", RFCDTC, &RDTCIPanel[11]},
		{"reportMostRecentTestFailedDTC", RMRTFDTC, &RDTCIPanel[12]},
		{"reportMostRecentConfirmedDTC", RMRCDTC, &RDTCIPanel[13]},
		{"reportDTCFaultDetectionCounter", RDTCFDC, &RDTCIPanel[14]},
		{"reportDTCWithPermanentStatus", RDTCWPS, &RDTCIPanel[15]},
		{"reportDTCExtDataRecordByRecordNumber", RDTCEDRBR, &RDTCIPanel[16]},
		{"reportUserDefMemoryDTCByStatusMask", RUDMDTCBSM, &RDTCIPanel[17]},
		{"reportUserDefMemoryDTCSnapshotRecordByDTCNumber", RUDMDTCSSBDTC, &RDTCIPanel[18]},
		{"reportUserDefMemoryDTCExtDataRecordByDTCNumber", RUDMDTCEDRBDN, &RDTCIPanel[19]},
		{"reportDTCExtendedDataRecordIdentification", RDTCEDI, &RDTCIPanel[20]},
		{"reportWWHOBDDTCByMaskRecord", RWWHOBDDTCBMR, &RDTCIPanel[21]},
		{"reportWWHOBDDTCWithPermanentStatus", RWWHOBDDTCWPS, &RDTCIPanel[22]},
		{"reportDTCInformationByDTCReadinessGroupIdentifier", RDTCBRGI, &RDTCIPanel[23]},
		{NULL, -1, NULL}}
};

int _DTCStatusMaskRequest() {
	
	if (GetByteFromEntry(&DTCSM, _DTCStatusMask.elements[1].widget)) {
		gtk_label_set_text(GTK_LABEL(_DTCStatusMask.elements[2].widget), "Value must be [0..255]");
		return 1;
	}
	
	VALID(_DTCStatusMask.elements[2].widget);

	if (!DTCSM)
		return 1;

	return 0;
}

int _DTCMaskRecordRequest() {
	if (GetValueFromComboBox(&aux, _DTCMaskRecord.elements[1].widget)) {
		NOTVALID(_DTCMaskRecord.elements[2].widget);
		return 1;
	}

	VALID(_DTCMaskRecord.elements[2].widget);

	if (!DTCMaskRecord) 
		DTCMaskRecord = (DTCRecord*)malloc(sizeof(DTCRecord));

	DTCMaskRecord->DTCHB = aux >> 16;
	DTCMaskRecord->DTCMB = (aux & 0xff00) >> 8;
	DTCMaskRecord->DTCLB = aux & 0xff;

	return 0;
}

int _DTCSnapshotRecordNumberRequest() {
	if (GetValueFromComboBox(&aux, _DTCSnapshotRecordNumber.elements[1].widget)) {
		NOTVALID(_DTCSnapshotRecordNumber.elements[2].widget);
		return 1;
	}

	VALID(_DTCSnapshotRecordNumber.elements[2].widget);

	SETBYTE(DTCSSRN, (BYTE)aux);

	return 0;
}

int _DTCStoredDataRecordNumberRequest() {
	if (GetByteFromEntry(&DTCSDRN, _DTCStoredDataRecordNumber.elements[1].widget)) {
		gtk_label_set_text(GTK_LABEL(_DTCStoredDataRecordNumber.elements[2].widget), "Value must be [0..255]");
		return 1;
	}
	
	VALID(_DTCStoredDataRecordNumber.elements[2].widget);

	if (!DTCSDRN) return 1;

	return 0;
}

int _DTCExtDataRecordNumberRequest() {
	if (GetValueFromComboBox(&aux, _DTCExtDataRecordNumber.elements[1].widget)) {
		NOTVALID(_DTCExtDataRecordNumber.elements[2].widget);
		return 1;
	}

	VALID(_DTCExtDataRecordNumber.elements[2].widget);

	SETBYTE(DTCEDRN, (BYTE)aux);

	return 0;
}

int _DTCExtDataRecordNumberLabelRequest() {
	if (GetByteFromEntry(&DTCEDRN, _DTCExtDataRecordNumberLabel.elements[1].widget)) {
		gtk_label_set_text(GTK_LABEL(_DTCExtDataRecordNumberLabel.elements[2].widget), "Invalid input");
		return 1;
	}

	subfunctionElement subfunction = temp[serviceIndex].parameters->subfunctionList[subfunctionIndex];

	if (*DTCEDRN < 1 || *DTCEDRN > 0xfd && subfunction.subfunctionValue == RDTCEDI) {
		gtk_label_set_text(GTK_LABEL(_DTCExtDataRecordNumberLabel.elements[2].widget), "Value must be [1..254]");
		return 1;
	}

	if (*DTCEDRN > 0xef && subfunction.subfunctionValue == RDTCEDRBR) {
		gtk_label_set_text(GTK_LABEL(_DTCExtDataRecordNumberLabel.elements[2].widget), "Value must be [0..239]");
		return 1;
	}

	VALID(_DTCExtDataRecordNumberLabel.elements[2].widget);

	if (!DTCEDRN) return 1;

	return 0;
}

int _DTCSeverityMaskRecordRequest() {

	if (GetByteFromEntry(&DTCSVM, _DTCSeverityMaskRecord.elements[2].widget)) {
		gtk_label_set_text(GTK_LABEL(_DTCSeverityMaskRecord.elements[3].widget), "Value must be [0..255]");
		return 1;
	}

	VALID(_DTCSeverityMaskRecord.elements[3].widget);

	if (!DTCSVM) return 1;

	if (GetByteFromEntry(&DTCSM, _DTCSeverityMaskRecord.elements[5].widget)) {
		gtk_label_set_text(GTK_LABEL(_DTCSeverityMaskRecord.elements[6].widget), "Value must be [0..255]");
		return 1;
	}
	
	if (!DTCSM) return 1;

	VALID(_DTCSeverityMaskRecord.elements[6].widget);
	
	return 0;
}

int _MemorySelectionRequest() {

	if (GetByteFromEntry(&MEMYS, _MemorySelection.elements[1].widget)) {
		gtk_label_set_text(GTK_LABEL(_MemorySelection.elements[2].widget), "Value must be [0..255]");
		return 1;
	}

	VALID(_MemorySelection.elements[2].widget);

	if (!MEMYS) return 1;

	return 0;
}

int _UserDefDTCSnapshotRecordNumberRequest() {
	if (GetByteFromEntry(&UDDTCSSRN, _UserDefDTCSnapshotRecordNumber.elements[1].widget)) {
		gtk_label_set_text(GTK_LABEL(_UserDefDTCSnapshotRecordNumber.elements[2].widget), "Value must be [0..255]");
		return 1;
	}

	VALID(_UserDefDTCSnapshotRecordNumber.elements[2].widget);

	if (!UDDTCSSRN) return 1;

	return 0;
}

int _FunctionalGroupIdentifierRequest() {
	
	if (GetByteFromEntry(&FGID, _FunctionalGroupIdentifier.elements[1].widget) || aux > 0xfe) {
		gtk_label_set_text(GTK_LABEL(_FunctionalGroupIdentifier.elements[2].widget), "Value must be [0..254]");
		return 1;
	}

	VALID(_FunctionalGroupIdentifier.elements[2].widget);

	if (!FGID) return 1;

	return 0;
}

int _DTCReadinessGroupIdentifierRequest() {
	if (GetByteFromEntry(&DTCRGI, _DTCReadinessGroupIdentifier.elements[1].widget) || aux > 0xfe) {
		gtk_label_set_text(GTK_LABEL(_DTCReadinessGroupIdentifier.elements[2].widget), "Value must be [0..254]");
		return 1;
	}

	VALID(_DTCReadinessGroupIdentifier.elements[2].widget);

	if (DTCRGI) return 1;

	return 0;
}

int _DTCStatusMaskUpdate() {
	SETENTRYTEXT(_DTCStatusMask, DTCSM);
	return 0;
}

int _DTCMaskRecordUpdate() {
	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(_DTCMaskRecord.elements[1].widget));

	if (!DTCs)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(_DTCMaskRecord.elements[1].widget), "No DTCs at this moment!");

	DTCItem* item = DTCs;
	for (; item; item = item->next) {
		sprintf(buff, "%x%x%x", item->DTC.record.DTCHB, item->DTC.record.DTCMB, item->DTC.record.DTCLB);
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(_DTCMaskRecord.elements[1].widget), buff);
	}

	return 0;
}


int _DTCSnapshotRecordNumberUpdate() {
	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(_DTCSnapshotRecordNumber.elements[1].widget));

	if (!DTCs) {
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(_DTCSnapshotRecordNumber.elements[1].widget), "No DTCs at this moment!");
		return 0;
	}

	GetValueFromComboBox(&aux, _DTCMaskRecord.elements[1].widget);
	
	DTCSnapshotItem* item = NULL;

	if (aux != (unsigned int)(-1)) {

		DTCRecord Record;

		Record.DTCHB = aux >> 16;
		Record.DTCMB = (aux & 0xff00) >> 8;
		Record.DTCLB = aux & 0xff;

		item = (AddRecord(&DTCs, Record))->snapshots;
	}

	if (!item)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(_DTCSnapshotRecordNumber.elements[1].widget), "No snapshots at this moment!");
	for (; item; item = item->next) {
		sprintf(buff, "%x", item->SnapshotNumber);
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(_DTCSnapshotRecordNumber.elements[1].widget), buff);
	}

	return 0;
}

int _DTCStoredDataRecordNumberUpdate() {
	SETENTRYTEXT(_DTCStoredDataRecordNumber, DTCSDRN);
	return 0;
}

int _DTCExtDataRecordNumberUpdate() {
	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(_DTCExtDataRecordNumber.elements[1].widget));

	if (!DTCs) {
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(_DTCExtDataRecordNumber.elements[1].widget), "No DTCs at this moment!");
		return 0;
	}

	GetValueFromComboBox(&aux, _DTCExtDataRecordNumber.elements[1].widget);

	DTCExtDataItem* item = NULL;

	if (aux != (unsigned int)(-1)) {

		DTCRecord Record;

		Record.DTCHB = aux >> 16;
		Record.DTCMB = (aux & 0xff00) >> 8;
		Record.DTCLB = aux & 0xff;

		item = (AddRecord(&DTCs, Record))->extendedData;
	}

	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(_DTCExtDataRecordNumber.elements[1].widget), "FF");

	for(; item; item = item->next) {
		sprintf(buff, "%x", item->DTCExtDataNumber);
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(_DTCExtDataRecordNumber.elements[1].widget), buff);
	}

	return 0;
}

int _DTCExtDataRecordNumberLabelUpdate() {
	SETENTRYTEXT(_DTCExtDataRecordNumberLabel, DTCEDRN);
	return 0;
}

int _DTCSeverityMaskRecordUpdate() {
	
	GtkEntryBuffer* buffer = gtk_entry_get_buffer(GTK_ENTRY(_DTCSeverityMaskRecord.elements[2].widget));
	if (DTCSVM) {
		sprintf(buff, "%x", *DTCSVM);
		gtk_entry_buffer_set_text(buffer, buff, strlen(buff));
	}

	buffer = gtk_entry_get_buffer(GTK_ENTRY(_DTCSeverityMaskRecord.elements[5].widget));
	if (DTCSAM) {
		sprintf(buff, "%x", *DTCSAM);
		gtk_entry_buffer_set_text(buffer, buff, strlen(buff));
	}
	return 0;
}

int _MemorySelectionUpdate() {
	SETENTRYTEXT(_MemorySelection, MEMYS);
	return 0;
}

int _UserDefDTCSnapshotRecordNumberUpdate() {
	SETENTRYTEXT(_UserDefDTCSnapshotRecordNumber, UDDTCSSRN);
	return 0;
}

int _FunctionalGroupIdentifierUpdate() {
	SETENTRYTEXT(_FunctionalGroupIdentifier, FGID);
	return 0;
}

int _DTCReadinessGroupIdentifierUpdate() {
	SETENTRYTEXT(_DTCReadinessGroupIdentifier, DTCRGI);
	return 0;
}

int __DTCStatusAvailabilityMaskUpdate() {
	SETLABELTEXT(__DTCStatusAvailabilityMask, DTCSAM);
	return 0;
}
int __DTCSeverityAvailabilityMaskUpdate() {
	SETLABELTEXT(__DTCSeverityAvailabilityMask, DTCSVAM);
	return 0;
}
int __DTCFormatIdentifierUpdate() {

	if (!DTCFID)
		sprintf(buff, "Not set");
	else {
		switch (*DTCFID) {
			case SAE_J2012_F0:
				sprintf(buff, "SAE_J2012_F0");
				break;
			case ISO_14229:
				sprintf(buff, "ISO_14229");
				break;
			case SAE_J1939:
				sprintf(buff, "SAE_J1939");
				break;
			case ISO_11992:
				sprintf(buff, "ISO_11992");
				break;
			case SAE_J2012_F4:
				sprintf(buff, "SAE_J2012_F4");
				break;
			default:
				sprintf(buff, "Unknown");
				break;
		}
	}

	gtk_label_set_text(GTK_LABEL(__DTCFormatIdentifier.elements[1].widget), buff);

	return 0;
}

int __DTCCountUpdate() {
	if (!DTCCount) {
		sprintf(buff, "Not set");
		gtk_label_set_text(GTK_LABEL(__DTCCount.elements[2].widget), buff);
		gtk_label_set_text(GTK_LABEL(__DTCCount.elements[4].widget), buff);
		return 0;
	}

	sprintf(buff, "%x", (*DTCCount & 0xff00) >> 8);
	gtk_label_set_text(GTK_LABEL(__DTCCount.elements[2].widget), buff);
	sprintf(buff, "%x", *DTCCount & 0xff);
	gtk_label_set_text(GTK_LABEL(__DTCCount.elements[4].widget), buff);
	return 0;
}

int __DTCRecordUpdate() {

	EMPTYLIST(__DTCRecord);

	if (!DTCs)
		gtk_list_box_append(GTK_LIST_BOX(__DTCRecord.elements[1].widget), gtk_label_new("No recorded DTC"));
	
	DTCItem* item = DTCs;
	for (; item; item = item->next) {
		sprintf(buff, "%x%x%x", item->DTC.record.DTCHB, item->DTC.record.DTCMB, item->DTC.record.DTCLB);
		gtk_list_box_append(GTK_LIST_BOX(__DTCRecord.elements[1].widget), gtk_label_new(buff));
	}
	return 0;
}

int __DTCStatusUpdate() {

	DTCItem* item;
	GETSELECTEDRECORD(item, __DTCStatus);

	if (!item) {
		sprintf(buff, "Status Unavailable");
		gtk_label_set_text(GTK_LABEL(__DTCStatus.elements[1].widget), buff);
		return 0;
	}

	SETLABELTEXT(__DTCStatus, item->DTC.SODTC);

	return 0;
}

int __DTCSnapshotRecordNumberUpdate() {

	EMPTYLIST(__DTCSnapshotRecordNumber);
	DTCItem* item;
	GETSELECTEDRECORD(item, __DTCSnapshotRecordNumber);

	if (!item->snapshots) {
		sprintf(buff, "Snapshot numbers Unavailable");
		gtk_list_box_append(GTK_LIST_BOX(__DTCSnapshotRecordNumber.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
		return 0;
	}

	DTCSnapshotItem* snapshot = item->snapshots;
	for (; snapshot; snapshot = snapshot->next) {
		sprintf(buff, "%x", snapshot->SnapshotNumber);
		gtk_list_box_append(GTK_LIST_BOX(__DTCSnapshotRecordNumber.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
	}

	return 0;
}

int __DTCDataIdentifierUpdate() {

	EMPTYLIST(__DTCSnapshotRecord);
	DTCItem* item;
	GETSELECTEDRECORD(item, __DTCSnapshotRecord);

	if (!item->snapshots) {
		gtk_label_set_text(GTK_LABEL(__DTCDataIdentifier.elements[1].widget), "Snapshot number of identifiers Unavailable");
		return 0;
	}

	DTCSnapshotItem* snapshot;
	GETSELECTEDSNAPSHOT(snapshot, item, __DTCSnapshotRecord);

	if (!snapshot) {
		gtk_label_set_text(GTK_LABEL(__DTCDataIdentifier.elements[1].widget), "Snapshot number of identifiers Unavailable");
		return 0;
	}

	unsigned short int did = (snapshot->DTCSnapshot.DIDBH << 8) + snapshot->DTCSnapshot.DIDBL;
	
	SETLABELTEXT(__DTCDataIdentifier, &did);

	return 0;
}

int __DTCSnapshotRecordNumberOfIdentifiersUpdate() {

	EMPTYLIST(__DTCSnapshotRecordNumberOfIdentifiers);
	DTCItem* item;
	GETSELECTEDRECORD(item, __DTCSnapshotRecordNumberOfIdentifiers);

	if (!item->snapshots) {
		sprintf(buff, "Snapshot number of identifiers Unavailable");
		gtk_list_box_append(GTK_LIST_BOX(__DTCSnapshotRecordNumberOfIdentifiers.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
		return 0;
	}

	DTCSnapshotItem* snapshot = item->snapshots;
	for (; snapshot; snapshot = snapshot->next) {
		sprintf(buff, "%x", snapshot->SnapshotNumberOfIdentifiers);
		gtk_list_box_append(GTK_LIST_BOX(__DTCSnapshotRecordNumberOfIdentifiers.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
	}

	return 0;
}

int __DTCSnapshotRecordUpdate() {

	EMPTYLIST(__DTCSnapshotRecord);
	DTCItem* item;
	GETSELECTEDRECORD(item, __DTCSnapshotRecord);

	if (!item->snapshots) {
		sprintf(buff, "Snapshot number of identifiers Unavailable");
		gtk_list_box_append(GTK_LIST_BOX(__DTCSnapshotRecord.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
		return 0;
	}

	DTCSnapshotItem* snapshot;
	GETSELECTEDSNAPSHOT(snapshot, item, __DTCSnapshotRecord);

	if (snapshot && snapshot->DTCSnapshot.dataRecord.data) {
		for (unsigned int i = 0; i < snapshot->DTCSnapshot.dataRecord.Length; ++i) {
			sprintf(buff, "%x", snapshot->DTCSnapshot.dataRecord.data[i]);
			gtk_list_box_append(GTK_LIST_BOX(__DTCSnapshotRecord.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
		}
	} else {
		sprintf(buff, "Snapshot number of identifiers Unavailable");
		gtk_list_box_append(GTK_LIST_BOX(__DTCSnapshotRecord.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
	}
	return 0;
}

int __DTCStoredDataRecordNumberUpdate() {
	EMPTYLIST(__DTCStoredDataRecordNumber);
	DTCItem* item;
	GETSELECTEDRECORD(item, __DTCStoredDataRecordNumber);

	if (!item) {
		sprintf(buff, "Snapshot numbers Unavailable");
		gtk_list_box_append(GTK_LIST_BOX(__DTCStoredDataRecordNumber.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
		return 0;
	}

	DTCStoredDataItem* storedData = item->storedData;
	for (; storedData; storedData = storedData->next) {
		sprintf(buff, "%x", storedData->StoredDataNumber);
		gtk_list_box_append(GTK_LIST_BOX(__DTCStoredDataRecordNumber.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
	}

	return 0;
}

int __DTCStoredDataRecordNumberOfIdentifiersUpdate() {
	EMPTYLIST(__DTCStoredDataRecordNumberOfIdentifiers);
	DTCItem* item;
	GETSELECTEDRECORD(item, __DTCStoredDataRecordNumberOfIdentifiers);

	if (!item) {
		sprintf(buff, "Snapshot number of identifiers Unavailable");
		gtk_list_box_append(GTK_LIST_BOX(__DTCStoredDataRecordNumberOfIdentifiers.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
		return 0;
	}

	DTCStoredDataItem* storedData = item->storedData;
	for (; storedData; storedData = storedData->next) {
		sprintf(buff, "%x", storedData->StoredDataNumberOfIdentifiers);
		gtk_list_box_append(GTK_LIST_BOX(__DTCStoredDataRecordNumberOfIdentifiers.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
	}

	return 0;
}

int __DTCExtDataRecordNumberUpdate() {
	EMPTYLIST(__DTCExtDataRecordNumber);
	DTCItem* item;
	GETSELECTEDRECORD(item, __DTCExtDataRecordNumber);

	if (!item) {
		sprintf(buff, "Snapshot numbers Unavailable");
		gtk_list_box_append(GTK_LIST_BOX(__DTCExtDataRecordNumber.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
		return 0;
	}

	DTCExtDataItem* extendedData = item->extendedData;
	for (; extendedData; extendedData = extendedData->next) {
		sprintf(buff, "%x", extendedData->DTCExtDataNumber);
		gtk_list_box_append(GTK_LIST_BOX(__DTCExtDataRecordNumber.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
	}

	return 0;
}

int __DTCExtDataRecordUpdate() {
	EMPTYLIST(__DTCExtDataRecord);
	DTCItem* item;
	GETSELECTEDRECORD(item, __DTCExtDataRecord);

	if (!item) {
		sprintf(buff, "Snapshot number of identifiers Unavailable");
		gtk_list_box_append(GTK_LIST_BOX(__DTCExtDataRecord.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
		return 0;
	}

	DTCExtDataItem* extendedData;
	GETSELECTEDEXTENDED(extendedData, item, __DTCExtDataRecord);

	for (unsigned int i = 0; i < extendedData->DTCExtDataRecord.Length; ++i) {
		sprintf(buff, "%x", extendedData->DTCExtDataRecord.data[i]);
		gtk_list_box_append(GTK_LIST_BOX(__DTCExtDataRecord.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
	}

	return 0;
}

int __DTCSeverityUpdate() {
	EMPTYLIST(__DTCSeverity);
	DTCItem* item;
	GETSELECTEDRECORD(item, __DTCSeverity);

	if (!item) {
		sprintf(buff, "Severity of DTC unavailable");
		gtk_list_box_append(GTK_LIST_BOX(__DTCSeverity.elements[1].widget), GTK_WIDGET(gtk_label_new(buff)));
		return 0;
	}

	SETLABELTEXT(__DTCSeverity, item->severity);

	return 0;

}

int __DTCFunctionalUnitUpdate() {

	DTCItem* item;
	GETSELECTEDRECORD(item, __DTCFunctionalUnit);

	if (!item) {
		sprintf(buff, "Functional Unit Unavailable");
		gtk_label_set_text(GTK_LABEL(__DTCFunctionalUnit.elements[1].widget), buff);
		return 0;
	}

	SETLABELTEXT(__DTCFunctionalUnit, item->functionalUnit);

	return 0;
}

int __DTCFaultDetectionCounterUpdate() {

	DTCItem* item;
	GETSELECTEDRECORD(item, __DTCFaultDetectionCounter);

	if (!item) {
		sprintf(buff, "Fault Counter Unavailable");
		gtk_label_set_text(GTK_LABEL(__DTCFaultDetectionCounter.elements[1].widget), buff);
		return 0;
	}

	SETLABELTEXT(__DTCFaultDetectionCounter, item->DTC.SODTC);

	return 0;
}

int __FunctionalGroupIdentifierUpdate() {
	SETLABELTEXT(__FunctionalGroupIdentifier, FGID);
	return 0;
}

int __UserDefDTCSnapshotRecordNumberUpdate() {
	SETLABELTEXT(__UserDefDTCSnapshotRecordNumber, UDDTCSSRN);
	return 0;
}

int __DTCReadinessGroupIdentifierUpdate() {
	SETLABELTEXT(__DTCReadinessGroupIdentifier, DTCRGI);
	return 0;
}

int __MemorySelectionUpdate() {
	SETLABELTEXT(__MemorySelection, MEMYS);
	return 0;
}

int CreateRDCTIRequests() {

	NEWENTRY(_DTCStatusMask, "DTCStatusMask:");
	_DTCStatusMask.requestFunc = _DTCStatusMaskRequest;
	_DTCStatusMask.updateFunc = _DTCStatusMaskUpdate;

	NEWCOMBO(_DTCMaskRecord, "DTCMaskRecord:", True);
	_DTCMaskRecord.requestFunc = _DTCMaskRecordRequest;
	_DTCMaskRecord.updateFunc = _DTCMaskRecordUpdate;

	NEWCOMBO(_DTCSnapshotRecordNumber, "DTCSnapshotRecordNumber:", False);
	_DTCSnapshotRecordNumber.requestFunc = _DTCSnapshotRecordNumberRequest;
	_DTCSnapshotRecordNumber.updateFunc = _DTCSnapshotRecordNumberUpdate;

	NEWENTRY(_DTCStoredDataRecordNumber, "DTCStoredDataRecordNumber:");
	_DTCStoredDataRecordNumber.requestFunc = _DTCStoredDataRecordNumberRequest;
	_DTCStoredDataRecordNumber.updateFunc = _DTCStoredDataRecordNumberUpdate;

	NEWCOMBO(_DTCExtDataRecordNumber, "DTCExtDataRecordNumber:", False);
	_DTCExtDataRecordNumber.requestFunc = _DTCExtDataRecordNumberRequest;
	_DTCExtDataRecordNumber.updateFunc = _DTCExtDataRecordNumberUpdate;
	
	NEWENTRY(_DTCExtDataRecordNumberLabel, "DTCExtDataRecordNumber:");
	_DTCExtDataRecordNumberLabel.requestFunc = _DTCExtDataRecordNumberLabelRequest;
	_DTCExtDataRecordNumberLabel.updateFunc = _DTCExtDataRecordNumberLabelUpdate;

	_DTCSeverityMaskRecord.elements = (SimpleElement*)malloc(8 * sizeof(SimpleElement));
	_DTCSeverityMaskRecord.elements[0].widget = gtk_label_new("DTCSeverityMaskRecord");
	_DTCSeverityMaskRecord.elements[0].xPos = 0, _DTCSeverityMaskRecord.elements[0].yPos = 0;
	_DTCSeverityMaskRecord.elements[1].widget = gtk_label_new("DTCSeverityMask:");
	_DTCSeverityMaskRecord.elements[1].xPos = 0, _DTCSeverityMaskRecord.elements[1].yPos = 1;
	_DTCSeverityMaskRecord.elements[2].widget = gtk_entry_new();
	_DTCSeverityMaskRecord.elements[2].xPos = 1, _DTCSeverityMaskRecord.elements[2].yPos = 1;
	gtk_entry_set_max_length(GTK_ENTRY(_DTCSeverityMaskRecord.elements[2].widget), 2);
	gtk_entry_set_placeholder_text(GTK_ENTRY(_DTCSeverityMaskRecord.elements[2].widget), "XX");
	_DTCSeverityMaskRecord.elements[3].widget = gtk_label_new(NULL);
	_DTCSeverityMaskRecord.elements[3].xPos = 2, _DTCSeverityMaskRecord.elements[3].yPos = 1;
	_DTCSeverityMaskRecord.elements[4].widget = gtk_label_new("DTCStatusMask:");
	_DTCSeverityMaskRecord.elements[4].xPos = 0, _DTCSeverityMaskRecord.elements[4].yPos = 2;
	_DTCSeverityMaskRecord.elements[5].widget = gtk_entry_new();
	_DTCSeverityMaskRecord.elements[5].xPos = 1, _DTCSeverityMaskRecord.elements[5].yPos = 2;
	gtk_entry_set_max_length(GTK_ENTRY(_DTCSeverityMaskRecord.elements[5].widget), 2);
	gtk_entry_set_placeholder_text(GTK_ENTRY(_DTCSeverityMaskRecord.elements[5].widget), "XX");
	_DTCSeverityMaskRecord.elements[6].widget = gtk_label_new(NULL);
	_DTCSeverityMaskRecord.elements[6].xPos = 2, _DTCSeverityMaskRecord.elements[6].yPos = 2;
	_DTCSeverityMaskRecord.elements[7].widget = NULL;
	_DTCSeverityMaskRecord.requestFunc = _DTCSeverityMaskRecordRequest;
	_DTCSeverityMaskRecord.updateFunc = _DTCSeverityMaskRecordUpdate;
	CREATEELEMENT(_DTCSeverityMaskRecord);

	NEWENTRY(_MemorySelection, "MemorySelection:");
	_MemorySelection.requestFunc = _MemorySelectionRequest;
	_MemorySelection.updateFunc = _MemorySelectionUpdate;
	
	NEWENTRY(_UserDefDTCSnapshotRecordNumber, "UserDefDTCSnapshotRecordNumber:");
	_UserDefDTCSnapshotRecordNumber.requestFunc = _UserDefDTCSnapshotRecordNumberRequest;
	_UserDefDTCSnapshotRecordNumber.updateFunc = _UserDefDTCSnapshotRecordNumberUpdate;

	NEWENTRY(_FunctionalGroupIdentifier, "FunctionalGroupIdentifier:");
	_FunctionalGroupIdentifier.requestFunc = _FunctionalGroupIdentifierRequest;
	_FunctionalGroupIdentifier.updateFunc = _FunctionalGroupIdentifierUpdate;

	NEWENTRY(_DTCReadinessGroupIdentifier, "DTCReadinessGroupIdentifier:");
	_DTCReadinessGroupIdentifier.requestFunc = _DTCReadinessGroupIdentifierRequest;
	_DTCReadinessGroupIdentifier.updateFunc = _DTCReadinessGroupIdentifierUpdate;

	RDTCIRequests[0].elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	RDTCIRequests[0].elements[0].handle = &_DTCStatusMask;
	RDTCIRequests[0].elements[0].xPos = 0, RDTCIRequests[0].elements[0].yPos = 0;
	RDTCIRequests[0].elements[1].handle = NULL;

	RDTCIRequests[1].elements = (PanelHandle*)malloc(3 * sizeof(PanelHandle));
	RDTCIRequests[1].elements[0].handle = &_DTCMaskRecord;
	RDTCIRequests[1].elements[0].xPos = 0, RDTCIRequests[1].elements[0].yPos = 0;
	RDTCIRequests[1].elements[1].handle = &_DTCSnapshotRecordNumber;
	RDTCIRequests[1].elements[1].xPos = 0, RDTCIRequests[1].elements[1].yPos = 1;
	RDTCIRequests[1].elements[2].handle = NULL;

	RDTCIRequests[2].elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	RDTCIRequests[2].elements[0].handle = &_DTCStoredDataRecordNumber;
	RDTCIRequests[2].elements[0].xPos = 0, RDTCIRequests[2].elements[0].yPos = 0;
	RDTCIRequests[2].elements[1].handle = NULL;

	RDTCIRequests[3].elements = (PanelHandle*)malloc(3 * sizeof(PanelHandle));
	RDTCIRequests[3].elements[0].handle = &_DTCMaskRecord;
	RDTCIRequests[3].elements[0].xPos = 0, RDTCIRequests[3].elements[0].yPos = 0;
	RDTCIRequests[3].elements[1].handle = &_DTCExtDataRecordNumber;
	RDTCIRequests[3].elements[1].xPos = 0, RDTCIRequests[3].elements[1].yPos = 1;
	RDTCIRequests[3].elements[2].handle = NULL;

	RDTCIRequests[4].elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	RDTCIRequests[4].elements[0].handle = &_DTCSeverityMaskRecord;
	RDTCIRequests[4].elements[0].xPos = 0, RDTCIRequests[4].elements[0].yPos = 0;
	RDTCIRequests[4].elements[1].handle = NULL;

	RDTCIRequests[5].elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	RDTCIRequests[5].elements[0].handle = &_DTCMaskRecord;
	RDTCIRequests[5].elements[0].xPos = 0, RDTCIRequests[5].elements[0].yPos = 0;
	RDTCIRequests[5].elements[1].handle = NULL;
	
	RDTCIRequests[6].elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	RDTCIRequests[6].elements[0].handle = &_NoParameters;
	RDTCIRequests[6].elements[0].xPos = 0, RDTCIRequests[6].elements[0].yPos = 0;
	RDTCIRequests[6].elements[1].handle = NULL;

	RDTCIRequests[7].elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	RDTCIRequests[7].elements[0].handle = &_DTCExtDataRecordNumberLabel;
	RDTCIRequests[7].elements[0].xPos = 0, RDTCIRequests[7].elements[0].yPos = 0;
	RDTCIRequests[7].elements[1].handle = NULL;

	RDTCIRequests[8].elements = (PanelHandle*)malloc(3 * sizeof(PanelHandle));
	RDTCIRequests[8].elements[0].handle = &_DTCStatusMask;
	RDTCIRequests[8].elements[0].xPos = 0, RDTCIRequests[8].elements[0].yPos = 0;
	RDTCIRequests[8].elements[1].handle = &_MemorySelection;
	RDTCIRequests[8].elements[1].xPos = 0, RDTCIRequests[8].elements[1].yPos = 1;
	RDTCIRequests[8].elements[2].handle = NULL;

	RDTCIRequests[9].elements = (PanelHandle*)malloc(4 * sizeof(PanelHandle));
	RDTCIRequests[9].elements[0].handle = &_DTCMaskRecord;
	RDTCIRequests[9].elements[0].xPos = 0, RDTCIRequests[9].elements[0].yPos = 0;
	RDTCIRequests[9].elements[1].handle = &_UserDefDTCSnapshotRecordNumber;
	RDTCIRequests[9].elements[1].xPos = 0, RDTCIRequests[9].elements[1].yPos = 1;
	RDTCIRequests[9].elements[2].handle = &_MemorySelection;
	RDTCIRequests[9].elements[2].xPos = 0, RDTCIRequests[9].elements[2].yPos = 2;
	RDTCIRequests[9].elements[3].handle = NULL;
	
	RDTCIRequests[10].elements = (PanelHandle*)malloc(4 * sizeof(PanelHandle));
	RDTCIRequests[10].elements[0].handle = &_DTCMaskRecord;
	RDTCIRequests[10].elements[0].xPos = 0, RDTCIRequests[10].elements[0].yPos = 0;
	RDTCIRequests[10].elements[1].handle = &_DTCExtDataRecordNumber;
	RDTCIRequests[10].elements[1].xPos = 0, RDTCIRequests[10].elements[1].yPos = 1;
	RDTCIRequests[10].elements[2].handle = &_MemorySelection;
	RDTCIRequests[10].elements[2].xPos = 0, RDTCIRequests[10].elements[2].yPos = 2;
	RDTCIRequests[10].elements[3].handle = NULL;

	RDTCIRequests[11].elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	RDTCIRequests[11].elements[0].handle = &_DTCExtDataRecordNumberLabel;
	RDTCIRequests[11].elements[0].xPos = 0, RDTCIRequests[11].elements[0].yPos = 0;
	RDTCIRequests[11].elements[1].handle = NULL;

	RDTCIRequests[12].elements = (PanelHandle*)malloc(3 * sizeof(PanelHandle));
	RDTCIRequests[12].elements[0].handle = &_FunctionalGroupIdentifier;
	RDTCIRequests[12].elements[0].xPos = 0, RDTCIRequests[12].elements[0].yPos = 0;
	RDTCIRequests[12].elements[1].handle = &_DTCSeverityMaskRecord;
	RDTCIRequests[12].elements[1].xPos = 0, RDTCIRequests[12].elements[1].yPos = 1;
	RDTCIRequests[12].elements[2].handle = NULL;

	RDTCIRequests[13].elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	RDTCIRequests[13].elements[0].handle = &_FunctionalGroupIdentifier;
	RDTCIRequests[13].elements[0].xPos = 0, RDTCIRequests[13].elements[0].yPos = 0;
	RDTCIRequests[13].elements[1].handle = NULL;
	
	RDTCIRequests[14].elements = (PanelHandle*)malloc(3 * sizeof(PanelHandle));
	RDTCIRequests[14].elements[0].handle = &_FunctionalGroupIdentifier;
	RDTCIRequests[14].elements[0].xPos = 0, RDTCIRequests[14].elements[0].yPos = 0;
	RDTCIRequests[14].elements[1].handle = &_DTCReadinessGroupIdentifier;
	RDTCIRequests[14].elements[1].xPos = 0, RDTCIRequests[14].elements[1].yPos = 1;
	RDTCIRequests[14].elements[2].handle = NULL;

	/* For Debug 
	DTCRecord rec;
	rec.DTCHB = 0x12;
	rec.DTCMB = 0x34;
	rec.DTCLB = 0x56;
	DTCItem* temp = AddRecord(&DTCs, rec);
	temp->DTC.SODTC = (BYTE*)malloc(sizeof(BYTE));
	*(temp->DTC.SODTC) = 0x24;
	rec.DTCHB = 0x78;
	rec.DTCMB = 0x9a;
	rec.DTCLB = 0xbc;
	temp = AddRecord(&DTCs, rec);
	temp->DTC.SODTC = (BYTE*)malloc(sizeof(BYTE));
	*(temp->DTC.SODTC) = 0x10;
	/*Enc Debug*/
}

int CreateRDTCIResponse() {
	NEWLABEL(__DTCStatusAvailabilityMask, "DTCStatusAvailabilityMask:");
	__DTCStatusAvailabilityMask.requestFunc = NULL;
	__DTCStatusAvailabilityMask.updateFunc = __DTCStatusAvailabilityMaskUpdate;

	NEWLABEL(__DTCSeverityAvailabilityMask, "DTCSeverityAvailabilityMask:");
	__DTCSeverityAvailabilityMask.requestFunc = NULL;
	__DTCSeverityAvailabilityMask.updateFunc = __DTCSeverityAvailabilityMaskUpdate;
	
	NEWLABEL(__DTCFormatIdentifier, "DTCFormatIdentifier:");
	__DTCFormatIdentifier.requestFunc = NULL;
	__DTCFormatIdentifier.updateFunc = __DTCFormatIdentifierUpdate;
	
	NEWLABEL(__FunctionalGroupIdentifier, "FunctionalGroupIdentifier:");
	__FunctionalGroupIdentifier.requestFunc = NULL;
	__FunctionalGroupIdentifier.updateFunc = __FunctionalGroupIdentifierUpdate;
	
	NEWLABEL(__MemorySelection, "MemorySelection:");
	__MemorySelection.requestFunc = NULL;
	__MemorySelection.updateFunc = __MemorySelectionUpdate;
	
	__DTCCount.elements = (SimpleElement*)malloc(6 * sizeof(SimpleElement));
	__DTCCount.elements[0].widget = gtk_label_new("DTCCount");
	__DTCCount.elements[0].xPos = 0, __DTCCount.elements[0].yPos = 0;
	__DTCCount.elements[1].widget = gtk_label_new("HighByte:");
	__DTCCount.elements[1].xPos = 0, __DTCCount.elements[1].yPos = 1;
	__DTCCount.elements[2].widget = gtk_label_new(NULL);
	__DTCCount.elements[2].xPos = 1, __DTCCount.elements[2].yPos = 1;
	__DTCCount.elements[3].widget = gtk_label_new("LowByte:");
	__DTCCount.elements[3].xPos = 0, __DTCCount.elements[3].yPos = 2;
	__DTCCount.elements[4].widget = gtk_label_new(NULL);
	__DTCCount.elements[4].xPos = 1, __DTCCount.elements[4].yPos = 2;
	__DTCCount.elements[5].widget = NULL;
	CREATEELEMENT(__DTCCount);
	__DTCCount.requestFunc = NULL;
	__DTCCount.updateFunc = __DTCCountUpdate;

	NEWLABEL(__DTCSeverity, "DTCSeverity:");
	__DTCSeverity.requestFunc = NULL;
	__DTCSeverity.updateFunc = __DTCSeverityUpdate;

	NEWLABEL(__DTCFunctionalUnit, "DTCFunctionalUnit:");
	__DTCFunctionalUnit.requestFunc = NULL;
	__DTCFunctionalUnit.updateFunc = __DTCFunctionalUnitUpdate;

	NEWLIST(__DTCRecord, "DTCRecord[]", GTK_SELECTION_SINGLE);
	__DTCRecord.requestFunc = NULL;
	__DTCRecord.updateFunc = __DTCRecordUpdate;

	NEWLABEL(__DTCStatus, "DTCStatus:");
	__DTCStatus.requestFunc = NULL;
	__DTCStatus.updateFunc = __DTCStatusUpdate;

	NEWLIST(__DTCSnapshotRecordNumber, "DTCSnapshotRecordNumber[]", GTK_SELECTION_SINGLE);
	__DTCSnapshotRecordNumber.requestFunc = NULL;
	__DTCSnapshotRecordNumber.updateFunc = __DTCSnapshotRecordNumberUpdate;

	NEWLABEL(__DTCDataIdentifier, "DTCDataIdentifier:");
	__DTCDataIdentifier.requestFunc = NULL;
	__DTCDataIdentifier.updateFunc = __DTCDataIdentifierUpdate;

	NEWLIST(__DTCSnapshotRecord, "DTCSnapshotRecord[]", GTK_SELECTION_NONE);
	__DTCSnapshotRecord.requestFunc = NULL;
	__DTCSnapshotRecord.updateFunc = __DTCSnapshotRecordUpdate;

	NEWLIST(__DTCSnapshotRecordNumberOfIdentifiers, "DTCSnapshotRecordNumberOfIdentifiers[]", GTK_SELECTION_NONE);
	__DTCSnapshotRecordNumberOfIdentifiers.requestFunc = NULL;
	__DTCSnapshotRecordNumberOfIdentifiers.updateFunc = __DTCSnapshotRecordNumberOfIdentifiersUpdate;

	NEWLIST(__DTCStoredDataRecordNumber, "DTCStoredDataRecordNumber[]", GTK_SELECTION_SINGLE);
	__DTCStoredDataRecordNumber.requestFunc = NULL;
	__DTCStoredDataRecordNumber.updateFunc = __DTCStoredDataRecordNumberUpdate;
	
	NEWLIST(__DTCStoredDataRecordNumberOfIdentifiers, "DTCStoredDataRecordNumberOfIdentifiers[]", GTK_SELECTION_NONE);
	__DTCStoredDataRecordNumberOfIdentifiers.requestFunc = NULL;
	__DTCStoredDataRecordNumberOfIdentifiers.updateFunc = __DTCStoredDataRecordNumberOfIdentifiersUpdate;

	NEWLIST(__DTCExtDataRecordNumber, "DTCExtDataRecordNumber[]", GTK_SELECTION_SINGLE);
	__DTCExtDataRecordNumber.requestFunc = NULL;
	__DTCExtDataRecordNumber.updateFunc = __DTCExtDataRecordNumberUpdate;

	NEWLIST(__DTCExtDataRecord, "DTCExtDataRecord[]", GTK_SELECTION_NONE);
	__DTCExtDataRecord.requestFunc = NULL;
	__DTCExtDataRecord.updateFunc = __DTCExtDataRecordUpdate;

	NEWLIST(__DTCFaultDetectionCounter, "DTCFaultDetectionCounter[]", GTK_SELECTION_NONE);
	__DTCFaultDetectionCounter.requestFunc = NULL;
	__DTCFaultDetectionCounter.updateFunc = __DTCFaultDetectionCounterUpdate;

	NEWLABEL(__UserDefDTCSnapshotRecordNumber, "UserDefDTCSnapshotRecordNumber:");
	__UserDefDTCSnapshotRecordNumber.requestFunc = NULL;
	__UserDefDTCSnapshotRecordNumber.updateFunc = __UserDefDTCSnapshotRecordNumberUpdate;

	NEWLABEL(__DTCReadinessGroupIdentifier, "DTCReadinessGroupIdentifier:");
	__DTCReadinessGroupIdentifier.requestFunc = NULL;
	__DTCReadinessGroupIdentifier.updateFunc = __DTCReadinessGroupIdentifierUpdate;

	RDTCIResponses[0].elements = (PanelHandle*)malloc(4 * sizeof(PanelHandle));
	RDTCIResponses[0].elements[0].handle = &__DTCStatusAvailabilityMask;
	RDTCIResponses[0].elements[0].xPos = 0, RDTCIResponses[0].elements[0].yPos = 0;
	RDTCIResponses[0].elements[1].handle = &__DTCFormatIdentifier;
	RDTCIResponses[0].elements[1].xPos = 0, RDTCIResponses[0].elements[1].yPos = 1;
	RDTCIResponses[0].elements[2].handle = &__DTCCount;
	RDTCIResponses[0].elements[2].xPos = 0, RDTCIResponses[0].elements[2].yPos = 2;
	RDTCIResponses[0].elements[3].handle = NULL;

	RDTCIResponses[1].elements = (PanelHandle*)malloc(4 * sizeof(PanelHandle));
	RDTCIResponses[1].elements[0].handle = &__DTCStatusAvailabilityMask;
	RDTCIResponses[1].elements[0].xPos = 0, RDTCIResponses[1].elements[0].yPos = 0;
	RDTCIResponses[1].elements[1].handle = &__DTCRecord;
	RDTCIResponses[1].elements[1].xPos = 0, RDTCIResponses[1].elements[1].yPos = 2;
	RDTCIResponses[1].elements[2].handle = &__DTCStatus;
	RDTCIResponses[1].elements[2].xPos = 0, RDTCIResponses[1].elements[2].yPos = 1;
	RDTCIResponses[1].elements[3].handle = NULL;

	RDTCIResponses[2].elements = (PanelHandle*)malloc(3 * sizeof(PanelHandle));
	RDTCIResponses[2].elements[0].handle = &__DTCRecord;
	RDTCIResponses[2].elements[0].xPos = 0, RDTCIResponses[2].elements[0].yPos = 0;
	RDTCIResponses[2].elements[1].handle = &__DTCSnapshotRecordNumber;
	RDTCIResponses[2].elements[1].xPos = 1, RDTCIResponses[2].elements[1].yPos = 0;
	RDTCIResponses[2].elements[2].handle = NULL;

	RDTCIResponses[3].elements = (PanelHandle*)malloc(7 * sizeof(PanelHandle));
	RDTCIResponses[3].elements[0].handle = &__DTCRecord;
	RDTCIResponses[3].elements[0].xPos = 0, RDTCIResponses[3].elements[0].yPos = 2;
	RDTCIResponses[3].elements[1].handle = &__DTCSnapshotRecordNumber;
	RDTCIResponses[3].elements[1].xPos = 1, RDTCIResponses[3].elements[1].yPos = 2;
	RDTCIResponses[3].elements[2].handle = &__DTCStatus;
	RDTCIResponses[3].elements[2].xPos = 0, RDTCIResponses[3].elements[2].yPos = 0;
	RDTCIResponses[3].elements[3].handle = &__DTCSnapshotRecordNumberOfIdentifiers;
	RDTCIResponses[3].elements[3].xPos = 0, RDTCIResponses[3].elements[3].yPos = 3;
	RDTCIResponses[3].elements[4].handle = &__DTCDataIdentifier;
	RDTCIResponses[3].elements[4].xPos = 0, RDTCIResponses[3].elements[4].yPos = 1;
	RDTCIResponses[3].elements[5].handle = &__DTCSnapshotRecord;
	RDTCIResponses[3].elements[5].xPos = 1, RDTCIResponses[3].elements[5].yPos = 3;
	RDTCIResponses[3].elements[6].handle = NULL;

	RDTCIResponses[4].elements = (PanelHandle*)malloc(7 * sizeof(PanelHandle));
	RDTCIResponses[4].elements[0].handle = &__DTCRecord;
	RDTCIResponses[4].elements[0].xPos = 0, RDTCIResponses[4].elements[0].yPos = 2;
	RDTCIResponses[4].elements[1].handle = &__DTCStoredDataRecordNumber;
	RDTCIResponses[4].elements[1].xPos = 1, RDTCIResponses[4].elements[1].yPos = 2;
	RDTCIResponses[4].elements[2].handle = &__DTCStatus;
	RDTCIResponses[4].elements[2].xPos = 0, RDTCIResponses[4].elements[2].yPos = 1;
	RDTCIResponses[4].elements[3].handle = &__DTCStoredDataRecordNumberOfIdentifiers;
	RDTCIResponses[4].elements[3].xPos = 0, RDTCIResponses[4].elements[3].yPos = 3;
	RDTCIResponses[4].elements[4].handle = &__DTCDataIdentifier;
	RDTCIResponses[4].elements[4].xPos = 0, RDTCIResponses[4].elements[4].yPos = 0;
	RDTCIResponses[4].elements[5].handle = &__DTCSnapshotRecord;
	RDTCIResponses[4].elements[5].xPos = 1, RDTCIResponses[4].elements[5].yPos = 3;
	RDTCIResponses[4].elements[6].handle = NULL;

	RDTCIResponses[5].elements = (PanelHandle*)malloc(5 * sizeof(PanelHandle));
	RDTCIResponses[5].elements[0].handle = &__DTCRecord;
	RDTCIResponses[5].elements[0].xPos = 0, RDTCIResponses[5].elements[0].yPos = 0;
	RDTCIResponses[5].elements[1].handle = &__DTCExtDataRecordNumber;
	RDTCIResponses[5].elements[1].xPos = 0, RDTCIResponses[5].elements[1].yPos = 1;
	RDTCIResponses[5].elements[2].handle = &__DTCStatus;
	RDTCIResponses[5].elements[2].xPos = 1, RDTCIResponses[5].elements[2].yPos = 0;
	RDTCIResponses[5].elements[3].handle = &__DTCExtDataRecord;
	RDTCIResponses[5].elements[3].xPos = 1, RDTCIResponses[5].elements[3].yPos = 1;
	RDTCIResponses[5].elements[4].handle = NULL;

	RDTCIResponses[6].elements = (PanelHandle*)malloc(6 * sizeof(PanelHandle));
	RDTCIResponses[6].elements[0].handle = &__DTCRecord;
	RDTCIResponses[6].elements[0].xPos = 0, RDTCIResponses[6].elements[0].yPos = 4;
	RDTCIResponses[6].elements[1].handle = &__DTCStatus;
	RDTCIResponses[6].elements[1].xPos = 0, RDTCIResponses[6].elements[1].yPos = 3;
	RDTCIResponses[6].elements[2].handle = &__DTCFunctionalUnit;
	RDTCIResponses[6].elements[2].xPos = 0, RDTCIResponses[6].elements[2].yPos = 2;
	RDTCIResponses[6].elements[3].handle = &__DTCStatusAvailabilityMask;
	RDTCIResponses[6].elements[3].xPos = 0, RDTCIResponses[6].elements[3].yPos = 0;
	RDTCIResponses[6].elements[4].handle = &__DTCSeverity;
	RDTCIResponses[6].elements[4].xPos = 0, RDTCIResponses[6].elements[4].yPos = 1;
	RDTCIResponses[6].elements[5].handle = NULL;
	
	RDTCIResponses[7].elements = (PanelHandle*)malloc(3 * sizeof(PanelHandle));
	RDTCIResponses[7].elements[0].handle = &__DTCRecord;
	RDTCIResponses[7].elements[0].xPos = 0, RDTCIResponses[7].elements[0].yPos = 0;
	RDTCIResponses[7].elements[1].handle = &__DTCFaultDetectionCounter;
	RDTCIResponses[7].elements[1].xPos = 1, RDTCIResponses[7].elements[1].yPos = 0;
	RDTCIResponses[7].elements[2].handle = NULL;

	RDTCIResponses[8].elements = (PanelHandle*)malloc(5 * sizeof(PanelHandle));
	RDTCIResponses[8].elements[0].handle = &__DTCRecord;
	RDTCIResponses[8].elements[0].xPos = 0, RDTCIResponses[8].elements[0].yPos = 0;
	RDTCIResponses[8].elements[1].handle = &__DTCStatus;
	RDTCIResponses[8].elements[1].xPos = 1, RDTCIResponses[8].elements[1].yPos = 0;
	RDTCIResponses[8].elements[2].handle = &__DTCExtDataRecordNumber;
	RDTCIResponses[8].elements[2].xPos = 0, RDTCIResponses[8].elements[2].yPos = 1;
	RDTCIResponses[8].elements[3].handle = &__DTCExtDataRecord;
	RDTCIResponses[8].elements[3].xPos = 1, RDTCIResponses[8].elements[3].yPos = 1;
	RDTCIResponses[8].elements[4].handle = NULL;
	
	RDTCIResponses[9].elements = (PanelHandle*)malloc(5 * sizeof(PanelHandle));
	RDTCIResponses[9].elements[0].handle = &__DTCRecord;
	RDTCIResponses[9].elements[0].xPos = 0, RDTCIResponses[9].elements[0].yPos = 3;
	RDTCIResponses[9].elements[1].handle = &__DTCStatus;
	RDTCIResponses[9].elements[1].xPos = 0, RDTCIResponses[9].elements[1].yPos = 2;
	RDTCIResponses[9].elements[2].handle = &__MemorySelection;
	RDTCIResponses[9].elements[2].xPos = 0, RDTCIResponses[9].elements[2].yPos = 0;
	RDTCIResponses[9].elements[3].handle = &__DTCStatusAvailabilityMask;
	RDTCIResponses[9].elements[3].xPos = 0, RDTCIResponses[9].elements[3].yPos = 1;
	RDTCIResponses[9].elements[4].handle = NULL;
	
	RDTCIResponses[10].elements = (PanelHandle*)malloc(6 * sizeof(PanelHandle));
	RDTCIResponses[10].elements[0].handle = &__DTCRecord;
	RDTCIResponses[10].elements[0].xPos = 0, RDTCIResponses[10].elements[0].yPos = 3;
	RDTCIResponses[10].elements[1].handle = &__DTCStatus;
	RDTCIResponses[10].elements[1].xPos = 0, RDTCIResponses[10].elements[1].yPos = 2;
	RDTCIResponses[10].elements[2].handle = &__MemorySelection;
	RDTCIResponses[10].elements[2].xPos = 0, RDTCIResponses[10].elements[2].yPos = 0;
	RDTCIResponses[10].elements[3].handle = &__UserDefDTCSnapshotRecordNumber;
	RDTCIResponses[10].elements[3].xPos = 0, RDTCIResponses[10].elements[3].yPos = 1;
	RDTCIResponses[10].elements[4].handle = &__DTCSnapshotRecordNumberOfIdentifiers;
	RDTCIResponses[10].elements[4].xPos = 1, RDTCIResponses[10].elements[4].yPos = 3;
	RDTCIResponses[10].elements[5].handle = NULL;
	
	RDTCIResponses[11].elements = (PanelHandle*)malloc(6 * sizeof(PanelHandle));
	RDTCIResponses[11].elements[0].handle = &__DTCRecord;
	RDTCIResponses[11].elements[0].xPos = 0, RDTCIResponses[11].elements[0].yPos = 2;
	RDTCIResponses[11].elements[1].handle = &__DTCStatus;
	RDTCIResponses[11].elements[1].xPos = 1, RDTCIResponses[11].elements[1].yPos = 2;
	RDTCIResponses[11].elements[2].handle = &__DTCExtDataRecordNumber;
	RDTCIResponses[11].elements[2].xPos = 0, RDTCIResponses[11].elements[2].yPos = 1;
	RDTCIResponses[11].elements[3].handle = &__MemorySelection;
	RDTCIResponses[11].elements[3].xPos = 0, RDTCIResponses[11].elements[3].yPos = 0;
	RDTCIResponses[11].elements[4].handle = &__DTCExtDataRecord;
	RDTCIResponses[11].elements[4].xPos = 2, RDTCIResponses[11].elements[4].yPos = 2;
	RDTCIResponses[11].elements[5].handle = NULL;
	
	RDTCIResponses[12].elements = (PanelHandle*)malloc(5 * sizeof(PanelHandle));
	RDTCIResponses[12].elements[0].handle = &__DTCRecord;
	RDTCIResponses[12].elements[0].xPos = 1, RDTCIResponses[12].elements[0].yPos = 1;
	RDTCIResponses[12].elements[1].handle = &__DTCStatus;
	RDTCIResponses[12].elements[1].xPos = 2, RDTCIResponses[12].elements[1].yPos = 1;
	RDTCIResponses[12].elements[2].handle = &__DTCExtDataRecordNumber;
	RDTCIResponses[12].elements[2].xPos = 0, RDTCIResponses[12].elements[2].yPos = 1;
	RDTCIResponses[12].elements[3].handle = &__DTCStatusAvailabilityMask;
	RDTCIResponses[12].elements[3].xPos = 0, RDTCIResponses[12].elements[3].yPos = 0;
	RDTCIResponses[12].elements[4].handle = NULL;
	
	RDTCIResponses[13].elements = (PanelHandle*)malloc(8 * sizeof(PanelHandle));
	RDTCIResponses[13].elements[0].handle = &__DTCRecord;
	RDTCIResponses[13].elements[0].xPos = 1, RDTCIResponses[13].elements[0].yPos = 4;
	RDTCIResponses[13].elements[1].handle = &__DTCStatus;
	RDTCIResponses[13].elements[1].xPos = 2, RDTCIResponses[13].elements[1].yPos = 4;
	RDTCIResponses[13].elements[2].handle = &__DTCStatusAvailabilityMask;
	RDTCIResponses[13].elements[2].xPos = 0, RDTCIResponses[13].elements[2].yPos = 1;
	RDTCIResponses[13].elements[3].handle = &__FunctionalGroupIdentifier;
	RDTCIResponses[13].elements[3].xPos = 0, RDTCIResponses[13].elements[3].yPos = 0;
	RDTCIResponses[13].elements[4].handle = &__DTCSeverity;
	RDTCIResponses[13].elements[4].xPos = 0, RDTCIResponses[13].elements[4].yPos = 4;
	RDTCIResponses[13].elements[5].handle = &__DTCSeverityAvailabilityMask;
	RDTCIResponses[13].elements[5].xPos = 0, RDTCIResponses[13].elements[5].yPos = 2;
	RDTCIResponses[13].elements[6].handle = &__DTCFormatIdentifier;
	RDTCIResponses[13].elements[6].xPos = 0, RDTCIResponses[13].elements[6].yPos = 3;
	RDTCIResponses[13].elements[7].handle = NULL;

	RDTCIResponses[14].elements = (PanelHandle*)malloc(6 * sizeof(PanelHandle));
	RDTCIResponses[14].elements[0].handle = &__DTCRecord;
	RDTCIResponses[14].elements[0].xPos = 0, RDTCIResponses[14].elements[0].yPos = 3;
	RDTCIResponses[14].elements[1].handle = &__DTCStatus;
	RDTCIResponses[14].elements[1].xPos = 1, RDTCIResponses[14].elements[1].yPos = 3;
	RDTCIResponses[14].elements[2].handle = &__FunctionalGroupIdentifier;
	RDTCIResponses[14].elements[2].xPos = 0, RDTCIResponses[14].elements[2].yPos = 0;
	RDTCIResponses[14].elements[3].handle = &__DTCStatusAvailabilityMask;
	RDTCIResponses[14].elements[3].xPos = 0, RDTCIResponses[14].elements[3].yPos = 1;
	RDTCIResponses[14].elements[4].handle = &__DTCFormatIdentifier;
	RDTCIResponses[14].elements[4].xPos = 0, RDTCIResponses[14].elements[4].yPos = 2;
	RDTCIResponses[14].elements[5].handle = NULL;

	RDTCIResponses[15].elements = (PanelHandle*)malloc(7 * sizeof(PanelHandle));
	RDTCIResponses[15].elements[0].handle = &__DTCRecord;
	RDTCIResponses[15].elements[0].xPos = 0, RDTCIResponses[15].elements[0].yPos = 4;
	RDTCIResponses[15].elements[1].handle = &__DTCStatus;
	RDTCIResponses[15].elements[1].xPos = 1, RDTCIResponses[15].elements[1].yPos = 4;
	RDTCIResponses[15].elements[2].handle = &__FunctionalGroupIdentifier;
	RDTCIResponses[15].elements[2].xPos = 0, RDTCIResponses[15].elements[2].yPos = 0;
	RDTCIResponses[15].elements[3].handle = &__DTCStatusAvailabilityMask;
	RDTCIResponses[15].elements[3].xPos = 0, RDTCIResponses[15].elements[3].yPos = 1;
	RDTCIResponses[15].elements[4].handle = &__DTCFormatIdentifier;
	RDTCIResponses[15].elements[4].xPos = 0, RDTCIResponses[15].elements[4].yPos = 2;
	RDTCIResponses[15].elements[5].handle = &__DTCReadinessGroupIdentifier;
	RDTCIResponses[15].elements[5].xPos = 0, RDTCIResponses[15].elements[5].yPos = 3;
	RDTCIResponses[15].elements[6].handle = NULL;
}

int CreateRDTCIPanel() {
	for (unsigned int i = 0; RDTCIPanel[i].requestPanel && RDTCIPanel[i].responsePanel; ++i) {
		if (!RDTCIPanel[i].requestPanel->panel) {
			RDTCIPanel[i].requestPanel->panel = gtk_grid_new();
			RDTCIPanel[i].requestPanel->frame = gtk_frame_new("Request Panel");
			gtk_frame_set_child(GTK_FRAME(RDTCIPanel[i].requestPanel->frame), GTK_WIDGET(RDTCIPanel[i].requestPanel->panel));
		}
		if (!RDTCIPanel[i].responsePanel->panel) {
			RDTCIPanel[i].responsePanel->panel = gtk_grid_new();
			RDTCIPanel[i].responsePanel->frame = gtk_frame_new("Response Panel");
			gtk_frame_set_child(GTK_FRAME(RDTCIPanel[i].responsePanel->frame), GTK_WIDGET(RDTCIPanel[i].responsePanel->panel));
		}
		CreateNewPanel(&RDTCIPanel[i]);
	}
	CreateRDCTIRequests();
	CreateRDTCIResponse();	
	
}
