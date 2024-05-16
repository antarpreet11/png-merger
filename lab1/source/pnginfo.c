#include <stdlib.h>
#include <string.h> 
#include "lab_png.h"

int is_png(U8 *buf, size_t n) {
	U8 compare[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
	return memcmp(buf, compare, n) == 0;
}

int pnginfo(const char *buf) {
	FILE *img = fopen(buf, "r");
	char content;

	U8 header[8];
	fread(header, 8, 1, img);
	int isPNG = is_png(header, 8);
	if (isPNG != 1) {
		printf("It is not a PNG\n");
		return 0;
	}
	printf("It is a PNG\n");
	/*
	while(1) {
		content = fgetc(img);

		if(feof(img)) break;

		printf("%c", content);	
	}
	*/
	fclose(img);

	return 0;
}

int main(int argc, char **argv) {
	pnginfo(argv[1]);
	return 0;
}