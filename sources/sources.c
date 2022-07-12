#include "../headers/header.h"
#include <pthread.h>
#include <string.h>

GtkWidget *mainLayout;

unsigned int* Identifier = NULL;
Bool IDE = False,
	RTR = False;

Service temp[4];
int subfunctionIndex, serviceIndex;
unsigned char buff[1024];
CustomButton *scanButton = NULL, *connectButton = NULL, *closeButton = NULL;

int GetValueFromEntry(unsigned int* res, GtkWidget* entry) {
	
	const char* temp = gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(gtk_entry_get_buffer(GTK_ENTRY(entry))));
	*res = 0;

	if (*temp == '\0')
		return 1;

	for (const char* i = temp; *i != '\0'; ++i) {
		if ((*i <= '9' && *i >= '0') || (*i <= 'f' && *i >= 'a') || (*i <= 'F' && *i >= 'A')) {
			*res *= 16;
			if (*i <= '9' && *i >= '0')
				*res += *i - '0';
			if (*i <= 'f' && *i >= 'a')
				*res += *i - 'a' + 10;
			if (*i <= 'F' && *i >= 'A')
				*res += *i - 'A' + 10;
		} else return 1;
	}

	return 0;
}

int GetByteFromEntry(unsigned char** result, GtkWidget* entry) {
	
	unsigned int res;

	if (GetValueFromEntry(&res, entry)) return 1;

	if (res < 0 || res > 255)
		return 1;

	if (!(*result))
		*result = (unsigned char*)malloc(sizeof(unsigned char));
	
	*(*result) = res;

	return 0;
}

int GetSelectedValueFromList(unsigned int* result, GtkWidget* list_box) {

	GtkListBoxRow* row = gtk_list_box_get_selected_row(GTK_LIST_BOX(list_box));

	if (!row)
		row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(list_box), 0);

	if (!row) {
		*result = -1;
		return 1;
	}

	const char* temp = gtk_label_get_text(GTK_LABEL(gtk_list_box_row_get_child(row)));

	if (*temp == '\0') {
		*result = -1;
		return 1;
	}

	unsigned int res = 0;

	for (const char* i = temp; *i != '\0'; ++i) {
		if ((*i <= '9' && *i >= '0') || (*i <= 'f' && *i >= 'a') || (*i <= 'F' && *i >= 'A')) {
			res *= 16;
			if (*i <= '9' && *i >= '0')
				res += *i - '0';
			if (*i <= 'f' && *i >= 'a')
				res += *i - 'a' + 10;
			if (*i <= 'F' && *i >= 'A')
				res += *i - 'A' + 10;
		} else return 1;
	}

	*result = res;
	return 0;
}

int GetValueFromComboBox(unsigned int* result, GtkWidget* combobox) {
	char* temp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combobox));

	if (!temp) {
		*result = -1;
		return 1;
	}
	
	if (*temp == '\0') {
		*result = -1;
		g_free(temp);
		return 1;
	}

	unsigned int res = 0;

	for (char* i = temp; *i != '\0'; ++i) {
		if ((*i <= '9' && *i >= '0') || (*i <= 'f' && *i >= 'a') || (*i <= 'F' && *i >= 'A')) {
			res *= 16;
			if (*i <= '9' && *i >= '0')
				res += *i - '0';
			if (*i <= 'f' && *i >= 'a')
				res += *i - 'a' + 10;
			if (*i <= 'F' && *i >= 'A')
				res += *i - 'A' + 10;
		} else return 1;
	}

	*result = res;
	g_free(temp);
	return 0;
}

int UpdateComboEvent(GtkWidget* combo, void* data) {
	ConfigPanel* newPanel = temp[serviceIndex].parameters->subfunctionList[subfunctionIndex].panel;

	for (unsigned int i = 0, flag = 1; newPanel->requestPanel->elements && newPanel->requestPanel->elements[i].handle; ++i)
		if (newPanel->requestPanel->elements[i].handle->updateFunc) {
			if (!flag)
				newPanel->requestPanel->elements[i].handle->updateFunc();
			if (GTK_WIDGET(combo) == GTK_WIDGET(newPanel->requestPanel->elements[i].handle->elements[1].widget))
				flag = 0;
		}

}

int UpdateListEvent(GtkWidget* list, void* data) {
	GtkListBoxRow* selectedRow = gtk_list_box_get_selected_row(GTK_LIST_BOX(list));

	ConfigPanel* newPanel = temp[serviceIndex].parameters->subfunctionList[subfunctionIndex].panel;
	
	for (unsigned int i = 0, flag = 1; newPanel->responsePanel->elements && newPanel->responsePanel->elements[i].handle; ++i)
		if (newPanel->responsePanel->elements[i].handle->updateFunc) {
			if (!flag)
				newPanel->responsePanel->elements[i].handle->updateFunc();
			if (GTK_WIDGET(list) == GTK_WIDGET(newPanel->responsePanel->elements[i].handle->elements[1].widget))
				flag = 0;
		}
	
	gtk_list_box_select_row(GTK_LIST_BOX(list), selectedRow);
}

int UpdatePanels() {
	ConfigPanel* newPanel = temp[serviceIndex].parameters->subfunctionList[subfunctionIndex].panel;

	for (unsigned int i = 0; newPanel->requestPanel->elements && newPanel->requestPanel->elements[i].handle; ++i)
		if (newPanel->requestPanel->elements[i].handle->updateFunc)
			newPanel->requestPanel->elements[i].handle->updateFunc();

	for (unsigned int i = 0; newPanel->responsePanel->elements && newPanel->responsePanel->elements[i].handle; ++i)
		if (newPanel->responsePanel->elements[i].handle->updateFunc)
			newPanel->responsePanel->elements[i].handle->updateFunc();

}

int CreateNewPanel(ConfigPanel* NewPanel) {
	NewPanel->panel = gtk_grid_new();
	gtk_grid_attach(GTK_GRID(mainLayout), NewPanel->panel, 1, 0, 1, 1);
	if (!NewPanel->requestPanel) {
		NewPanel->requestPanel = (SimplePanel*)malloc(sizeof(SimplePanel));
		NewPanel->requestPanel->frame = gtk_frame_new("Request Panel");
		NewPanel->requestPanel->panel = gtk_grid_new();
		gtk_frame_set_child(GTK_FRAME(NewPanel->requestPanel->frame), GTK_WIDGET(NewPanel->requestPanel->panel));
	}
	if (!NewPanel->responsePanel) {
		NewPanel->responsePanel = (SimplePanel*)malloc(sizeof(SimplePanel));
		NewPanel->responsePanel->frame = gtk_frame_new("Response Panel");
		NewPanel->responsePanel->panel = gtk_grid_new();
		gtk_frame_set_child(GTK_FRAME(NewPanel->responsePanel->frame), GTK_WIDGET(NewPanel->responsePanel->panel));
	}
	//if (!gtk_widget_get_parent(GTK_WIDGET(NewPanel->requestPanel->panel)))
	//	gtk_grid_attach(GTK_GRID(NewPanel->panel), NewPanel->requestPanel->panel, 0, 0, 1, 1);
	//if (!gtk_widget_get_parent(GTK_WIDGET(NewPanel->responsePanel->panel)))
	//	gtk_grid_attach(GTK_GRID(NewPanel->panel), NewPanel->responsePanel->panel, 0, 1, 1, 1);
	gtk_widget_hide(GTK_WIDGET(NewPanel->panel));
}

void *buttonProgress(void* data) {
	if (!data)
		return NULL;
	char buff[20];
	CustomButton* button = (CustomButton*)data;
	const char* buttonLabel = gtk_button_get_label(GTK_BUTTON(button->button));
	size_t labelSize = strlen(buttonLabel);
	memcpy(buff, buttonLabel, labelSize);
	for (unsigned char i = 0; button->flag; i = (i + 1) * (i < 2)) {
		buff[labelSize + i] = '.';
		buff[labelSize + i + 1] = '\0';
		gtk_button_set_label(GTK_BUTTON(button->button), buff);
		sleep(1);
	}
	buff[labelSize] = '\0';
	gtk_button_set_label(GTK_BUTTON(button->button), buff);
}

void *buttonError(void* data) {
	if (!data)
		return NULL;
	CustomButton* button = (CustomButton*)data;
	if (button->err) {
		gtk_widget_show(GTK_WIDGET(button->error));
		sleep(3);
		gtk_widget_hide(GTK_WIDGET(button->error));
		button->err = 0;
	}

}
