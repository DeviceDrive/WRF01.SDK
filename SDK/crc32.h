#ifndef _CRC32_H
#define _CRC32_H

unsigned int calcCrc(unsigned char* buffer, int size);
unsigned int* createCrcTable(void);
unsigned int crc32(unsigned int crc, unsigned int* crc_table, unsigned char* buffer, unsigned int size);

#endif