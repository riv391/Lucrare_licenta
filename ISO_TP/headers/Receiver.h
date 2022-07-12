#ifndef RECEIVER_H_
#define RECEIVER_H_
#include "Config.h"

int StartReceiving(Format, N_AI, N_Data*);

int R_RecvMessage(N_PCI*, N_Data*, BYTE);

int R_SendMessage(N_PCI*);

#endif

