#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "version.h"

uint8_t data[65536+21]; // scratch area for file building

uint16_t compLen = 0;
uint16_t uncompLen = 0;

int main(int argc, const char **argv) {
  FILE *f;
  uint16_t start;
  uint16_t len;
  uint16_t opos;
  uint16_t ipos;
  char name[11];

  if (argc < 3) {
    printf("nluncomp <in_blk> <out_blk>\n");
    return 1;
  }


  f = fopen(argv[1], "rb");
  if (!f) { printf("cannot open tap file\n"); return 1; }
  len = fread(data, 1, sizeof data, f);
  fclose(f);

  /* skip header */
  compLen = data[0x5f] | (data[0x60] << 8);
  uncompLen = data[0x61] | (data[0x62] << 8);

  compLen -= 25905;
  uncompLen -= 25905;

  opos = uncompLen;
  ipos = compLen;

  while (ipos > 0x62) {
    if (data[ipos] == 0xc0) {
      // compress
      uint8_t n = data[ipos-1];
      ipos -= 2;
      while (n --) data[opos--] = ' ';
    } else data[opos--] = data[ipos--];
  }

  f = fopen(argv[2], "wb");
  if (!f) { printf("cannot write data file\n"); return 1; }

  ipos ++;
  while (ipos < uncompLen) {
    fwrite(&data[ipos], 1, 64, f);
    fputc('\n', f);
    ipos += 64;
  }


  //fwrite(data, 1, uncompLen, f);
  fclose(f);

  return 0;
}