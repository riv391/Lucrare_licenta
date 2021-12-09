#ifndef _GENERAL_TYPES_H_
#define _GENERAL_TYPES_H_

typedef unsigned char BYTE;
typedef char WORD;

#define BYTE_HIGH(byte) ((byte & 240) >> 4)
#define BYTE_LOW(byte) (byte & 15)

typedef enum {
	False,
	True
} Bool;

#endif
