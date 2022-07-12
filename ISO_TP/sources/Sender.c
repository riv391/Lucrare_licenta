#include <stdlib.h>
#include "../headers/Sender.h"
#include "../headers/CAN_Types.h"

// Sender-ul va avea structurile de date:
// Format din Data_link.h
// N_AI din AI.h
// Si mesajele vor fi in N_PCI
// IDE din Data_link.h

int S_SendFirstMessage(N_PCI *pci, N_Data *Msg, unsigned int *offset) {

	BYTE mask = 0;

	if (Msg->Length < CAN_DL[handler.ConnectionFormat.DLC]) {
		pci->type = SF;
		pci->DL[0] = Msg->Length;
		mask = (1 << (SF + 1));
	} else {
		pci->type = FF;
		//unsigned int swapLength = BSWAP(Msg->Length);
		//MEMCPY(pci->DL, &swapLength, sizeof(unsigned int));
		MEMCPY(pci->DL, &Msg->Length, sizeof(unsigned int));
		mask = (1 << (FF + 1));
	}
	
	return S_SendMessage(pci, Msg, offset, mask);
}

int StartSender(Format format, N_AI AddressInfo, N_Data* Msg) {
/*
 *
 * 1) Trimite SF sau FF
 *
 * 2) Asteapta pentru un FC
 *
 * 3) In functie de FC fie trimite CF sau astepta pentru o perioada
 *
 */
	// Un mesaj este compus din N_AItobytes si N_PCItobyes si data
	N_PCI pci;
	unsigned int offset = 0;
	
	ConfigConnection(format, AddressInfo);

	if (S_SendFirstMessage(&pci, Msg, &offset)) return 1;

	while (offset < Msg->Length) {

retry:

		if (S_RecvMessage(&pci)) return 1;
		
		if (pci.type != FC)
			return 1;

		switch (pci.FS) {
			case CTS: 
				break;
			case WAIT:
				goto retry;
			case OVFLW: return 1; // Semnaleaza ca a aparut un N_BUFFER_OVFLW
			default: return 1;
		}
		
		pci.type = CF;

		// Trimite mesaje in functie de ultimul BS primit de la un FC
		// Daca BS-ul este 0 atuncti sender-ul trimite restul de mesaje fara sa mai astepte FC-uri
		for (int i = 0; offset < Msg->Length && (i < pci.BS || pci.BS == 0); ++i)
			S_SendMessage(&pci, Msg, &offset, (1 << (CF + 1)));
	}

	return 0;
}

// Ar trebui sa blocheze pana primeste un mesaj
int S_RecvMessage(N_PCI *pci) {
	BYTE* msg = NULL;

	BYTE tempDLC = handler.ConnectionFormat.DLC;

	if (RecvFromDataLink && RecvFromDataLink(&handler, &msg)) // Aloca in msg payload-ul primit de la sender si updateaza handler-ul (DLC-ul)
		return 1;

	int rez = NewPCI(pci, msg);
	free(msg);

	handler.ConnectionFormat.DLC = tempDLC;

	return (rez <= 0);
}

int S_SendMessage(N_PCI* pci, N_Data* Msg, unsigned int* offset, BYTE mask) {
	
	BYTE *CAN_Data_Field = NULL;
	int pciSize = PCIToBytes(&CAN_Data_Field, pci);

	if (!((1 << (pci->type + 1)) & mask)) return 1;

	if (pciSize <= 0) return 1;

	BYTE tempDLC = handler.ConnectionFormat.DLC;

	unsigned int	maxLength = CAN_DL[tempDLC],
			msgLength = (maxLength - pciSize) * (Msg->Length - *offset >= (maxLength - pciSize)) +
			 (Msg->Length - *offset) * (Msg->Length - *offset < (maxLength - pciSize));

	if (msgLength == 0 || msgLength + pciSize > maxLength) return 1;

	MEMCPY(CAN_Data_Field + pciSize, Msg->data + *offset, msgLength);

	// Trateaza cazul in care este nevoie de padding	
	if (msgLength + pciSize < maxLength)
		switch (handler.Padding) {
			case DataPadding:
				memset(CAN_Data_Field + pciSize + msgLength, 0xCC, maxLength - msgLength - pciSize); // Este necesara valoarea 0xCC pentru a reduce numarul de biti de stuffing introdusi
				break;
			
			case OptimizedPadding:
				handler.ConnectionFormat.DLC = pciSize + msgLength;
				break;

			default: break;
		}
	
	*offset += msgLength;
	
	if (!SendToDataLink) return 1;

	if (SendToDataLink(CAN_Data_Field, &handler.ConnectionFormat))
		return 1;

	handler.ConnectionFormat.DLC = tempDLC;

	if (!DelayFor) return 1;

	// Vezi Table 20 (38 of 60)
	if (pci->STmin >= 0 && pci->STmin <= 0x7f)
		DelayFor(pci->STmin, micro);
	else if (pci->STmin >= 0xf1 && pci->STmin <= 0x79)
		DelayFor(pci->STmin, mili);
	
	return 0;
}
