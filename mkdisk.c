#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NR_TRACKS	40
#define NR_SECTORS	18
#define NR_CAT_SECTORS	7
#define SECTOR_SIZE	256
#define SECTOR_MASK	0xff00
#define LEN_TO_SECT(l)	(((l) + SECTOR_SIZE - 1) / SECTOR_SIZE)

#define MAX_FILES	111
struct {
	uint8_t *data;
	uint32_t len;
} files[MAX_FILES];

int nrFiles = 0;

const static uint8_t bootsec[] = {
  0x18, 0x05, 0x28, 0x12, 0x40, 0x44, 0x04, 0x7e, 0xdd, 0x77, 0x00, 0x23, 0x7e, 0xdd, 0x77, 0x01,
  0xdd, 0x7e, 0x02, 0xe6, 0x2f, 0x57, 0x23, 0x7e, 0xe6, 0xd0, 0xb2, 0xdd, 0x77, 0x02, 0xc9, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x40, 0xe5, 0x01, 0xf7, 0x18, 0x4e, 0x0c, 0x00, 0x03, 0xf5, 0x01, 0xfe, 0x01, 0x27, 0x01, 0x00,
  0x01, 0x07, 0x01, 0x01, 0x01, 0xf7, 0x16, 0x4e, 0x0c, 0x00, 0x03, 0xf5, 0x01, 0xfb, 0x40, 0xe5,
  0x40, 0xe5, 0x40, 0xe5, 0x40, 0xe5, 0x01, 0xf7, 0x18, 0x4e, 0x0c, 0x00, 0x03, 0xf5, 0x01, 0xfe,
  0x01, 0x27, 0x01, 0x00, 0x01, 0x0e, 0x01, 0x01, 0x01, 0xf7, 0x16, 0x4e, 0x0c, 0x00, 0x03, 0xf5,
  0x01, 0xfb, 0x40, 0xe5, 0x40, 0xe5, 0x40, 0xe5, 0x40, 0xe5, 0x01, 0xf7, 0x18, 0x4e, 0x0c, 0x00,
  0x03, 0xf5, 0x01, 0xfe, 0x01, 0x27, 0x01, 0x00, 0x01, 0x03, 0x01, 0x01, 0x01, 0xf7, 0x16, 0x4e,
  0x0c, 0x00, 0x03, 0xf5, 0x01, 0xfb, 0x40, 0xe5, 0x40, 0xe5, 0x40, 0xe5, 0x40, 0xe5, 0x01, 0xf7,
  0x18, 0x4e, 0x0c, 0x00, 0x03, 0xf5, 0x01, 0xfe, 0x01, 0x27, 0x01, 0x00, 0x01, 0x0a, 0x01, 0x01
};

uint8_t cat[7*256];
uint8_t data[65536]; // scratch area for file building
uint8_t diskname[10];

void openTap(const char *s) {
	FILE *f = fopen(s, "rb");
	if (f && nrFiles < MAX_FILES) {
		fseek(f, 0, SEEK_END);
		files[nrFiles].len = ftell(f);
		rewind(f);
		files[nrFiles].data = (uint8_t *)malloc(files[nrFiles].len);
		fread(files[nrFiles].data, 1, files[nrFiles].len, f);
		fclose(f);
		nrFiles ++;
	}
}

void writeEndHeader(uint8_t *rec, const char *name) {
	uint16_t bytesPerSector = SECTOR_SIZE - 1;
	rec[0] = bytesPerSector & 0xff; // bytes per sector
	rec[1] = bytesPerSector >> 8;

	uint16_t totalBlocks = NR_TRACKS * NR_SECTORS - 1;

	rec[2] = totalBlocks & 0xff;
	rec[3] = totalBlocks >> 8;

	rec[4] = 0xff;
	rec[5] = 0xff;

	memcpy(rec + 6, name, 10);
}


uint16_t writeHeader(uint8_t *rec, uint16_t len, uint16_t ssect, const char *name) {
	char n[11];
	memcpy(n, name, 10);
	n[10] = '\0';

	uint16_t bytesInLastSector = ((len - 1) % SECTOR_SIZE);
	rec[0] = bytesInLastSector & 0xff;
	rec[1] = bytesInLastSector >> 8;

	rec[2] = ssect & 0xff;
	rec[3] = ssect >> 8;

	uint16_t esect = ssect + ((len + SECTOR_SIZE - 1) / SECTOR_SIZE) - 1;
	printf("Write header %s len %d start %d end %d\n", n, len, ssect, esect);

	rec[4] = esect & 0xff;
	rec[5] = esect >> 8;

	memcpy(rec + 6, name, 10);
	return esect + 1;
}

void readTapFilesFromScript(char *script) {
	FILE *f = fopen(script, "r");
	char str[256];
	int l = 0;
	if (f) {
		while (!feof(f)) {
				str[l] = fgetc(f);
				if (str[l] == '\r' || str[l] == '\n') {
					if (l) {
						str[l] = '\0';
						printf("Adding: %s\n", str);
						openTap(str);
						l = 0;
					}
				}
				else l ++;
		}
		fclose(f);
	}
}

int main(int argc, char **argv) {
	int disknameLen = strlen(argv[1]);

	if (disknameLen > 10) {
	  memcpy(diskname, argv[1], 10);
	} else {
	  memset(diskname, ' ', sizeof diskname);
	  memcpy(diskname, argv[1], disknameLen);
	}


	for (int i=2; i<argc; i++) {
		if (argv[i][0] == '@') {
			readTapFilesFromScript(&argv[i][1]);
		}
		openTap(argv[i]);

	}

	char filename[255];
	sprintf(filename, "%s.opd", argv[1]);

	FILE *f = fopen(filename, "wb");
	if (f) {
		fwrite(bootsec, 1, sizeof bootsec, f);
		memset(cat, 0xe5, sizeof cat);

		uint8_t *hdr = (uint8_t *)cat;
		uint16_t ssect = 0;

		ssect = writeHeader(cat, NR_CAT_SECTORS*SECTOR_SIZE, ssect, diskname);
		hdr += 16;

		for (int i=0; i<nrFiles; i++) {
			ssect = writeHeader(hdr, files[i].len - (21 + 3 + 1) + 7, ssect, files[i].data+4);
			hdr += 16;
		}
		writeEndHeader(hdr, diskname);


		fwrite (cat, 1, sizeof cat, f);

		uint16_t nrBlocks = 1 + (sizeof cat) / SECTOR_SIZE;

		for (int i=0; i<nrFiles; i++) {
			memset(data, 0xe5, sizeof data);
			data[0] = files[i].data[3]; // type
			data[1] = files[i].data[14]; // length
			data[2] = files[i].data[15];
			data[3] = files[i].data[16]; // address
			data[4] = files[i].data[17];
			data[5] = files[i].data[18]; // something
			data[6] = files[i].data[19];
			memcpy(&data[7], files[i].data + 21 + 3, files[i].len - (21 + 3 + 1));
			int len = 7 + files[i].len - (21 + 3 + 1);
			fwrite(data, 1, LEN_TO_SECT(len) * SECTOR_SIZE, f);
			
			nrBlocks += LEN_TO_SECT(len);
		}

		memset(data, 0xe5, 256);
		for (int i=nrBlocks; i<(40*18); i++) {
			fwrite(data, 1, 256, f);
		}
		fclose(f);

	}	



	for (int i=0; i<nrFiles; i++) {
		free(files[i].data);
	}

	return 0;
}
