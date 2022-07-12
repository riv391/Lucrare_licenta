#include <stdlib.h>
#include "../headers/Receiver.h"
#include "../headers/CAN_Types.h"

int StartReceiving(Format format, N_AI AddressInfo, N_Data* Msg) {
	// Se astepta primul mesaj
	// Se determina Identificatorul pe baza primului mesaj
	// Daca primul mesaj este SF atunci proceseaza payload-ul
	// Daca este FF atunci trimite un CF cu campurile BS si STMin si astepta urmatorele mesaje
	
	N_PCI pci;
	
	ConfigConnection(format, AddressInfo);
	
	pci.STmin = 0x50;//0x75; // Aloca timpul minim de la Receiver poate avea valori intre 0x00 - 0x76 si 0xf1 - 0xf9 (Table 20 (38 of 60))

	pci.BS = 0x02;

	R_RecvMessage(&pci, Msg, (1 << (SF + 1)) | (1 << (FF + 1)));

	if (pci.type == SF)
		return 0; // Am primit tot mesajul si se iese cu success

	if (pci.type != FF) return 1; // Daca nu este nici SF nici FF atunci a aparut o eroare la transmisie

	unsigned int Length = ToINT(pci.DL);

	while (Msg->Length < Length) {
		R_SendMessage(&pci);

		for (unsigned int i = 0; Msg->Length < Length && (!pci.BS || i < pci.BS); ++i) {
			R_RecvMessage(&pci, Msg, (1 << (CF + 1)));

			if (pci.type != CF)
				return 1;
		}
	}
	
	return 0;
}

int R_RecvMessage(N_PCI *pci, N_Data* Msg, BYTE mask) {
	// Primeste mesaj, citeste dlc-ul din mesajul primit
	BYTE* msg = NULL;
	
	if (!RecvFromDataLink) return 1;

	if (RecvFromDataLink(&handler, &msg)) // Aloca in msg payload-ul primit de la sender si updateaza handler-ul (DLC-ul)
		return 1;

	int msgRez = NewPCI(pci, msg);

	if (msgRez <= 0) return 1;

	if (!((1 << (pci->type + 1)) & mask)) return 1;

	if (pci->type == SF || pci->type == FF) {

		if (Msg == NULL) {
			Msg = (N_Data*)malloc(sizeof(N_Data));
			Msg->data = NULL;
		}

		if (Msg->data != NULL)
			free(Msg->data);
		
		Msg->data = (BYTE*)calloc(ToINT(pci->DL), sizeof(BYTE));

		Msg->Length = 0; // Initial lungimea mesajului este 0
	}

	MEMCPY(Msg->data + Msg->Length, msg + msgRez, CAN_DL[handler.ConnectionFormat.DLC] - msgRez); // Se adauga date la mesajul primit

	Msg->Length += CAN_DL[handler.ConnectionFormat.DLC] - msgRez; // Ar trebui la final lungimea mesajului trimis sa fie aceeasi ce cea a mesajului primit

	free(msg);

	return 0;
}

int R_SendMessage(N_PCI* pci) {
	pci->type = FC;
	
	BYTE *CAN_Data_Field = NULL;
	int pciSize = PCIToBytes(&CAN_Data_Field, pci);

	if (pciSize <= 0) return 1;

	switch (handler.Padding) {
		case DataPadding:
			memset(CAN_Data_Field + pciSize, 0xCC, CAN_DL[handler.ConnectionFormat.DLC] - pciSize); // Este necesara valoarea 0xCC pentru a reduce numarul de biti de stuffing introdusi
			break;
		
		case OptimizedPadding:
			handler.ConnectionFormat.DLC = pciSize;
			break;

		default: return 1;
	}
	
	if (SendToDataLink && SendToDataLink(CAN_Data_Field, &handler.ConnectionFormat))
		return 1;
	
	if (!DelayFor) return 1;

	// Vezi Table 20 (38 of 60)
	if (pci->STmin >= 0 && pci->STmin <= 0x7f)
		DelayFor(pci->STmin, micro);
	else if (pci->STmin >= 0xf1 && pci->STmin <= 0x79)
		DelayFor(pci->STmin, mili);
	return 0;
}
