#ifndef _ISO_TP_TYPES_H_
#define _ISO_TP_TYPES_H_

#include "headers/General_Types.h"
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

typedef enum {
	Normal_11bits,
	Normal_29bits,
	NormalFixed_29bits,
	Extended_11bits,
	Extended_29bits,
	Mixed_11bits,
	Mixed_29bits
} AddressingMode;

//Vezi (46 of 60) din ISO_15765-2

typedef enum {
	STmin,
	BS
} Parameter_Type;

typedef enum {
	Physical,
	Functional
} N_TAType;

typedef enum {
	N_OK,
       	N_TIMEOUT_A,
       	N_TIMEOUT_Bs,
       	N_TIMEOUT_Cr, 
	N_WRONG_SN, 
	N_INVALID_FS,
       	N_UNEXP_PDU,
       	N_WFT_OVRN,
       	N_BUFFER_OVFLW,
       	N_ERROR
} N_Result_Type;

typedef enum {
	N_OK,
       	N_RX_ON,
       	N_WRONG_PARAMETER,
       	N_WRONG_VALUE
}Result_ChangeParameter_Type;

typedef struct {
	BYTE N_SA;
	BYTE N_TA;
	N_TAType N_TAtype;
	BYTE N_AE;
	BYTE Length[4];
	Parameter_Type Parameter;
	BYTE Parameter_Value;
	N_Result_Type N_Result;
	Result_ChangeParameter_Type Result_ChangeParameter;
	BYTE* MessageData;
}N_AI;

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
	
}ISO_TP_Message;

#endif
