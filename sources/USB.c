#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include "../headers/USB.h"

char usb_buff[1024];
GtkWidget *interfaceList;
int usb_fd = -1;

PortList* selected_port = NULL;
PortList* available_ports = NULL;

void CheckOSForAvailablePorts()
{
	char* temp_buff;
	size_t* read_size;
	temp_buff = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1 , 0);
	read_size = mmap(NULL, sizeof(size_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1 , 0);
	int link[2];

	if(pipe(link) == -1)
		fprintf(stderr, "error: execvp pipe\n");

	if (fork() == 0) {
		dup2 (link[1], STDOUT_FILENO);
		close(link[0]);
		close(link[1]);
		execl("/usr/bin/sh", "/usr/bin/sh", "-c", "/usr/bin/ls /dev | grep ttyUSB", NULL);
		fprintf(stderr, "dwm: execvp /usr/bin/sh\n");
	} else {
		wait(NULL);
		close(link[1]);
		*read_size = read(link[0], temp_buff, 1024);
		PortList* tempPort;
		while (available_ports) {
			tempPort = available_ports;
			available_ports = available_ports->next;
			free(tempPort->port_name);
			free(tempPort);
		}
		char* aux_ptr = temp_buff;
		char* line_ptr = strchr(aux_ptr, '\n');
		while (line_ptr) {
			tempPort = (PortList*)malloc(sizeof(PortList));
			tempPort->port_name = NULL;
			tempPort->port_name = (char*)malloc((line_ptr - aux_ptr + 1) * sizeof(char));
			strncpy(tempPort->port_name, aux_ptr, line_ptr - aux_ptr);
			tempPort->next = available_ports;
			available_ports = tempPort;
			aux_ptr = line_ptr + 1;
			line_ptr = strchr(aux_ptr, '\n');
		}
		munmap(temp_buff, sizeof *temp_buff);
		munmap(read_size, sizeof *read_size);
	}
}

void ChangePort(GtkWidget* widget, void* data) {
	char* activePort = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(interfaceList));

	if (activePort) {

		if (!selected_port || !selected_port->port_name || strncmp(selected_port->port_name, activePort, strlen(selected_port->port_name))) {
			gtk_widget_hide(GTK_WIDGET(closeButton->panel));
			gtk_widget_show(GTK_WIDGET(connectButton->panel));
		} else {
			gtk_widget_show(GTK_WIDGET(closeButton->panel));
			gtk_widget_hide(GTK_WIDGET(connectButton->panel));
		}

		g_free(activePort);
	}
}

void 
ShowPorts() {
	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(interfaceList));
	if (selected_port && selected_port->port_name) {
		sprintf(buff, "%s (*)", selected_port->port_name);
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(interfaceList), NULL, buff);
		gtk_combo_box_set_active(GTK_COMBO_BOX(interfaceList), 0);
	}

	for (PortList* port = available_ports; port; port = port->next)
		if (!selected_port || !selected_port->port_name || strcmp(selected_port->port_name, port->port_name))
			gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(interfaceList), NULL, port->port_name);

	if (gtk_combo_box_get_active(GTK_COMBO_BOX(interfaceList)) < 0)
		gtk_combo_box_set_active(GTK_COMBO_BOX(interfaceList), 0);
}

int sendDataToPort() {
	int status;
	if (usb_fd == -1)
		return 1;
	if (!Message.data)
		return 1;
	//status = send(usb_fd, Message.data, Message.Length, 0);
	for (unsigned int i = Message.Length; i != 0; buff[i - 1] = Message.data[i - 1], --i);
	buff[Message.Length] = '\r';
	status = write(usb_fd, buff, Message.Length + 1);
	if (status != Message.Length + 1)
		return 1;
	return 0;
}

int recvDataFromPort() {
	
	if (Message.data) {
		free(Message.data);
		Message.data = NULL;
	}

	//int bytes_read = recv(usb_fd, usb_buff, sizeof(usb_buff), 0);
	int bytes_read = read(usb_fd, usb_buff, sizeof(usb_buff));
	int msg_size = bytes_read;
	
	while (1) {
		bytes_read = read(usb_fd, usb_buff + msg_size, sizeof(usb_buff) - msg_size);
		if (bytes_read > 0)
			msg_size += bytes_read;
		else
			break;
	}

	msg_size -= (usb_buff[msg_size - 1] == '\r');
	
	if (msg_size > 0) {
		Message.data = (BYTE*)malloc(msg_size * sizeof(BYTE));
		memcpy(Message.data, usb_buff, msg_size);
		Message.Length = msg_size;
	}
}

void*
scanForPorts (void* data) {
	pthread_t waitScan;
	pthread_create(&waitScan, NULL, buttonProgress, scanButton);
	CheckOSForAvailablePorts();
	ShowPorts();
	scanButton->flag = 0;
	pthread_join(waitScan, NULL);
}

int connecttoport(PortList* option) {

	if (usb_fd != -1) {
		close(usb_fd);
		usb_fd = -1;
	}
	
	sprintf(buff, "/dev/%s", option->port_name);
	usb_fd = open(buff, O_RDWR | O_NOCTTY);

	if (usb_fd != -1) {

		struct termios tty;

		tcgetattr(usb_fd, &tty);

		tty.c_cflag &= ~PARENB;
		tty.c_cflag &= ~CSTOPB;
		tty.c_cflag &= ~CSIZE;
		tty.c_cflag |= CS8;
		tty.c_cflag &= ~CRTSCTS;
		tty.c_cflag |= CREAD | CLOCAL;
		tty.c_lflag &= ~ICANON;
		tty.c_lflag &= ~ECHO;
		tty.c_lflag &= ~ECHOE;
		tty.c_lflag &= ~ECHONL;
		tty.c_lflag &= ~ISIG;
		tty.c_iflag &= ~(IXON | IXOFF | IXANY);
		tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
		tty.c_oflag &= ~OPOST;
		tty.c_oflag &= ~ONLCR;
		tty.c_cc[VTIME] = 10;
		tty.c_cc[VMIN] = 0;
		cfsetispeed(&tty, B9600);
		cfsetospeed(&tty, B9600);

		tcsetattr(usb_fd, TCSANOW, &tty);
		
		selected_port = option;
		ShowPorts();
		return 0;
	}

	return 1;
}

void *connectToPort(void* data) {
	char* tempDevice = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(interfaceList));
	if (tempDevice) {
		PortList* tempPort;
		for (tempPort = available_ports; tempPort->port_name && strcmp(tempDevice, tempPort->port_name); tempPort = tempPort->next);
		g_free(tempDevice);
		if (tempPort && tempPort->port_name) {
			pthread_t waitConnection;
			pthread_create(&waitConnection, NULL, buttonProgress, connectButton);
			selected_port = tempPort;

			if (selected_port)
				connecttoport(selected_port);

			ShowPorts();
			connectButton->flag = 0;
			pthread_join(waitConnection, NULL);
		}
	}
}

void *closePort(void* data) {
	pthread_t waitClose;
	pthread_create(&waitClose, NULL, buttonProgress, closeButton);
	
	if (selected_port)
		selected_port = NULL;
	
	if (usb_fd != -1) {
		close(usb_fd);
		usb_fd = -1;
	}
	
	ShowPorts();
	closeButton->flag = 0;
	pthread_join(waitClose, NULL);
}
