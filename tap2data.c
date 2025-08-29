#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "version.h"

uint8_t data[65536+21]; // scratch area for file building

int main(int argc, const char **argv) {
  FILE *f;
  uint16_t start;
  uint16_t len;
  char name[11];

  f = fopen(argv[1], "rb");
  if (!f) { printf("cannot open tap file\n"); return 1; }
  len = fread(data, 1, sizeof data, f);
  fclose(f);

  f = fopen(argv[2], "wb");

  if (!f) { printf("cannot write data file\n"); return 1; }
  fwrite(&data[3+17+1+3], 1, len - (3+1+17+1+3), f);
  fclose(f);

  return 0;
}