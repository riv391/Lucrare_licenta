#include <stdlib.h>
#include "../headers/Config.h"
#include "../headers/CAN_Types.h"

int NewPCI(N_PCI* res, BYTE* Data) {
	if (res == NULL)
		res = (N_PCI*)malloc(sizeof(N_PCI));
	res->type = BYTE_HIGH(Data[0]); // Determina tipul mesajului receptionat
	switch (res->type) {
		case SF: return MessageToSF(&res, Data);
		case FF: return MessageToFF(&res, Data);
		case CF: return MessageToCF(&res, Data);
		case FC: return MessageToFC(&res, Data);
		default: return -1;
	}
}

int PCIToBytes(BYTE** res, N_PCI* src) {
	if (*res == NULL)
		*res = (BYTE*)calloc(8, sizeof(BYTE));
	else
		memset(*res, 0, 8 * sizeof(BYTE));
	(*res)[0] = (BYTE_LOW(src->type) << 4);
	switch(src->type) {
		case SF: return SFToMessage(res, src);
		case FF: return FFToMessage(res, src);
		case CF: return CFToMessage(res, src);
		case FC: return FCToMessage(res, src);
		default: return -1;
	}
}

int MessageToSF(N_PCI** dest, BYTE* Data) {
	
	// Determina lungimea din mesaj in functie de valorile din tableul 9 (32 of 60) din 15765-2 (valoare de 8 biti)
	BYTE DataLength = BYTE_LOW(Data[0]) * (BYTE_LOW(Data[0]) != 0) + (BYTE_LOW(Data[0]) == 0) * Data[1];

	// mode il iei din primul N_USData primit, poate va fi o variabila globala
	// Verifica cazurile de eroare

	unsigned int DLC = handler.ConnectionFormat.DLC;

	AddressingMode mode = handler.AddressMode;

	// Revezi N_PCI->DLC (nu mai exista)
	if (CAN_DL[DLC] <= 8) // Verifica conditiile de eroare pentru CAN_DL[DLC] <= 8 (tabel 10 (33 of 60) si tabel 12 (34 of 60)) 
	{
		if (DataLength == 0 || !(((ISNORMAL(mode) && CAN_DL[DLC] > 1 && CAN_DL[DLC] == DataLength + 1) || (!ISNORMAL(mode) && CAN_DL[DLC] > 2 && CAN_DL[DLC] == DataLength + 2))))
		       return -1;

	
	} else // Verifica conditiile de eroare pentru CAN_DL[DLC] > 8 (tabel 11 (33 of 60) si tabel 13 (34 of 60))
	       	if (!(BYTE_LOW(Data[0]) == 0 && ((ISNORMAL(mode) && CAN_DL[DLC - 1] < DataLength && CAN_DL[DLC] - 2 >= DataLength) || (!ISNORMAL(mode) && CAN_DL[DLC - 1] <= DataLength && CAN_DL[DLC] - 3 >= DataLength))))
	       		return -1;

	(*dest)->DL[1] = 0;
	(*dest)->DL[2] = 0;
	(*dest)->DL[3] = 0;
	(*dest)->DL[0] = DataLength;

	return 1 + (CAN_DL[DLC] > 8);
}

// Trebuie verificat, nu sunt sigur
int SFToMessage(BYTE** res, N_PCI* src) {

	if (CAN_DL[handler.ConnectionFormat.DLC] <= 8) {
		(*res)[0] |= src->DL[0];
		return 1;
	} else {
		(*res)[1] = src->DL[0];
		return 2;
	}
}

int MessageToFF(N_PCI** dest, BYTE* Data) {
	// Exista un FF->DLmin
	
	if (CAN_DL[handler.ConnectionFormat.DLC] < 8)
		return -1;

	// Determina lungimea din mesaj in functie de valorile din tableul 9 (32 of 60) din 15765-2 (valoare intre 12 si 32 biti)
	unsigned int DataLength = (Data[1] != 0 || BYTE_LOW(Data[0]) != 0) * ((BYTE_LOW(Data[0]) << sizeof(BYTE)) + Data[1]) + (Data[1] == 0 && BYTE_LOW(Data[0]) == 0) * BSWAP(ToINT(*Data + 2));

	if (DataLength < handler.FF_DLmin) // Vezi Table 14
		return -1;

	// Vezi conditia 2 din 9.6.3.2 (35 of 60)

	if (BYTE_LOW(Data[0]) == 0 && Data[1] == 0 && DataLength < (1 << 12))
		return -1;

	MEMCPY((*dest)->DL, (unsigned char*)(&DataLength), sizeof(unsigned int));

	return 2 + (DataLength >= (1 << 12)) * 4;
}

int FFToMessage(BYTE** res, N_PCI* src) {
	if (ToINT(src->DL) < handler.FF_DLmin) // Pentru FF_DLmin vezi tabelul 14 din (34 of 60)
		return -1;	

	if (ToINT(src->DL) < (1 << 12)) {
		(*res)[0] |= BYTE_LOW(src->DL[1]);
		(*res)[1] = src->DL[0];
		return 2;
	} else {
		(*res)[1] = 0;
		MEMCPY(*res + 2, src->DL, 4); // Poate trebuie un BSWAP pentru src->DL, ai grija !!!
		return 6;
	}
}

int MessageToCF(N_PCI** dest, BYTE* Data) {
	
	if (BYTE_HIGH(Data[0]) == 2) // Daca mesajul anterior este un CF
	{
		if ((*dest)->SN > 15)
			return -1;
		(*dest)->SN = (BYTE_LOW(Data[0]) + 1) * (BYTE_LOW(Data[0]) < 15); // 9.6.4.3
	} else {
		if (BYTE_HIGH(Data[0]) == 1) // Daca mesajul anterior este un FF
			(*dest)->SN = 1;
		else
		       	return -1;
	}

	return 1;
}

int CFToMessage(BYTE** res, N_PCI* src) {
	src->SN = (src->SN + 1) & 0x0f;
	(*res)[0] |= src->SN;
	return 1;
}

int MessageToFC(N_PCI** dest, BYTE* Data) {
	switch (BYTE_LOW(Data[0])) {
		case 0:
			(*dest)->FS = CTS;
			break;
		case 1:
			(*dest)->FS = WAIT;
			break;
		case 2:
			(*dest)->FS = OVFLW;
			break;
		default:
			return -1;
	}

	// BS este 0 pentru a indica faptul ca nu mai urmeaza nici un mesaj de tip FS

	if ((*dest)->BS == CTS)
		(*dest)->BS = Data[1];

	// Table 20 (38 of 60) si 9.6.5.5 (a doua cerinta nu o inteleg)
	// Vezi 9.6.5.6
	(*dest)->STmin = ((Data[2] >= 0x80 && Data[2] <= 0xf0) || (Data[2] >= 0xfa && Data[2] <= 0xff)) ? 0x7f : Data[2];

	return 3;
}

int FCToMessage(BYTE** res, N_PCI* src) {
//	(*res)[0] |= src->FS;
	(*res)[0] |= CTS;
	(*res)[1] = src->BS;
	(*res)[2] = src->STmin;
	return 3;
}
