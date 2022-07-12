#ifndef CONFIG_H_
#define CONFIG_H_

#include "PCI.h"

#define ISMIXED(mode) (mode == Mixed_11bits || mode == Mixed_29bits)
#define ISEXTENDED(mode) (mode == Extended_11bits || mode == Extended_29bits)
#define ISNORMAL(mode) (mode == Normal_11bits || mode == Normal_29bits || mode == NormalFixed_29bits)
#define MEMCPY(dest, src, length) for (int i = length; i > 0; (dest)[i - 1] = (src)[i - 1], --i)

typedef enum {
	diagnostics = 0x0U,
	remote = 0x2U
} MType;

typedef struct {
	BYTE SA;
	BYTE TA;
	BYTE TAType;
	BYTE N_AE;
	BYTE* AI;
} N_AI;

typedef enum {
	Std = 0x0U,
	Ext = 0x4U
} IDE_Field;

typedef enum {
	STmin,
	BS
} Parameter_Type;

//typedef enum {
//	N_OK,
//       	N_RX_ON,
//       	N_WRONG_PARAMETER,
//       	N_WRONG_VALUE
//}Result_ChangeParameter_Type;
//
//typedef enum {
//	N_OK,
//       	N_TIMEOUT_A,
//       	N_TIMEOUT_Bs,
//       	N_TIMEOUT_Cr, 
//	N_WRONG_SN, 
//	N_INVALID_FS,
//       	N_UNEXP_PDU,
//       	N_WFT_OVRN,
//       	N_BUFFER_OVFLW,
//       	N_ERROR
//} N_Result_Type;

typedef enum {
	Normal_11bits,
	Normal_29bits,
	NormalFixed_29bits,
	Extended_11bits,
	Extended_29bits,
	Mixed_11bits,
	Mixed_29bits
} AddressingMode;

typedef enum {
	Physical,
	Functional // Poate transmite mesaje 1-n, trimite doar pentru transmiterea de SF
} TAType;

typedef enum {
	DataPadding,
	OptimizedPadding
} PaddingType;

typedef enum {
	same,
	micro,
	mili
} DelayType;

typedef struct {
	MType RTR;
	MType SRR; // Placeholder pentru RTR in cazul mesajelor extinse (?)
	IDE_Field IDE;
	BYTE r0; // Rezervat vezi 3.1.1 din "Introduction to CAN sloa101b"
	BYTE r1; // Bit rezervat in cazul mesajelor extinse
	BYTE DLC; // 4 biti (de la 0-8 pentru Classic CAN si restul 9-15 pentru CAN_FD)
} Format;

typedef struct {
	MType type;			// Tipul transmisiei 
	PaddingType Padding;		// Tipul de padding al mesajului (DataPadding sau Otimized)
	TAType TAtype;			// Tipul mesajului: Physical sau Functional
	AddressingMode AddressMode;	// Modul de addresare: Standard, Extended sau Mixed
	unsigned int FF_DLmin;		// Dimensiunea minimă a unui mesaj fragmentat în octeți
	N_AI AddressInfo;		// Conține detalii despre blocul de identificare
	Parameter_Type Param;		//
	Format ConnectionFormat;	// Detalii despre formatul mesajului, valorile pentru: RTR, SRR, IDE, r0, r1 și DLC
} TP_Handler;

extern TP_Handler handler;

extern void (*DelayFor)(BYTE, DelayType);

extern int (*SendToDataLink)(BYTE*, Format*);

extern int (*RecvFromDataLink)(TP_Handler*, BYTE**);

int ConfigConnection(Format , N_AI);

int GenerateAIField(BYTE**, N_AI*, N_PCIType);

#endif
