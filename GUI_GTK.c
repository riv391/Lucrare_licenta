#include <stdlib.h>
#include <pthread.h>
#include "headers/header.h"
#include "headers/bluetooth.h"
#include "headers/USB.h"
#include "headers/ECUReset.h"
#include "headers/DiagnosticSessionControl.h"
#include "headers/ReadDTCInformation.h"
#include "headers/ReadDataByIdentifier.h"
#include "headers/NegativeResponse.h"

#define SENDDATA {\
	if (gtk_switch_get_state(GTK_SWITCH(connectionTypeSwitch)))\
		sendDataToSocket();\
	else\
		sendDataToPort();\
}

#define RECVDATA {\
	if (gtk_switch_get_state(GTK_SWITCH(connectionTypeSwitch)))\
		recvDataFromSocket();\
	else\
		recvDataFromPort();\
}

Neg_Resp neg_resp[] = {
	{"Subfunction Not Supported", SFNS},
	{"Incorrect Message Length Or Invalid Format", IMLOIF},
	{"Condition Not Correct", CNC},
	{"Request Sequence Error", RSE},
	{"Request Out Of Range", ROOR},
	{"Security Access Denied", SAD},
	{"Invalid Key", IK},
	{"Exceeded Number Of Attempts", ENOA},
	{"Required Time Delay Not Expired", RTDNE},
	{"Response Too Long", RTL},
	{"Authentication Required", AR},
	{"General Programming Failure", GPF},
	{"Upload Download Not Accepted", UDNA},
	{"Transfer Data Suspended", TDS},
	{"Wrong Block Sequence Counter", WBSC},
	{"Voltage Too High", VTH},
	{"Voltage Too Low", VTL},
	{"Secure Data Verification Failed", SDVF},
	{NULL, -1}
};

char* sessionType = "safe";

GtkApplication *app;
//GtkTextBuffer *consoleText;
GtkWidget *window,
	*services,
	*subfunctionsHandle,
//	*console,
	*servicePanel,
	*usbPanel,
	*bluetoothPanel,
	*parameterPanel,
	*ideSwitch,
	*rtrSwitch,
	*connectionButtonsPanel,
	*connectionTypeSwitch,
	*idEntry,
	*idError,
	*suppressSwitch;

CustomButton* requestButton = NULL;

ServiceParam* servicesInfo[] = {&_DSC, &_ECUR, &_RDTCI, &_RDBI, NULL};

A_Data Message;

void ChangePanel(GtkWidget* widget, void* data) {

	if (parameterPanel) {
		GtkWidget* tempWidget = gtk_widget_get_first_child(parameterPanel);
		while (tempWidget) {
			g_object_ref(G_OBJECT(tempWidget));
			gtk_grid_remove(GTK_GRID(parameterPanel), tempWidget);
			tempWidget = gtk_widget_get_first_child(parameterPanel);
		}
		gtk_widget_hide(GTK_WIDGET(parameterPanel));
	}

	serviceIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(services));
	if (serviceIndex >= 0) {
	
		subfunctionIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(temp[serviceIndex].subfunctions));
	
		if (subfunctionIndex >= 0) {

			ConfigPanel* newPanel = temp[serviceIndex].parameters->subfunctionList[subfunctionIndex].panel;

			if (newPanel && newPanel->panel) {
				parameterPanel = newPanel->panel;
				for (unsigned int i = 0; newPanel->requestPanel->elements && newPanel->requestPanel->elements[i].handle; ++i) {
					GtkWidget* parentHandle = gtk_widget_get_parent(newPanel->requestPanel->elements[i].handle->panel);
					if (parentHandle) {
						g_object_ref(G_OBJECT(newPanel->requestPanel->elements[i].handle->panel));
						gtk_grid_remove(GTK_GRID(parentHandle), newPanel->requestPanel->elements[i].handle->panel);
					}
					if (newPanel->requestPanel->elements[i].handle->updateFunc)
						newPanel->requestPanel->elements[i].handle->updateFunc();
					gtk_grid_attach(GTK_GRID(newPanel->requestPanel->panel), newPanel->requestPanel->elements[i].handle->panel, newPanel->requestPanel->elements[i].xPos, newPanel->requestPanel->elements[i].yPos, 1, 1);
				}
				gtk_grid_attach(GTK_GRID(parameterPanel), newPanel->requestPanel->frame, 0, 0, 1, 1);
				for (unsigned int i = 0; newPanel->responsePanel->elements && newPanel->responsePanel->elements[i].handle; ++i) {
					GtkWidget* parentHandle = gtk_widget_get_parent(newPanel->responsePanel->elements[i].handle->panel);
					if (parentHandle) {
						g_object_ref(G_OBJECT(newPanel->responsePanel->elements[i].handle->panel));
						gtk_grid_remove(GTK_GRID(parentHandle), newPanel->responsePanel->elements[i].handle->panel);
					}
					if (newPanel->responsePanel->elements[i].handle->updateFunc)
						newPanel->responsePanel->elements[i].handle->updateFunc();
					gtk_grid_attach(GTK_GRID(newPanel->responsePanel->panel), newPanel->responsePanel->elements[i].handle->panel, newPanel->responsePanel->elements[i].xPos, newPanel->responsePanel->elements[i].yPos, 1, 1);
				}
				gtk_grid_attach(GTK_GRID(parameterPanel), newPanel->responsePanel->frame, 0, 1, 1, 1);
				gtk_widget_show(GTK_WIDGET(parameterPanel));
			}
		}
	}
}

static void UpdateGUI(GtkWidget* widget, void* data) {
	UpdatePanels();
}

int setConfigurations(Bool forceFlag) {

	unsigned int currentId; 
	unsigned int i = 0;

	if (GetValueFromEntry(&currentId, idEntry)) {
		gtk_label_set_text(GTK_LABEL(idError), "Not a hexadecimal value");
		return 1;
	}	

	if (!gtk_switch_get_state(GTK_SWITCH(ideSwitch)) && currentId > 0x7ff) {
		gtk_label_set_text(GTK_LABEL(idError), "Values must be between [0..0x7ff]");
		return 1;
	}

	if (gtk_switch_get_state(GTK_SWITCH(ideSwitch)) && currentId > 0x1fffffff) {
		gtk_label_set_text(GTK_LABEL(idError), "Values must be between [0..0x1fffffff]");
		return 1;
	}

	Bool currentIDE = gtk_switch_get_state(GTK_SWITCH(ideSwitch)), 
	     currentRTR = gtk_switch_get_state(GTK_SWITCH(rtrSwitch));

	if (forceFlag || !Identifier || (*Identifier != currentId) ||
			IDE != currentIDE ||
			RTR != currentRTR) {
		sprintf(buff, "Config: %u;%u;%u", currentId, currentIDE, currentRTR);
		printf("%s\n", buff);
		/**/
		if (Message.data)
			free(Message.data);
		Message.data = NULL;
		Message.Length = strlen(buff);
		Message.data = (BYTE*)malloc(Message.Length * sizeof(BYTE));
		memcpy(Message.data, buff, Message.Length);
		printf("Sending configurations: ");
		for (; i < Message.Length - 1; ++i)
			printf("%c", Message.data[i]);
		printf("%c\n\n", Message.data[i]);
		sleep(1);
		SENDDATA
		sleep(1);
		RECVDATA
		sleep(1);
		printf("Receiving configuration ack: ");
		if (Message.data) {
			for (i = 0; i < Message.Length - 1; ++i)
				printf("%c", Message.data[i]);
			printf("%c\n\n", Message.data[i]);
			if (!memcmp(Message.data, buff, Message.Length)) {
				if (!Identifier)
					Identifier = (unsigned int*)malloc(sizeof(unsigned int));
				*Identifier = currentId;
				IDE = currentIDE;
				RTR = currentRTR;
			} else {
				gtk_label_set_text(GTK_LABEL(idError), "Error while setting configurations");
				return 1;
			}
		} else {
			gtk_label_set_text(GTK_LABEL(requestButton->error), "Server timeout (config)");
			return 1;
		}
	}

	return 0;
}

void *makeRequest(void* data) {
	
	if (gtk_switch_get_state(GTK_SWITCH(connectionTypeSwitch))) {
		if ((!selectedDevice || s < 0)) {
			SETBUTTONERR(requestButton, "Connection has not been establised");
			requestButton->flag = 0;
			return NULL;
		}
	}

	if (setConfigurations(False)) {
		requestButton->flag = 0;
		printf("no\n");
		return NULL;
	}

	unsigned int i = 0;
	int status;
	
	gtk_label_set_text(GTK_LABEL(idError), NULL);

	pthread_t waitRequest;
	pthread_create(&waitRequest, NULL, buttonProgress, requestButton);
	subfunctionElement subfunction = temp[serviceIndex].parameters->subfunctionList[subfunctionIndex];

	for (i = 0; subfunction.panel->requestPanel->elements && subfunction.panel->requestPanel->elements[i].handle; ++i)
		if (subfunction.panel->requestPanel->elements[i].handle->requestFunc && subfunction.panel->requestPanel->elements[i].handle->requestFunc())
			break;

	if (subfunction.panel->requestPanel->elements[i].handle)
		gtk_label_set_text(GTK_LABEL(requestButton->error), "An error has occured");
	else {
		//gtk_label_set_text(GTK_LABEL(requestButton->error), NULL);
retry:
		if (Message.data)
			free(Message.data);
		Message.data = NULL;
		temp[serviceIndex].parameters->generateRequest(&Message, gtk_switch_get_state(GTK_SWITCH(suppressSwitch)), subfunction.subfunctionValue);
		printf("Sent A_Data.data: ");
		for (i = 0; i < Message.Length - 1; ++i)
			printf("0x%x_", Message.data[i]);
		printf("0x%x\nA_Data.Length: %u\n\n", Message.data[i], Message.Length);
		SENDDATA
		RECVDATA
		printf("Recv A_Data.data: ");
		if (Message.data) {
			if (Message.Length < 13 || memcmp(Message.data, "Error: Config", 13)) {
				for (i = 0; i < Message.Length - 1; ++i)
					printf("0x%x_", Message.data[i]);
				printf("0x%x\nA_Data.Length: %u\n\n", Message.data[i], Message.Length);
				if (Message.data[0] != 0x7f) {
					status = temp[serviceIndex].parameters->receiveResponse(Message, subfunction.subfunctionValue);
					/*if (status) {
						printf("Response analysis has encountered an error\n");
						if (status > 1) {
							printf("Checking for pending data packets\n");
							sleep(1);
							appendData();
							if (Message.data) {
								printf("Appended A_Data.data: ");
								for (i = 0; i < Message.Length - 1; ++i)
									printf("0x%x_", Message.data[i]);
								printf("0x%x\nA_Data.Length: %u\n\n", Message.data[i], Message.Length);
								temp[serviceIndex].parameters->receiveResponse(Message, subfunction.subfunctionValue);
							} else {
								printf("Unknown error retrying request\n");
								goto retry;
							}
						}
					}*/
				} else {
					for (unsigned int i = 0; neg_resp[i].name != NULL; ++i)
						if (neg_resp[i].code == Message.data[2]) {
							sprintf(buff, "Negative Response: %s", neg_resp[i].name);
							gtk_label_set_text(GTK_LABEL(requestButton->error), buff);
							break;
						}
				}
			} else {
				if (setConfigurations(True)) {
					requestButton->flag = 0;
					printf("no\n");
					pthread_join(waitRequest, NULL);
					return NULL;
				}
				goto retry;
			}
		} else {
			gtk_label_set_text(GTK_LABEL(requestButton->error), "Server timeout");
			requestButton->flag = 0;
			pthread_join(waitRequest, NULL);
			return NULL;
		}
		UpdatePanels(); // Grija aici
	}
	requestButton->flag = 0;
	pthread_join(waitRequest, NULL);
}

static void RequestCallback(GtkWidget* widget, void* data) {
	if (!strcmp(sessionType, "safe"))
		makeRequest(NULL);
	else {
		if (!requestButton->flag) {
			requestButton->flag = 1;
			pthread_t requestThread;
			pthread_create(&requestThread, NULL, makeRequest, NULL);
		} else 
			SETBUTTONERR(requestButton, "Request is still in progress");
	}

}

int NewService(GtkWidget* layout, Service* service) {

	service->subfunctions = gtk_combo_box_text_new();

	gtk_grid_attach(GTK_GRID(layout), service->subfunctions, 0, 3, 1, 1);
	gtk_widget_hide(GTK_WIDGET(service->subfunctions));
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(services), NULL, service->parameters->serviceName);
	
	for (unsigned int i = 0; service->parameters->subfunctionList[i].subfunctionName != NULL; ++i) {
		sprintf(buff, "%s (0x%X)", service->parameters->subfunctionList[i].subfunctionName, service->parameters->subfunctionList[i].subfunctionValue);
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(service->subfunctions), NULL, buff);
	}

	g_signal_connect(G_OBJECT(service->subfunctions), "changed", G_CALLBACK(ChangePanel), NULL);
	g_signal_connect(G_OBJECT(service->subfunctions), "show", G_CALLBACK(ChangePanel), NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(service->subfunctions), 0);

	return 0;
}

static void ChangeSubfunctions(GtkWidget* widget) {
	gtk_widget_hide(GTK_WIDGET(subfunctionsHandle));
	subfunctionsHandle = temp[gtk_combo_box_get_active(GTK_COMBO_BOX(widget))].subfunctions;
	gtk_widget_show(GTK_WIDGET(subfunctionsHandle));
}

static void ScanForDevices(GtkWidget* widget, void* data) {
	if (!strcmp(sessionType, "safe")) {
		if (gtk_switch_get_state(GTK_SWITCH(connectionTypeSwitch)))
			scanForDevices(NULL);
		else
			scanForPorts(NULL);
	} else {
		if (!scanButton->flag) {
			scanButton->flag = 1;
			pthread_t scanThread;
			if (gtk_switch_get_state(GTK_SWITCH(connectionTypeSwitch)))
				pthread_create(&scanThread, NULL, scanForDevices, NULL);
			else
				pthread_create(&scanThread, NULL, scanForPorts, NULL);
		} else 
			SETBUTTONERR(scanButton, "Scanning is still in progress");
	}		
}

static void ConnectToDevice(GtkWidget* widget, void* data) {
	if (!strcmp(sessionType, "safe")) {
		if (gtk_switch_get_state(GTK_SWITCH(connectionTypeSwitch)))
			connectToDevice(NULL);
		else
			connectToPort(NULL);
	} else {
		if (!connectButton->flag && !scanButton->flag) {
			connectButton->flag = 1;
			pthread_t connectThread;
			if (gtk_switch_get_state(GTK_SWITCH(connectionTypeSwitch)))
				pthread_create(&connectThread, NULL, connectToDevice, NULL);
			else
				pthread_create(&connectThread, NULL, connectToPort, NULL);
		} else 
			SETBUTTONERR(connectButton, "Connection has not been establised");
	}
}

static void CloseConnection(GtkWidget* widget, void* data) {
	if (!strcmp(sessionType, "safe")) {
		if (gtk_switch_get_state(GTK_SWITCH(connectionTypeSwitch)))
			closeSocket(NULL);
		else
			closePort(NULL);
	} else {
		if (!closeButton->flag && !scanButton->flag && !connectButton->flag) {
			closeButton->flag = 1;
			pthread_t closeThread;
			if (gtk_switch_get_state(GTK_SWITCH(connectionTypeSwitch)))
				pthread_create(&closeThread, NULL, closeSocket, NULL);
			else
				pthread_create(&closeThread, NULL, closePort, NULL);
		} else 
			SETBUTTONERR(closeButton, "Closing socket not available");
	}
}

void CreateConfigPanel() {
	
	GtkWidget *ideLabel = gtk_label_new("IDE: "),
	  *idLabel = gtk_label_new("CAN Identifier:"),
	  *physicalLabel = gtk_label_new("Physical"),
	  *remoteLabel = gtk_label_new("Remote"),
	  *rtrLabel = gtk_label_new("RTR:"),
	  *classicLabel = gtk_label_new("Classic"),
	  *extendedLabel = gtk_label_new("Extended"),
	  *configPanel = gtk_grid_new(),
	  *idePanel = gtk_grid_new(),
	  *rtrPanel = gtk_grid_new();

	GtkWidget *configPanelFrame = gtk_frame_new("ConfigPanel");

	ideSwitch = gtk_switch_new();
	rtrSwitch = gtk_switch_new();
	idEntry = gtk_entry_new();
	idError = gtk_label_new(NULL);

	gtk_frame_set_child(GTK_FRAME(configPanelFrame), GTK_WIDGET(configPanel));

	gtk_grid_attach(GTK_GRID(mainLayout), configPanelFrame, 1, 1, 1, 1);

	gtk_grid_attach(GTK_GRID(configPanel), idLabel, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(configPanel), idEntry, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(configPanel), idError, 2, 0, 1, 1);

	gtk_grid_attach(GTK_GRID(configPanel), idePanel, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(configPanel), ideLabel, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(idePanel), classicLabel, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(idePanel), ideSwitch, 2, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(idePanel), extendedLabel, 3, 0, 1, 1);
	
	gtk_grid_attach(GTK_GRID(configPanel), rtrPanel, 1, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(configPanel), rtrLabel, 0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(rtrPanel), physicalLabel, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(rtrPanel), rtrSwitch, 2, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(rtrPanel), remoteLabel, 3, 0, 1, 1);
}

static void ChangeConnection(GtkWidget* widget, void* user_data) {
	if (gtk_switch_get_state(GTK_SWITCH(widget))) {
		gtk_widget_hide(GTK_WIDGET(usbPanel));
		gtk_widget_show(GTK_WIDGET(bluetoothPanel));
		GtkWidget* parentHandle = gtk_widget_get_parent(scanButton->panel);
		if (parentHandle) {
			g_object_ref(G_OBJECT(scanButton->panel));
			gtk_grid_remove(GTK_GRID(parentHandle), scanButton->panel);
		}
		gtk_grid_attach(GTK_GRID(bluetoothPanel), scanButton->panel, 0, 0, 1, 1);
		parentHandle = gtk_widget_get_parent(connectionButtonsPanel);
		if (parentHandle) {
			g_object_ref(G_OBJECT(connectionButtonsPanel));
			gtk_grid_remove(GTK_GRID(parentHandle), connectionButtonsPanel);
		}
		gtk_grid_attach(GTK_GRID(bluetoothPanel), connectionButtonsPanel, 1, 0, 1, 1);
	} else {
		gtk_widget_show(GTK_WIDGET(usbPanel));
		gtk_widget_hide(GTK_WIDGET(bluetoothPanel));
		GtkWidget* parentHandle = gtk_widget_get_parent(scanButton->panel);
		if (parentHandle) {
			g_object_ref(G_OBJECT(scanButton->panel));
			gtk_grid_remove(GTK_GRID(parentHandle), scanButton->panel);
		}
		gtk_grid_attach(GTK_GRID(usbPanel), scanButton->panel, 0, 0, 1, 1);
		parentHandle = gtk_widget_get_parent(connectionButtonsPanel);
		if (parentHandle) {
			g_object_ref(G_OBJECT(connectionButtonsPanel));
			gtk_grid_remove(GTK_GRID(parentHandle), connectionButtonsPanel);
		}
		gtk_grid_attach(GTK_GRID(usbPanel), connectionButtonsPanel, 1, 0, 1, 1);
	}
}

void CreateConnectionPanel() {
	GtkWidget *btlabel = gtk_label_new("Select Bluetooth:"),
		*connectionPanel = gtk_grid_new(),
		*connectionPanelFrame = gtk_frame_new("ConnectionPanel"),
		*connectionTypePanel = gtk_grid_new(),
		*usbLabel = gtk_label_new("USB"),
		*_btLabel = gtk_label_new("Bluetooth"),
		*_usbLabel = gtk_label_new("Select interface: ");
	
	NEWBUTTON(scanButton, "Scan");
	NEWBUTTON(connectButton, "Connect");
	NEWBUTTON(closeButton, "Close");

	usbPanel = gtk_grid_new();
	bluetoothPanel = gtk_grid_new();
	connectionTypeSwitch = gtk_switch_new();
	connectionButtonsPanel = gtk_grid_new(),
	
	interfaceList = gtk_combo_box_text_new();

	mac_list = gtk_combo_box_text_new();

	g_signal_connect(G_OBJECT(mac_list), "changed", G_CALLBACK(ChangeMAC), NULL);
	
	g_signal_connect(G_OBJECT(interfaceList), "changed", G_CALLBACK(ChangePort), NULL);

	gtk_grid_attach(GTK_GRID(connectionTypePanel), usbLabel, 0, 0, 1, 1);
	
	gtk_grid_attach(GTK_GRID(connectionTypePanel), connectionTypeSwitch, 1, 0, 1, 1);
	
	gtk_grid_attach(GTK_GRID(connectionTypePanel), _btLabel, 2, 0, 1, 1);

	gtk_frame_set_child(GTK_FRAME(connectionPanelFrame), GTK_WIDGET(connectionPanel));

	gtk_grid_attach(GTK_GRID(mainLayout), connectionPanelFrame, 0, 1, 1, 1);
	
	gtk_grid_attach(GTK_GRID(connectionPanel), connectionTypePanel, 0, 0, 1, 1);

	gtk_grid_attach(GTK_GRID(connectionPanel), usbPanel, 0, 1, 1, 1);

	gtk_grid_attach(GTK_GRID(usbPanel), _usbLabel, 0, 1, 1, 1);

	gtk_grid_attach(GTK_GRID(usbPanel), interfaceList, 1, 1, 1, 1);

	gtk_widget_hide(GTK_WIDGET(usbPanel));

	gtk_grid_attach(GTK_GRID(connectionPanel), bluetoothPanel, 0, 1, 1, 1);

	gtk_grid_attach(GTK_GRID(bluetoothPanel), scanButton->panel, 0, 0, 1, 1);

	gtk_grid_attach(GTK_GRID(bluetoothPanel), connectionButtonsPanel, 1, 0, 1, 1);

	gtk_grid_attach(GTK_GRID(connectionButtonsPanel), connectButton->panel, 0, 0, 1, 1);
	
	gtk_grid_attach(GTK_GRID(connectionButtonsPanel), closeButton->panel, 0, 0, 1, 1);

	gtk_widget_hide(GTK_WIDGET(closeButton->panel));

	gtk_grid_attach(GTK_GRID(bluetoothPanel), btlabel, 0, 1, 1, 1);

	gtk_grid_attach(GTK_GRID(bluetoothPanel), mac_list, 1, 1, 1, 1);
	
	g_signal_connect(G_OBJECT(connectionTypeSwitch), "notify", G_CALLBACK(ChangeConnection), NULL);

	gtk_switch_set_active(GTK_SWITCH(connectionTypeSwitch), 1);
}

void CreateServicePanel() {
	
	servicePanel = gtk_grid_new();
	
	GtkWidget *serviceLabel = gtk_label_new("Service:"),
		  *subfunctionLabel = gtk_label_new("Subfunction:"),
		  *switchLabel = gtk_label_new("Suppress Response:"),
		  *suppressPanel = gtk_grid_new();

	GtkWidget *servicePanelFrame = gtk_frame_new("ServicePanel");
	
	suppressSwitch = gtk_switch_new();
	gtk_switch_set_active(GTK_SWITCH(suppressSwitch), 0);

	services = gtk_combo_box_text_new();

	gtk_frame_set_child(GTK_FRAME(servicePanelFrame), GTK_WIDGET(servicePanel));

	gtk_grid_attach(GTK_GRID(mainLayout), servicePanelFrame, 0, 0, 1, 1);

	//gtk_grid_attach(GTK_GRID(mainLayout), parameterPanel, 1, 0, 1, 1);

	//gtk_grid_attach(GTK_GRID(mainLayout), console, 0, 1, 2, 1);

	gtk_grid_attach(GTK_GRID(servicePanel), serviceLabel, 0, 0, 1, 1);

	gtk_grid_attach(GTK_GRID(servicePanel), services, 0, 1, 1, 1);

	gtk_grid_attach(GTK_GRID(servicePanel), subfunctionLabel, 0, 2, 1, 1);

	gtk_grid_attach(GTK_GRID(servicePanel), suppressPanel, 0, 4, 1, 1);

	gtk_grid_attach(GTK_GRID(suppressPanel), switchLabel, 0, 0, 1, 1);

	gtk_grid_attach(GTK_GRID(suppressPanel), suppressSwitch, 1, 0, 1, 1);

	gtk_grid_attach(GTK_GRID(servicePanel), requestButton->panel, 0, 5, 1, 1);
}

static void CreateGUI(GtkApplication* app, void* user_data) {

	window = gtk_application_window_new(app);
	gtk_window_set_default_size(GTK_WINDOW(window), -1, -1);
	gtk_window_set_title(GTK_WINDOW(window), "Window");

	NEWBUTTON(requestButton, "Request");

	mainLayout = gtk_grid_new();

	//parameterPanel = gtk_grid_new();

	//consoleText = gtk_text_buffer_new(NULL);

	//console = gtk_text_view_new();

	//gtk_text_view_set_buffer(GTK_TEXT_VIEW(console), GTK_TEXT_BUFFER(consoleText));

	//gtk_text_view_set_editable(GTK_TEXT_VIEW(console), 0);

	gtk_window_set_child(GTK_WINDOW(window), mainLayout);

	CreateServicePanel();

	CreateConfigPanel();

	CreateConnectionPanel();

	//g_object_ref(G_OBJECT(subfunctionLabel));
	//gtk_grid_remove(GTK_GRID(servicePanel), subfunctionLabel);
	//gtk_grid_attach(GTK_GRID(servicePanel), subfunctionLabel, 0, 2, 1, 1);

	unsigned int i = 0;
	for (; servicesInfo[i] != NULL; ++i) {
		temp[i].parameters = servicesInfo[i];
		NewService(servicePanel, &temp[i]);
	}

	subfunctionsHandle = temp[0].subfunctions;

	gtk_widget_show(GTK_WIDGET(subfunctionsHandle));

	g_signal_connect(G_OBJECT(services), "changed", G_CALLBACK(ChangeSubfunctions), NULL);
	g_signal_connect(G_OBJECT(requestButton->button), "clicked", G_CALLBACK(RequestCallback), NULL);
	g_signal_connect(G_OBJECT(scanButton->button), "clicked", G_CALLBACK(ScanForDevices), NULL);
	g_signal_connect(G_OBJECT(connectButton->button), "clicked", G_CALLBACK(ConnectToDevice), NULL);
	g_signal_connect(G_OBJECT(closeButton->button), "clicked", G_CALLBACK(CloseConnection), NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(services), 0);

	FuncList panels[] = {CreateDSCPanel, CreateECURPanel, CreateRDTCIPanel, CreateRDBIPanel, NULL};

	for (i = 0; panels[i]; ++i)
		panels[i]();

	parameterPanel = DSCPanel.panel;

	ChangePanel(NULL, NULL);

	gtk_widget_show(parameterPanel);

	gtk_widget_show(window);
}

int main(int argc, char** argv) {

	if (argc > 0 && !strncmp((char*)argv[0], "unsafe", 6))
		sessionType = argv[0];

	Message.data = NULL;

	app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);

	g_signal_connect(app, "activate", G_CALLBACK(CreateGUI), NULL);

	//g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(quit_app), NULL);

	//g_application_run(G_APPLICATION(app), argc, argv);
	g_application_run(G_APPLICATION(app), 0, NULL);

	return 0;
}
