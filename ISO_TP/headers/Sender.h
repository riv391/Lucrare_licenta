#ifndef SENDER_H_
#define SENDER_H_
#include "Config.h"

int S_SendFirstMessage(N_PCI*, N_Data*, unsigned int*);

int StartSender(Format, N_AI, N_Data*);

int S_RecvMessage(N_PCI*);

int S_SendMessage(N_PCI*, N_Data*, unsigned int*, BYTE);

#endif
