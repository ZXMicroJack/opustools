#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "version.h"

uint8_t data[65536+21]; // scratch area for file building

uint16_t compLen = 0;
uint16_t uncompLen = 0;

// 17757 - 455D

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

  printf("compLen %04X(%u) uncompLen %04X(%u)\n", compLen, compLen, uncompLen, uncompLen);

  compLen -= 25905;
  uncompLen -= 25905;

  opos = uncompLen;
  ipos = compLen;

  while (ipos > 0x62 && opos > 0x62) {
    if ((data[ipos] & 0xf0) == 0xc0) {
      // compress
      uint16_t n = data[ipos-1] | ((data[ipos] & 0x0f) << 8);
      ipos -= 2;
      while (n --) data[opos--] = ' ';
     
    // } else if (data[ipos] == 0xc3) {
    //   printf("c3 at opos = %d linepos = %d\n", opos, (opos - 0x62) % 64);
    //   uint16_t n = 886; // 54 chars
    //   while (n --) data[opos--] = ' ';
    //   ipos -= 2;



      // 76 c3
      // 76 = 118
      // 14 lines
      // 374 hex fewer bytes + 2
      // 886 fewer bytes
      // 7.5 chars per 
      // 6.5cm at .5cm per line - 13 lines - 832 chars

    } else data[opos--] = data[ipos--];
  }
  printf("opos %d ipos %d\n", opos, ipos);

  f = fopen(argv[2], "wb");
  if (!f) { printf("cannot write data file\n"); return 1; }

  ipos ++;
  while (ipos < uncompLen) {
    fwrite(&data[ipos], 1, 64, f);
    // fputc('\n', f);
    ipos += 64;
  }


  //fwrite(data, 1, uncompLen, f);
  fclose(f);

  return 0;
}