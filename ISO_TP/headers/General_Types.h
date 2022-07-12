#ifndef GENERAL_TYPES_H_
#define GENERAL_TYPES_H_

typedef unsigned char BYTE;
typedef unsigned short int WORD;

#define BYTE_HIGH(byte) ((byte & 240) >> 4)
#define BYTE_LOW(byte) (byte & 15)
#define ToINT(string) (*(unsigned int*)(&string))
#define BSWAP(uint) ((((((uint & 0x000f) << 8 | (uint & 0x00f0)) << 8) | (uint & 0x0f00)) << 8) | (uint & 0xf000))
//#define BSWAP(bytes) ((((((((unsigned int)bytes[0] << 8) | bytes[1]) << 8) | bytes[2]) << 8) | bytes[3]))

typedef enum {
	False,
	True
} Bool;

#endif
