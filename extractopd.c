#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "version.h"

#define NR_TRACKS	40
#define NR_SECTORS	18
#define NR_CAT_SECTORS	7
#define SECTOR_SIZE	256
#define SECTOR_MASK	0xff00
#define LEN_TO_SECT(l)	(((l) + SECTOR_SIZE - 1) / SECTOR_SIZE)

#define MAX_FILES	111

uint32_t diskLen;
uint8_t *disk;

//#define W(w)  (((w & 0xff00) >> 8) | ((w & 0x00ff) << 8))
#define W(w)  (w)

uint8_t chksum(uint8_t *data, uint32_t len, uint8_t val) {
	while (len--) val ^= *data++;
	return val;
}

const static char usage[] = 
	"ExtractOPD - Microjack '25 "VERSION"\n\n"
	"Extracts OPD files to a set of single file .TAPs and a script for use with MkDisk.\n\n"
	"usage:\n"
	"\tExtractOPD disk.opd\n\n";

int main(int argc, char **argv) {
	FILE *f, *ds;

	if (argc < 2) {
		printf("%s\n", usage);
		return 1;
	}

	f = fopen(argv[1], "rb");
	if (!f) {
		printf("Error: cannot open disk\n");
		return 1;
	}

	fseek(f, 0, SEEK_END);

	diskLen = ftell(f);
	disk = (uint8_t *)malloc(diskLen);
	rewind(f);

	fread(disk, 1, diskLen, f);
	fclose(f);


	char name[11];
	uint8_t *hdr = disk + 256;
	uint16_t *rec;
	uint16_t sectorSize = 256;
	uint8_t *data;
	char fn[255];


	strcpy(fn, argv[1]);
	int q = strlen(fn) - 1;
	char *s = fn + strlen(fn) - 1;
	while (*s != '.' && q) { q--; s--; }
	strcpy(s, ".txt");
	while (*s != '/' && q) { q--; s--; }
	if (*s == '/') s++;

  printf("drive script name : %s\n", s);

	ds = fopen(s, "w");

	while (hdr[4] != 0xff && hdr[5] != 0xff) {
		rec = (uint16_t *)hdr;
		memcpy(name, hdr + 6, 10);
		name[10] = '\0';

		if (W(rec[1]) == 0) {
			sectorSize = W(rec[0])+1;
			printf("bps:%d cat_sect:%d max_files:%d diskname:%s\n", sectorSize,  W(rec[2]), ((W(rec[2])+1) * sectorSize / 16) - 2, name);
		} else {
			printf("bilb:%d ssect:%d esect:%d name:%s\n", W(rec[0]), W(rec[1]), W(rec[2]), name);

			data = disk + sectorSize + (W(rec[1])*sectorSize);

			uint8_t tap[21];
			memset(tap, 0x00, sizeof tap);
			tap[0] = 0x13;
			tap[1] = 0x00;
			tap[3] = data[0];
			memcpy(&tap[14], data + 1, 7);
			memcpy(&tap[4], name, 10);
			tap[20] = chksum(tap + 2, 18, 0x00);
			
			sprintf(fn, "_%s.tap", name);
			fprintf(ds, "_%s.tap\n", name);
			FILE *fout = fopen(fn, "wb");
			if (fout) {
				fwrite(tap, 1, sizeof tap, fout);
				uint16_t len = (data[1] | (data[2] << 8)) + 2;

				uint8_t blen[2];
				blen[0] = (len & 0xff);
				blen[1] = (len >> 8);
				fwrite(blen, 1, 2, fout);
				uint8_t ft = 0xff;
				fwrite(&ft, 1, 1, fout);
				fwrite(data + 7, 1, len - 2, fout);

				uint8_t cs = chksum(&ft, 1, 0x00);
				cs = chksum(data + 7, len - 2, cs);
				fwrite(&cs, 1, sizeof cs, fout);
				fclose(fout);
			}
		}
		hdr += 16;
	}
	fclose(ds);


	return 0;
}

