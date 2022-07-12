#ifndef HEADER_H_
#define HEADER_H_
#include <gtk/gtk.h>
#include "../UDS_Services/headers/General_Types.h"

	#define CHECKWIDGET(widget, type) G_TYPE_CHECK_INSTANCE_TYPE(widget, type)

	#define CREATEELEMENT(element) {\
		element.panel = gtk_grid_new();\
		for (unsigned int i = 0; element.elements[i].widget; ++i)\
			gtk_grid_attach(GTK_GRID(element.panel), element.elements[i].widget, element.elements[i].xPos, element.elements[i].yPos, 1, 1);\
	}
	

	#define NEWBUTTON(but, label) {\
		but = (CustomButton*)malloc(sizeof(CustomButton));\
		but->panel = gtk_grid_new();\
		but->button = gtk_button_new_with_label(label);\
		but->error = gtk_label_new(NULL);\
		but->flag = 0;\
		but->err = 0;\
		gtk_grid_attach(GTK_GRID(but->panel), but->button, 0, 0, 1, 1);\
		gtk_grid_attach(GTK_GRID(but->panel), but->error, 0, 1, 1, 1);\
		gtk_widget_hide(GTK_WIDGET(but->error));\
	}

	#define SETBUTTONERR(but, errLabel) { \
		if (!but->err) {\
			pthread_t errThread;\
			but->err = 1;\
			gtk_label_set_text(GTK_LABEL(but->error), errLabel);\
			pthread_create(&errThread, NULL, buttonError, but);\
		}\
	}

	typedef int (*FuncList)();

	typedef struct {
		int xPos, yPos;
		GtkWidget* widget;
	} SimpleElement;

	typedef struct {
		GtkWidget* panel;
		SimpleElement* elements;
		int (*requestFunc)();
		int (*updateFunc)();
	} PanelElement;

	typedef struct {
		int xPos, yPos;
		PanelElement* handle;
	} PanelHandle;

	typedef struct {
		GtkWidget* frame;
		GtkWidget* panel;
		PanelHandle* elements;
	} SimplePanel;

	typedef struct {
		GtkWidget* panel;
		SimplePanel* requestPanel;
		SimplePanel* responsePanel;
	} ConfigPanel;

	typedef struct {
		unsigned char* subfunctionName;
		size_t subfunctionValue; 
		ConfigPanel* panel;
	} subfunctionElement;

	typedef struct {
		unsigned char* serviceName;
		int (*generateRequest)(A_Data*, Bool, BYTE);
		int (*receiveResponse)(A_Data, BYTE);
		subfunctionElement subfunctionList[];
	} ServiceParam;
	
	typedef struct {
		ServiceParam* parameters;
		GtkWidget* subfunctions;
	} Service;

	typedef struct {
		GtkWidget* panel, *button, *error;
		BYTE flag, err;
	} CustomButton;

	extern unsigned char buff[1024];

	extern GtkWidget *mainLayout;

	extern unsigned int* Identifier;
	extern Bool IDE,
		RTR;

	extern Service temp[4];

	extern PanelElement _NoParameters;

	extern A_Data Message;

	extern int subfunctionIndex, serviceIndex;
	extern CustomButton* requestButton;
	extern CustomButton *scanButton, *connectButton, *closeButton;

	int UpdateComboEvent(GtkWidget*, void*);
	int UpdateListEvent(GtkWidget*, void*);
	int UpdatePanels();
	int GetValueFromEntry(unsigned int*, GtkWidget*);
	int GetByteFromEntry(unsigned char**, GtkWidget*);
	int GetValueFromComboBox(unsigned int*, GtkWidget*);
	int GetSelectedValueFromList(unsigned int*, GtkWidget*);
	int CreateNewPanel(ConfigPanel*);
	void *buttonProgress(void*);
	void *buttonError(void*);

#endif
