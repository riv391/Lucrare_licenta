#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_
#include "header.h"

extern struct sockaddr_rc address;
extern char** bt_devices;
extern char* selectedDevice;
extern unsigned int num_rsp;
extern char bt_buff[1024];
extern int s;
extern GtkWidget *mac_list;

int scan();
int connecttosock(char*);
int sendDataToSocket();
int recvDataFromSocket();
//int appendData();
void ChangeMAC(GtkWidget*, void*);
void updateMacList();
void *closeSocket(void*);
void *scanForDevices(void*);
void *connectToDevice(void*);

#endif
