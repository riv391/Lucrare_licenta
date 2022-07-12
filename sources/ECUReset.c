#include <stdlib.h>
#include "../headers/ECUReset.h"
#include "../UDS_Services/headers/ECUResetService.h"

PanelElement _PowerPanel;

ConfigPanel ECURPanel = {NULL, NULL, NULL};

ServiceParam _ECUR = {
	"ECUReset",
	ECUR_RequestService,
	ECUR_ReceiveResponse,
	{{"hardReset", HR, &ECURPanel},
		{"keyOffOnReset", KOFFONR, &ECURPanel},
		{"softReset", SR, &ECURPanel},
		{"enableRapidPowerShutDown", ERPSD, &ECURPanel},
		{"disableRapidPowerShutDown", DRPSD, &ECURPanel},
		{NULL, -1, NULL}}
};

int updateECURState() {
	if (powerDownTime) {
		if (*powerDownTime < 0xff)
			sprintf(buff, "%u s", *powerDownTime);
		else
			sprintf(buff, "no time available");
	} else
		sprintf(buff, "Not set");
	gtk_label_set_text(GTK_LABEL(ECURPanel.responsePanel->elements[0].handle->elements[1].widget), buff);
}

int CreateECURPanel() {
	CreateNewPanel(&ECURPanel);

	_PowerPanel.panel = gtk_grid_new();
	_PowerPanel.elements = (SimpleElement*)malloc(3 * sizeof(SimpleElement));
	_PowerPanel.elements[0].widget = gtk_label_new("PowerDownTime:");
	_PowerPanel.elements[0].xPos = 0, _PowerPanel.elements[0].yPos = 0;
	_PowerPanel.elements[1].widget = gtk_label_new(NULL);
	_PowerPanel.elements[1].xPos = 1, _PowerPanel.elements[1].yPos = 0;
	_PowerPanel.elements[2].widget = NULL;
	_PowerPanel.requestFunc = NULL;
	_PowerPanel.updateFunc = updateECURState;
	CREATEELEMENT(_PowerPanel);

	ECURPanel.requestPanel->elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	ECURPanel.requestPanel->elements[0].handle = &_NoParameters;
	ECURPanel.requestPanel->elements[0].xPos = 0, ECURPanel.requestPanel->elements[0].yPos = 0;
	ECURPanel.requestPanel->elements[1].handle = NULL;

	ECURPanel.responsePanel->elements = (PanelHandle*)malloc(2 * sizeof(PanelHandle));
	ECURPanel.responsePanel->elements[0].handle = &_PowerPanel;
	ECURPanel.responsePanel->elements[0].xPos = 0, ECURPanel.responsePanel->elements[0].yPos = 0;
	ECURPanel.responsePanel->elements[1].handle = NULL;
	
	//for (unsigned int i = 0; ECURPanel.requestPanel->elements[i].handle; ++i) {
	//	//if (CHECKWIDGET(ECURPanel.requestPanel->elements[i].widget, GTK_TYPE_LABEL))
	//	//	printf("xPos=%u : yPos%u\n", ECURPanel.requestPanel->elements[i].xPos, ECURPanel.requestPanel->elements[i].yPos);
	//	//gtk_grid_attach(GTK_GRID(ECURPanel.requestPanel->panel), ECURPanel.requestPanel->elements[i].handle, ECURPanel.requestPanel->elements[i].xPos, ECURPanel.requestPanel->elements[i].yPos, 1, 1);
	//	for (unsigned int j = 0; ECURPanel.requestPanel->elements[i].handle->elements[j].widget; ++j)
	//		gtk_grid_attach(GTK_GRID(ECURPanel.requestPanel->elements[i].handle->panel), ECURPanel.requestPanel->elements[i].handle->elements[j].widget, ECURPanel.requestPanel->elements[i].handle->elements[j].xPos, ECURPanel.requestPanel->elements[i].handle->elements[j].yPos, 1, 1);
	//}
	//for (unsigned int i = 0; ECURPanel.responsePanel->elements[i].handle; ++i) {
	//	//if (CHECKWIDGET(ECURPanel.responsePanel->elements[i].widget, GTK_TYPE_LABEL))
	//	//	printf("xPos=%u : yPos%u\n", ECURPanel.responsePanel->elements[i].xPos, ECURPanel.responsePanel->elements[i].yPos);
	//	//gtk_grid_attach(GTK_GRID(ECURPanel.responsePanel->panel), ECURPanel.responsePanel->elements[i].handle, ECURPanel.responsePanel->elements[i].xPos, ECURPanel.responsePanel->elements[i].yPos, 1, 1);
	//	for (unsigned int j = 0; ECURPanel.responsePanel->elements[i].handle->elements[j].widget; ++j)
	//		gtk_grid_attach(GTK_GRID(ECURPanel.responsePanel->elements[i].handle->panel), ECURPanel.responsePanel->elements[i].handle->elements[j].widget, ECURPanel.responsePanel->elements[i].handle->elements[j].xPos, ECURPanel.responsePanel->elements[i].handle->elements[j].yPos, 1, 1);
	//}
}


