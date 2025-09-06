#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "version.h"

uint8_t data[65536]; // scratch area for file building
uint8_t out[65536];
uint16_t outLen = 0;

uint16_t compLen = 0;
uint16_t uncompLen = 0;

static const uint8_t lut[] = {
  0x65, 0x6b, 0x71, 0x77,
  0x7d, 0x83, 0x89, 0x8f,
  0x95, 0x9b, 0xa1, 0xa7, 
  0xad, 0xb3, 0xb9, 0xbf, 
  0xc5, 0xcb, 0xd1, 0xd7, 
  0xdd, 0xe3, 0xe9
};

int main(int argc, const char **argv) {
  FILE *f;
  uint16_t start;
  uint16_t len;
  uint16_t opos;
  uint16_t ipos;
  char name[11];

  if (argc < 4) {
    printf("nlcomp <in_blk> <out_blk> <blk_nr>\n");
    return 1;
  }

  f = fopen(argv[1], "rb");
  if (!f) { printf("cannot open tap file\n"); return 1; }
  len = fread(data, 1, sizeof data, f);
  fclose(f);

  printf("nr pages = %d\n", len / (64 * 24));
  uint8_t nrPages = len / (64 * 24);

  /* write header */
  f = fopen(argv[2], "wb");
  if (!f) { printf("cannot write data file\n"); return 1; }

  for (int i=0; i<nrPages; i++) {
    fputc(0x00, f);
    fputc(i+1, f);
    fputc(0x94, f);
    fputc(lut[i], f);
  }

  for (int i=nrPages; i<23; i++) {
    fputc(0x00, f);
    fputc(0x00, f);
    fputc(0x00, f);
    fputc(0x00, f);
  }

  uint8_t blockNr = atoi(argv[3]);
  fputc(blockNr, f);
  fputc(nrPages, f);
  fputc(0x00, f);

  uint16_t nrLen = 0;
  for (int i=0; i<len; i++) {
    if (data[i] == ' ') {
      nrLen ++;
      if (nrLen == 0xfff) {
        out[outLen++] = nrLen & 0xff;
        out[outLen++] = 0xc0 | (nrLen >> 8);
        nrLen = 0;
      }
    }
    else if (nrLen == 1) {
      out[outLen++] = ' ';
      out[outLen++] = data[i];
      nrLen = 0;
    } else if (nrLen > 1) {
      out[outLen++] = nrLen & 0xff;
      out[outLen++] = 0xc0 | (nrLen >> 8);
      out[outLen++] = data[i];
      nrLen = 0;
    } else {
      out[outLen++] = data[i];
    }
  }

  if (nrLen) {
    out[outLen++] = nrLen & 0xff;
    out[outLen++] = 0xc0 | (nrLen >> 8);
  }
  
  compLen = outLen + 25905 + 0x63;
  uncompLen = len + 25905 + 0x63;

  printf("Uncompressed len: %d\n", len);
  printf("Compressed len: %d\n", outLen);
  printf("Uncompressed end addr: %d (0x%04X)\n", uncompLen, uncompLen);
  printf("Compressed end addr: %d (0x%04X)\n", compLen, compLen);

  fputc(compLen & 0xff, f);
  fputc(compLen >> 8, f);
  fputc(uncompLen & 0xff, f);
  fputc(uncompLen >> 8, f);

  fwrite(out, 1, outLen, f);

  fclose(f);
  return 0;
}