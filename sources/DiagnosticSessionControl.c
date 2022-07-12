#include <stdlib.h>
#include "../headers/DiagnosticSessionControl.h"
#include "../UDS_Services/headers/DiagnosticSessionControlService.h"

PanelElement _NoParameters;
PanelElement _ServerInfo;

ConfigPanel DSCPanel = {NULL, NULL, NULL};

ServiceParam _DSC = {
	"DiagnosticSessionControl",
	DSC_RequestService,
	DSC_ReceiveResponse,
	{{"defaultSession", DS, &DSCPanel},
		{"ProgrammingSession", PRGS, &DSCPanel},
		{"extendedDiagnosticSession", EXTDS, &DSCPanel},
		{"safetySystemDiagnosticSession", SSDS, &DSCPanel},
		{NULL, -1, NULL}}
};

int updateDSCState() {
	if (P2Server_max)
		sprintf(buff, "%u ms", *P2Server_max);
	else
		sprintf(buff, "Not set");
	gtk_label_set_text(GTK_LABEL(DSCPanel.responsePanel->elements[0].handle->elements[2].widget), buff);
	if (P2_aux_Server_max)
		sprintf(buff, "%u ms", *P2_aux_Server_max);
	else
		sprintf(buff, "Not set");
	gtk_label_set_text(GTK_LABEL(DSCPanel.responsePanel->elements[0].handle->elements[3].widget), buff);
}

int CreateDSCPanel() {
	CreateNewPanel(&DSCPanel);

	_NoParameters.panel = gtk_grid_new();
	_NoParameters.elements = (SimpleElement*)malloc(2 * sizeof(SimpleElement));
	_NoParameters.elements[0].widget = gtk_label_new("No parameters are required for this subfunction!");
	_NoParameters.elements[0].xPos = 0, _NoParameters.elements[0].yPos = 0;
	_NoParameters.elements[1].widget = NULL;
	_NoParameters.requestFunc = NULL;
	_NoParameters.updateFunc = NULL;
	CREATEELEMENT(_NoParameters);

	_ServerInfo.panel = gtk_grid_new();
	_ServerInfo.elements = (SimpleElement*)malloc(5 * sizeof(SimpleElement));
	_ServerInfo.elements[0].widget = gtk_label_new("P2Server_max:");
	_ServerInfo.elements[0].xPos = 0, _ServerInfo.elements[0].yPos = 0;
	_ServerInfo.elements[1].widget = gtk_label_new("P2*Server_max:");
	_ServerInfo.elements[1].xPos = 0, _ServerInfo.elements[1].yPos = 1;
	_ServerInfo.elements[2].widget = gtk_label_new(NULL);
	_ServerInfo.elements[2].xPos = 1, _ServerInfo.elements[2].yPos = 0;
       	_ServerInfo.elements[3].widget = gtk_label_new(NULL);
	_ServerInfo.elements[3].xPos = 1, _ServerInfo.elements[3].yPos = 1;
	_ServerInfo.elements[4].widget = NULL;
	_ServerInfo.requestFunc = NULL;
	_ServerInfo.updateFunc = updateDSCState;
	CREATEELEMENT(_ServerInfo);

	DSCPanel.requestPanel->elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	DSCPanel.requestPanel->elements[0].handle = &_NoParameters;
	DSCPanel.requestPanel->elements[0].xPos = 0, DSCPanel.requestPanel->elements[0].yPos = 0;
	DSCPanel.requestPanel->elements[1].handle = NULL;

	DSCPanel.responsePanel->elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	DSCPanel.responsePanel->elements[0].handle = &_ServerInfo;
	DSCPanel.responsePanel->elements[0].xPos = 0, DSCPanel.responsePanel->elements[0].yPos = 0;
	DSCPanel.responsePanel->elements[1].handle = NULL;

	//for (unsigned int i = 0; DSCPanel.requestPanel->elements[i].handle; ++i) {
	//	//if (CHECKWIDGET(DSCPanel.requestPanel->elements[i].widget, GTK_TYPE_LABEL))
	//	//	printf("xPos=%u : yPos%u\n", DSCPanel.requestPanel->elements[i].xPos, DSCPanel.requestPanel->elements[i].yPos);
	//	//gtk_grid_attach(GTK_GRID(DSCPanel.requestPanel->panel), DSCPanel.requestPanel->elements[i].handle, DSCPanel.requestPanel->elements[i].xPos, DSCPanel.requestPanel->elements[i].yPos, 1, 1);
	//	for (unsigned int j = 0; DSCPanel.requestPanel->elements[i].handle->elements[j].widget; ++j)
	//		gtk_grid_attach(GTK_GRID(DSCPanel.requestPanel->elements[i].handle->panel), DSCPanel.requestPanel->elements[i].handle->elements[j].widget, DSCPanel.requestPanel->elements[i].handle->elements[j].xPos, DSCPanel.requestPanel->elements[i].handle->elements[j].yPos, 1, 1);
	//}
	//for (unsigned int i = 0; DSCPanel.responsePanel->elements[i].handle; ++i) {
	//	//if (CHECKWIDGET(DSCPanel.responsePanel->elements[i].widget, GTK_TYPE_LABEL))
	//	//	printf("xPos=%u : yPos%u\n", DSCPanel.responsePanel->elements[i].xPos, DSCPanel.responsePanel->elements[i].yPos);
	//	//gtk_grid_attach(GTK_GRID(DSCPanel.responsePanel->panel), DSCPanel.responsePanel->elements[i].handle, DSCPanel.responsePanel->elements[i].xPos, DSCPanel.responsePanel->elements[i].yPos, 1, 1);
	//	for (unsigned int j = 0; DSCPanel.responsePanel->elements[i].handle->elements[j].widget; ++j)
	//		gtk_grid_attach(GTK_GRID(DSCPanel.responsePanel->elements[i].handle->panel), DSCPanel.responsePanel->elements[i].handle->elements[j].widget, DSCPanel.responsePanel->elements[i].handle->elements[j].xPos, DSCPanel.responsePanel->elements[i].handle->elements[j].yPos, 1, 1);
	//}
}


