#ifndef GENERAL_TYPESSERVICE_H_
#define GENERAL_TYPESSERVICE_H_

typedef unsigned char BYTE;
typedef unsigned short int WORD;

#define BYTE_HIGH(byte) ((byte & 240) >> 4)
#define BYTE_LOW(byte) (byte & 15)
#define ToINT(string) (*(unsigned int*)(&string))
#define BSWAP(bytes) ((((((((int)bytes[0] << 8) | bytes[1]) << 8) | bytes[2]) << 8) | bytes[3]))
#define LENGTH(vect) (sizeof(vect) / sizeof(vect[0]))

typedef struct {
	unsigned int Length;
	BYTE* data;
} A_Data;

typedef struct {
	BYTE DIDBH, DIDBL;
	A_Data dataRecord;
} DataIdentifier;

typedef enum {
	False,
	True
} Bool;

#endif
