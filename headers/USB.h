#ifndef USB_H_
#define USB_H_
#include "header.h"

typedef struct PortList {
	char* port_name;
	struct PortList* next;
} PortList;

extern char usb_buff[1024];
extern GtkWidget *interfaceList;
extern int usb_fd;

extern PortList* selected_ports;
extern PortList* available_ports;

void CheckOSForAvailablePorts();
void  ShowPorts();
void ChangePort(GtkWidget*, void*);
int sendDataToPort();
int recvDataFromPort();
void* scanForPorts(void*);
int connecttoport(PortList*);
void *connectToPort(void*);
void *closePort(void*);

#endif
