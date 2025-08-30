#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "version.h"

uint8_t tap[21];
uint8_t hdr[2];
uint8_t data[65536]; // scratch area for file building

uint8_t chksum(uint8_t *data, uint32_t len, uint8_t val) {
	while (len--) val ^= *data++;
	return val;
}

//./data2tap test1.bin 1 25905 test1a.tap

void writeBlock(FILE *f, uint8_t *data, uint16_t len, uint8_t type) {
  uint8_t blen[3];
  uint8_t cs;

  len += 2;
  blen[0] = len & 0xff;
  blen[1] = len >> 8;
  blen[2] = type;

  fwrite(blen, 1, sizeof blen, f);
  fwrite(data, 1, len-2, f);
  cs = chksum(&blen[2], 1, 0x00);
  cs = chksum(data, len - 2, cs);
  fwrite(&cs, 1, sizeof cs, f);
}

int main(int argc, const char **argv) {
  FILE *f;
  uint16_t start;
  uint16_t len;
  char name[11];

  if (argc < 2) {
    printf("usage: data2tap <datafile> <name> <address> <output.tap>\n");
    return 1;
  }

  f = fopen(argv[1], "rb");
  if (!f) { printf("cannot open data file\n"); return 1; }
  len = fread(data, 1, sizeof data, f);
  fclose(f);

  uint16_t address = atol(argv[3]);

  uint8_t tap[17];
  memset(tap, 0x00, sizeof tap);
  tap[0] = 0x03;
  memset(&tap[1], ' ', 10);
  int nlen = strlen(argv[2]);
  memcpy(&tap[1], argv[2], nlen > 10 ? 10 : nlen);

  tap[11] = len & 0xff;
  tap[12] = len >> 8;
  tap[13] = address & 0xff;
  tap[14] = address >> 8;
  tap[15] = 0x20; // not sure what this is for
  tap[16] = 0x00;

  f = fopen(argv[4], "wb");
  if (!f) { printf("cannot write tap file\n"); return 1; }
  writeBlock(f, tap, sizeof tap, 0x00);
  writeBlock(f, data, len, 0xff);
  fclose(f);

  return 0;
}
