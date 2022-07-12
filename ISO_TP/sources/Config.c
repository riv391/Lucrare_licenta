#include "../headers/Config.h"
#include <stdlib.h>

TP_Handler handler;
void (*DelayFor)(BYTE interval, DelayType type) = 0;
int (*SendToDataLink)(BYTE*, Format*) = 0;
int (*RecvFromDataLink)(TP_Handler*, BYTE**) = 0;

int ConfigConnection(Format format, N_AI AddressInfo) {
	// Specifice nivelului transport
	handler.type = diagnostics;
	handler.TAtype = Physical;
	handler.Padding = OptimizedPadding;
	handler.AddressMode = Normal_11bits;
	
	// Identifier-ul de la datalink
	handler.AddressInfo = AddressInfo;

	// Format primit de la nivelul data-link ...?
	handler.ConnectionFormat = format;
	if (format.DLC > 8)
		handler.Padding = DataPadding;
	return 0;	
}

//int GenerateAIField(BYTE** res, N_AI* src, N_PCIType type) {
int GenerateAIField(BYTE** res, N_AI* src, N_PCIType type) {
	if (handler.TAtype == Functional && type != SF)
		return 1;

	if (ISNORMAL(handler.AddressMode) || handler.AddressMode == Mixed_11bits)
		MEMCPY(res, handler.AddressInfo.AI, 2); // !!! Nu e bun
		//(*res)[0] = src->SA;
		//(*res)[1] = src->TA;
		//(*res)[2] = src->TAType;

		// if (mode == Mixed_11bits)
		// Trebuie atasat N_AE la inceputul N_PCI-ului 
	if (handler.AddressMode == NormalFixed_29bits) {
		int mask = handler.AddressInfo.SA | (handler.AddressInfo.TA << 7) | ((218 << 15) * (handler.TAtype == Physical) + (219 << 15) * (handler.TAtype == Functional)) | (6 << 25);
		MEMCPY(*res, &mask, sizeof(int));
	}

	if (ISEXTENDED(handler.AddressMode)) {
		(*res)[0] = handler.AddressInfo.SA;
		(*res)[1] = handler.TAtype;

		// TA trebuie pus la inceputul N_PCI-ului... -_-
	}	

	if (handler.AddressMode == Mixed_29bits) {
		int mask = handler.AddressInfo.SA | (handler.AddressInfo.TA << 7) | ((206 << 15) * (handler.TAtype == Physical) | (205 << 15) * (handler.TAtype == Functional)) | (6 << 25);
		MEMCPY(*res, &mask, sizeof(int));
		
		// Trebuie atasat N_AE la inceputul N_PCI-ului
	}

	return 0;
}

