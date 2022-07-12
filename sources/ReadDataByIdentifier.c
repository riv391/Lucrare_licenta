#include "../headers/ReadDataByIdentifier.h"
#include "../UDS_Services/headers/ReadDataByIdentifierService.h"

PanelElement _ResultPanel;

PanelElement _RequestPanel;

ConfigPanel RDBIPanel;

ServiceParam _RDBI = {
	"ReadDataByIdentifier",
	RDBI_RequestService,
	RDBI_ReceiveResponse,
	{{"None", 0x00, &RDBIPanel},
		{NULL, -1, NULL}}
};

RDBIIdentifiers standardIds[27] = {
		{"Boot software identification", &Identifiers[0]},
		{"Application software identification", &Identifiers[1]},
	 	{"Application data identification", &Identifiers[2]},
	 	{"Boot software fingerprint", &Identifiers[3]},
	 	{"Application software fingerprint", &Identifiers[4]},
	 	{"Application data fingerprint", &Identifiers[5]},
	 	{"Active diagnostic session", &Identifiers[6]},
	 	{"Manufacturer spare part number", &Identifiers[7]},
	 	{"Manufacturer ECU software number", &Identifiers[8]},
	 	{"Manufacturer ECU software version", &Identifiers[9]},
	 	{"Identifier of system supplier", &Identifiers[10]},
	 	{"ECU manufacturing date", &Identifiers[11]},
	 	{"ECU serial number", &Identifiers[12]},
	 	{"Supported functional units", &Identifiers[13]},
	 	{"Manufacturer kit assembly part number", &Identifiers[14]},
	 	{"Vehicle identification number (VIN)", &Identifiers[15]},
	 	{"System supplier ECU hardware number", &Identifiers[16]},
	 	{"System supplier ECU hardware version number", &Identifiers[17]},
	 	{"System supplier ECU software number", &Identifiers[18]},
	 	{"System supplier ECU software version number", &Identifiers[19]},
	 	{"Exhaust regulation/type approval number", &Identifiers[20]},
	 	{"System name / engine type", &Identifiers[21]},
	 	{"Repair shop code / tester serial number", &Identifiers[22]},
	 	{"Programming date", &Identifiers[23]},
	 	{"ECU installation date", &Identifiers[24]},
	 	{"ODX file", &Identifiers[25]},
		{NULL, NULL}
};

int _RequestPanelRequest() {
	selectedIdentifier = standardIds[gtk_combo_box_get_active(GTK_COMBO_BOX(_RequestPanel.elements[1].widget))].identifier;
	return 0;
}

int _ResultPanelUpdate() {
	DataIdentifier* tempID = standardIds[gtk_combo_box_get_active(GTK_COMBO_BOX(_RequestPanel.elements[1].widget))].identifier;
	if (tempID->dataRecord.data) {
		memcpy(buff, tempID->dataRecord.data, tempID->dataRecord.Length);
		buff[tempID->dataRecord.Length] = '\0';
		//for (unsigned int i = 0; i < tempID->dataRecord.Length; ++i)
		//	sprintf(buff + i, "%x", tempID->dataRecord.data[i]);
		gtk_label_set_text(GTK_LABEL(_ResultPanel.elements[1].widget), buff);
	} else 
		gtk_label_set_text(GTK_LABEL(_ResultPanel.elements[1].widget), "Not set");

	return 0;
}

void ChangeIdentifier(GtkWidget* widget, void* data) {
	_ResultPanelUpdate();
}

int CreateRDBIPanel() {
	CreateNewPanel(&RDBIPanel);

	_RequestPanel.panel = gtk_grid_new();
	_RequestPanel.elements = (SimpleElement*)malloc(4 * sizeof(SimpleElement));
	_RequestPanel.elements[0].widget = gtk_label_new("Select ID:");
	_RequestPanel.elements[0].xPos = 0, _RequestPanel.elements[0].yPos = 0;
	_RequestPanel.elements[1].widget = gtk_combo_box_text_new();
	_RequestPanel.elements[1].xPos = 1, _RequestPanel.elements[1].yPos = 0;
	_RequestPanel.elements[2].widget = gtk_label_new(NULL);
	_RequestPanel.elements[2].xPos = 2, _RequestPanel.elements[2].yPos = 0;
	_RequestPanel.elements[3].widget = NULL;
	_RequestPanel.requestFunc = _RequestPanelRequest;
	_RequestPanel.updateFunc = NULL;
	CREATEELEMENT(_RequestPanel);
	for (unsigned int i = 0; standardIds[i].name; ++i) {
		sprintf(buff, "%s (0x%X)", standardIds[i].name, (standardIds[i].identifier->DIDBH << 8) + standardIds[i].identifier->DIDBL);
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(_RequestPanel.elements[1].widget), NULL, buff);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(_RequestPanel.elements[1].widget), 0);
	g_signal_connect(G_OBJECT(_RequestPanel.elements[1].widget), "changed", G_CALLBACK(ChangeIdentifier), NULL);
	g_signal_connect(G_OBJECT(_RequestPanel.elements[1].widget), "show", G_CALLBACK(ChangeIdentifier), NULL);

	_ResultPanel.panel = gtk_grid_new();
	_ResultPanel.elements = (SimpleElement*)malloc(3 * sizeof(SimpleElement));
	_ResultPanel.elements[0].widget = gtk_label_new("Result:");
	_ResultPanel.elements[0].xPos = 0, _ResultPanel.elements[0].yPos = 0;
	_ResultPanel.elements[1].widget = gtk_label_new(NULL);
	_ResultPanel.elements[1].xPos = 1, _ResultPanel.elements[1].yPos = 0;
	_ResultPanel.elements[2].widget = NULL;
	_ResultPanel.requestFunc = NULL;
	_ResultPanel.updateFunc = _ResultPanelUpdate;
	CREATEELEMENT(_ResultPanel);

	RDBIPanel.requestPanel->elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	RDBIPanel.requestPanel->elements[0].handle = &_RequestPanel;
	RDBIPanel.requestPanel->elements[0].xPos = 0, RDBIPanel.requestPanel->elements[0].yPos = 0;
	RDBIPanel.requestPanel->elements[1].handle = NULL;

	RDBIPanel.responsePanel->elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	RDBIPanel.responsePanel->elements[0].handle = &_ResultPanel;
	RDBIPanel.responsePanel->elements[0].xPos = 0, RDBIPanel.responsePanel->elements[0].yPos = 0;
	RDBIPanel.responsePanel->elements[1].handle = NULL;

	return 0;
}
