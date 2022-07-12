#ifndef ISO_TP_H_
#define ISO_TP_H_

#include "Config.h"

typedef enum {
	Idle,
	Config,
	Receiving,
	Sending
} TPState;

void StartDiagnosis();
void ManageUARTRequest(size_t);

extern TPState currentState;
extern N_Data MessageBuffer;
extern BYTE uartRx[1024];

extern int (*validConfiguration)();
extern int (*isExtended)();
extern int (*isRemote)();
extern void (*configureFormat)(Format*);
extern void (*configureInfo)(N_AI*);
extern void (*sendUart)(size_t timeout);
extern void (*resetTimer)();
extern void (*setLeds)(unsigned char);
extern void (*setIDE)(unsigned char);
extern void (*setIdentifier)(unsigned int);
extern void (*setRTR)(unsigned char);

#endif
