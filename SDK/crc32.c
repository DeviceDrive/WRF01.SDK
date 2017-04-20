#include "crc32.h"
#include <stdlib.h> // malloc & free

unsigned int * createCrcTable(void)
{
	unsigned int* crc_table;
	unsigned int c;
	unsigned int i, j;
	crc_table = (unsigned int*)malloc(256 * 4);
	for (i = 0; i < 256; i++) {
		c = (unsigned int)i;
		for (j = 0; j < 8; j++) {
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		crc_table[i] = c;
	}
	return crc_table;
}

unsigned int crc32(unsigned int crc, unsigned int* crc_table, unsigned char *buffer, unsigned int size)
{
	unsigned int i;
	for (i = 0; i < size; i++) {
		crc = crc_table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
	}
	return crc;
}

/* Replace this function with your implementation */
unsigned int calcCrc(unsigned char* buffer, int size)
{
	unsigned int *crc_table = createCrcTable();
	unsigned int crc = 0xffffffff;
	crc = crc32(crc, crc_table, buffer, size);
	free(crc_table);
	return ~crc;
}