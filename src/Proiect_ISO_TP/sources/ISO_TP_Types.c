#include <stdlib.h>
#include <string.h>
#include "../headers/ISO_TP_Types.h"
#include "../headers/CAN_Types.h"

#define ToINT(string) (*(unsigned int*)(&string))
#define BSWAP(bytes) ((((((((int)bytes[0] << 8) | bytes[1]) << 8) | bytes[2]) << 8) | bytes[3]))
#define ISMIXED(mode) (mode == Mixed_11bits || mode == Mixed_29bits)
#define ISEXTENDED(mode) (mode == Extended_11bits || mode == Extended_29bits)
#define ISNORMAL(mode) (mode == Normal_11bits || mode == Normal_29bits || mode == NormalFixed_29bits)

N_PCI* NewPCI(BYTE* Data, BYTE DLC) {
	N_PCI* res = (N_PCI*)malloc(sizeof(N_PCI));
	res->type = BYTE_HIGH(Data[0]); // Determina tipul mesajului receptionat
	switch (res->type) {
		case SF: NewSF(&res, Data, DLC); break;
		case FF: NewFF(&res, Data); break;
		case CF: NewCF(&res, Data); break;
		case FC: NewFC(&res, Data); break;
		default: break;
	}
}

BYTE* PCIToBytes(N_PCI src) {
	BYTE* res = (BYTE*)calloc(8 * sizeof(BYTE));
	res[0] = (BYTE_LOW(src->type) << 4);
	switch(src->type) {
		case SF: SFToBytes(&res, src); break;
		case FF: FFToBytes(&res, src); break;
		case CF: CFToBytes(&res, src); break;
		case FC: FCToBytes(&res, src); break;
		default: break;
	}
}

// Cred ca e ok :)
int NewSF(N_PCI** dest, BYTE* Data, BYTE DLC) {
	
	// Determina lungimea din mesaj in functie de valorile din tableul 9 (32 of 60) din 15765-2 (valoare de 8 biti)
	BYTE DataLength = BYTE_LOW(Data[0]) * (BYTE_LOW(Data[0]) != 0) + (BYTE_LOW(Data[0]) == 0) * Data[1];

	//TODO: Determina daca atunci cand CAN_DL[DLC] > 8 atunci transforma formatul din 11 biti in 29
	//mode = mode + (ISNORMAL(mode) && DataLength >= 8) + (!ISNORMAL(mode) && DataLength >= 7);

	// Verifica cazurile de eroare

	// if (CAN_DL[DLC] != 8 && TX_DL == 8)
	// 	return 1;

	// Verifica conditiile de eroare pentru CAN_DL[DLC] <= 8 (tabel 10 (33 of 60) si tabel 12 (34 of 60))
	
	if (!(CAN_DL[DLC] <= 8 && ((ISNORMAL(mode) && CAN_DL[DLC] > 1 && CAN_DL[DLC] == DataLength + 1) || (!ISNORMAL(mode) && CAN_DL[DLC] > 2 && CAN_DL[DLC] == DataLength + 2))))
	       return 1;

	// Verifica conditiile de eroare pentru CAN_DL[DLC] > 8 (tabel 11 (33 of 60) si tabel 13 (34 of 60))
	
	if (!(CAN_DL[DLC] > 8 && BYTE_LOW(Data[0]) == 0 && ((ISNORMAL(mode) && CAN_DL[DLC - 1] < DataLength && CAN_DL[DLC] - 2 >= DataLength) || (!ISNORMAL(mode) && CAN_DL[DLC - 1] <= DataLength && CAN_DL[DLC] - 3 >= DataLength))))
		return 1;

	(*dest)->DL[0] = DataLength;

	return 0;
}

// Trebuie verificat, nu sunt sigur
int SFToBytes(BYTE** res, N_PCI* src) {
	if ((ISNORMAL(mode) && src->DL[0] > 1) || (!ISNORMAL(mode) && src->DL[0] > 2)) {
		(*res)[(src->DL[0] >= 8 - (!ISNORMAL(mode) && src->DL[0] > 2))] = src->DL[0];
		return 0;
	} else return 1;
}

int NewFF(N_PCI** dest, BYTE* Data, int DLC) {
	// Exista un FF->DLmin
	
	if (CAN_DL[DLC] < 8)
		return 1;

	// Determina lungimea din mesaj in functie de valorile din tableul 9 (32 of 60) din 15765-2 (valoare intre 12 si 32 biti)
	int DataLength = (Data[1] != 0) * ((BYTE_LOW(Data[0]) << sizeof(BYTE)) + Data[1]) + (Data[1] == 0) * BSWAP(ToINT(*Data + 2));

	if (DataLength < FF_DLmin)
		return 1;

	// Vezi conditia 2 din 9.6.3.2 (35 of 60)

	if (BYTE_LOW(Data[0]) == 0 && Data[1] == 0 && DataLength < (1 << 12))
		return 1;

	memcpy((*dest)->DL, DataLength, sizeof(int));

	return 0;
}

int FFToBytes(BYTE** res, N_PCI* src, AddressingMode mode) {
	if (ToINT(src-DL) < src->DLmin)
		return 1;	

	if (ToINT(src->DL) <= (1 << 12)) {
		(*res)[0] = BYTE_LOW(src->DL[1]);
		(*res)[1] = src->DL[0];
	} else {
		(*res)[1] = 0;
		memcpy((*res) + 2 * sizeof(BYTE), src->DL, 4 * sizeof(BYTE));
	}

	return 0;
}

//	BYTE SN; //SequenceNumber
//	FlowStatus FS; //FlowStatus
//	BYTE BS; //BlockSize
//	BYTE STmin; //SeparationTime

