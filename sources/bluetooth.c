#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sco.h>
#include <bluetooth/sdp_lib.h>
#include "../headers/bluetooth.h"

struct sockaddr_rc address;
char** bt_devices = NULL;
char* selectedDevice = NULL;
unsigned int num_rsp = 0;
char bt_buff[1024];
int s = -1;
GtkWidget *mac_list;

int scan() {

	inquiry_info *ii = NULL;
	int max_rsp;
	int dev_id, sock, len, flags;
	int i;
	char addr[19] = { 0 };
	char name[248] = { 0 };

	if (bt_devices) {
		for (unsigned int i = 0; i < num_rsp; ++i)
			free(bt_devices[i]);
		free(bt_devices);
		bt_devices = NULL;
	}

	dev_id = hci_get_route(NULL);
	sock = hci_open_dev( dev_id );
	if (dev_id < 0 || sock < 0)
		return 1;

	len = 8;
	max_rsp = 255;
	flags = IREQ_CACHE_FLUSH;
	ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

	num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
	if( num_rsp < 0 ) return 1;

	bt_devices = (char**)malloc(num_rsp * sizeof(char*));

	for (i = 0; i < num_rsp; i++) {
		ba2str(&(ii+i)->bdaddr, addr);
		memset(name, 0, sizeof(name));
		if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0)
			strcpy(name, "[unknown]");
		sprintf(buff, "%s %s", addr, name);
		bt_devices[i] = (char*)malloc((strlen(buff) + 1) * sizeof(buff));
		strcpy(bt_devices[i], buff);
	}

	free( ii );
	close( sock );
	return 0;
}

int connecttosock(char* option) {

	char* addr = (char*)malloc(18 * sizeof(char));

	int status;

	if (s > 0) {
		close(s);
		s = -1;
	}

	strncpy(addr, option, 17);

	addr[17] = '\0';

	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof timeout);

	if (s < 0) {
		printf("%s\n", addr);
		free(addr);
		return 1;
	}

	str2ba(addr, &address.rc_bdaddr);
	free(addr);
	address.rc_family = AF_BLUETOOTH;
	address.rc_channel = (BYTE)1;

	status = connect(s, (struct sockaddr*)&address, sizeof(address));

	if (status < 0)
		return 1;

	return 0;
}

int sendDataToSocket() {
	int status;
	if (!Message.data)
		return 1;
	status = send(s, Message.data, Message.Length, 0);
	if (status != Message.Length)
		return 1;
	return 0;
}

int recvDataFromSocket() {
	
	if (Message.data) {
		free(Message.data);
		Message.data = NULL;
	}

	int bytes_read = recv(s, bt_buff, sizeof(bt_buff), 0);
	int msg_size = bytes_read;

	for (; bytes_read != -1; msg_size += (bytes_read) * (bytes_read > 0))
		bytes_read = recv(s, bt_buff + msg_size, sizeof(bt_buff) - msg_size, 0);

	if (msg_size > 0) {
		Message.data = (BYTE*)malloc(msg_size * sizeof(BYTE));
		memcpy(Message.data, bt_buff, msg_size);
		Message.Length = msg_size;
	}
	
	/*if (Message.data) {
		free(Message.data);
		Message.data = NULL;
	}
	int bytes_read = recv(s, bt_buff, sizeof(bt_buff), 0);
	if (bytes_read == 1) {
		bytes_read = recv(s, bt_buff + 1, sizeof(bt_buff) - 1, 0);
		if (bytes_read != -1)
			++bytes_read;
		else
			bytes_read = 1;
	}
	if (bytes_read > 0) {
		Message.data = (BYTE*)malloc(bytes_read * sizeof(BYTE));
		memcpy(Message.data, bt_buff, bytes_read);
		Message.Length = bytes_read;
	}*/
}

/*int appendData() {
	unsigned int oldLength = Message.Length;
	memcpy(buff, Message.data, Message.Length);
	recvData();
	if (Message.data) {
		memcpy(buff + oldLength, Message.data, Message.Length);
		Message.Length += oldLength;
		free(Message.data);
		Message.data = (BYTE*)malloc(Message.Length);
		memcpy(Message.data, buff, Message.Length);
	}
}*/

void ChangeMAC(GtkWidget* widget, void* data) {
	char* activeMAC = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(mac_list));

	if (activeMAC) {

		if (!selectedDevice || strncmp(selectedDevice, activeMAC, 17)) {
			gtk_widget_hide(GTK_WIDGET(closeButton->panel));
			gtk_widget_show(GTK_WIDGET(connectButton->panel));
		} else {
			gtk_widget_show(GTK_WIDGET(closeButton->panel));
			gtk_widget_hide(GTK_WIDGET(connectButton->panel));
		}

		g_free(activeMAC);
	}
}

void updateMacList() {
	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(mac_list));
	if (selectedDevice) {
		sprintf(buff, "%s (*)", selectedDevice);
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(mac_list), NULL, buff);
		gtk_combo_box_set_active(GTK_COMBO_BOX(mac_list), 0);
	}
	
	for (unsigned int i = 0; i < num_rsp; ++i)
		if (!selectedDevice || strcmp(selectedDevice, bt_devices[i]))
			gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(mac_list), NULL, bt_devices[i]);

	if (gtk_combo_box_get_active(GTK_COMBO_BOX(mac_list)) < 0)
		gtk_combo_box_set_active(GTK_COMBO_BOX(mac_list), 0);
}

void *scanForDevices(void* data) {
	pthread_t waitScan;
	pthread_create(&waitScan, NULL, buttonProgress, scanButton);
	scan();
	updateMacList();
	scanButton->flag = 0;
	pthread_join(waitScan, NULL);
}

void *closeSocket(void* data) {
	pthread_t waitClose;
	pthread_create(&waitClose, NULL, buttonProgress, closeButton);
	if (selectedDevice) {
		g_free(selectedDevice);
		selectedDevice = NULL;
	}
	if (s > 0) {
		close(s);
		s = -1;
	}
	updateMacList();
	closeButton->flag = 0;
	pthread_join(waitClose, NULL);
}

void *connectToDevice(void* data) {
	char* tempDevice = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(mac_list));
	if (tempDevice) {
		pthread_t waitConnection;
		pthread_create(&waitConnection, NULL, buttonProgress, connectButton);
		
		if (selectedDevice) {
			if (strncmp(selectedDevice, tempDevice, 17)) {
				g_free(selectedDevice);
				selectedDevice = tempDevice;
			} else 
				g_free(tempDevice);
		} else
			selectedDevice = tempDevice;

		if (selectedDevice)
			connecttosock(selectedDevice);

		updateMacList();
		connectButton->flag = 0;
		pthread_join(waitConnection, NULL);
	}
}

