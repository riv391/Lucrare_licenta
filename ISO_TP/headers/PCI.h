#ifndef ISO_TP_TYPES_H_
#define ISO_TP_TYPES_H_

#include "General_Types.h"
/*
N_AI, address information

N_SA, network source address
	8bits
	00 - FF
	N_SA este folosit pentru codificarea unei entitati protocol nivel retea al transmitatorului

N_TA, network target address
	8 bits
	00 - FF
	N_TA este folosit pentru codificarea unuia sau mai multor entitati de tip protocol nivel retea primite la receptor (physical, functional in functie de N_TAtype)

N_TAtype, Network target address type
	enumeration
	Pentru tabela cu tipurile de M_TAtype vezi pagina (20 of 600 din ISO_15765-2
	Este o extensie al parametrului N_TA, are rolul de a codifica modul in care se executa comunicatia dintre doua noduri din aceeasi retea

Lungimea <Length>
	32 bits
	0000 0001 - FFFF FFFF
	Lungimea payload-ului care trebuie transmisa sau receptionata (max 2 ^ 32 <<4294967296>> octeti)

Payload <MessageData>
	Sir de octeti
	Lungimea este data de paramatrul descris mai sus
	Informatie primita de la protocoalele superioare

Parametru <Parameter>
	enumeration
	STmin || BS
	Acest parametru determina tipul nivelului retea

Rezultat <N_Result>
	enumeration
	N_OK || N_TIMEOUT_A || N_TIMEOUT_Bs || N_TIMEOUT_Cr, N_WRONG_SN, N_INVALID_FS, N_UNEXP_PDU, N_WFT_OVRN, N_BUFFER_OVFLW, N_ERROR
	Vezi (22 of 60) din ISO_15765-2 pentru descierea tipurilor
	Acest parametru contine rezultatul executiei serviciului

<Result_ChangeParameter>
	enumeration
	N_OK || N_RX_ON || N_WRONG_PARAMETER || N_WRONG_VALUE
	Vezi (23 of 60) din ISO_15765-2 pentru descierea tipurilor si a paramatrului
*/	

/*
 * misc
 *
 * Pentru CAN_FD exista un bit BRS care determina daca trebui folosit un bitrate diferit pentru partea de arbitrare, in cazul in care este necesar un bitrate diferit, atunci valoarea lui BRS trebuie sa fie mai mare sau egala cu bitrate-ul folosit pentru faza de arbitrare
 *
 * Trebuie sa ai in vedere capacitatea buffer-ului de transmitere TX_DL (Pentru (DLC <= 8) ? CLASIC CAN : CAN_FD)
 * Analog RX_DL
 */

/*
 * Diagrama de la (41 of 60) descrie comunicarea nivulului transport
 * cu informatiile de la 8.2 (17 of 60) pentru N_USData
 * si informatiile de la 10 (45 of 60) pentru L_Data
 */

//Vezi (46 of 60) din ISO_15765-2

// TAType este stiut doar de Sender, in cazul unei transmisii 1-n (Functional) mesajele transmise sunt SF, asa ca Receiver-ul nu are nevoie sa stie ce fel de TAType este folosit in comunicatie
// Table 24 Adresare normala pentru TAType = Physical
// Table 25 Adresare normala pentru TAType = Functional
// Table 26 Adresare normala fixa pe 29 biti pentru TAType = Physical (#5 si #7, vezi (20 of 60, table 4))
// Table 27 Adresare normala fixa pe 29 biti pentru TAType = Functional (#6 si #8)
//
// Table 28 Adresare extinsa pentru TAType = Physical
// Table 29 Adresare extinsa pentru TAType = Functional
//
// Table 30 Adresare mixta pe 29 biti pentru TAType = Physical (#5 si #7)
// Table 31 Adresare mixta pe 29 biti pentru TAType = Functional (#6 si #8)
// Table 32 Adresare mixta pe 11 biti pentru TAType = Physical (#1 si #3)
// Table 33 Adresare mixta pe 11 bits pentru TAType = Functional (#2 si #4)

typedef enum {
	SF, //SingleFrame
	FF, //FirstFrame
	CF, //ConsecutiveFrame
	FC  //FlowControl
} N_PCIType;

typedef enum {
	CTS, //ContinueToSend
	WAIT,
	OVFLW //OverFlow
} FlowStatus;

typedef struct {
	N_PCIType type;
	BYTE DL[4]; //DL
	BYTE SN; //SequenceNumber
	FlowStatus FS; //FlowStatus
	BYTE BS; //BlockSize
	BYTE STmin; //SeparationTime
} N_PCI;

typedef struct {
	unsigned int Length;
	BYTE* data;
} N_Data;

int NewPCI(N_PCI*, BYTE*);
int PCIToBytes(BYTE**, N_PCI*);
int MessageToSF(N_PCI**, BYTE*);
int SFToMessage(BYTE**, N_PCI*);
int MessageToFF(N_PCI**, BYTE*);
int FFToMessage(BYTE**, N_PCI*);
int MessageToCF(N_PCI**, BYTE*);
int CFToMessage(BYTE**, N_PCI*);
int MessageToFC(N_PCI**, BYTE*);
int FCToMessage(BYTE**, N_PCI*);

#endif
