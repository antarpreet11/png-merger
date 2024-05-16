#include <stdlib.h>
#include "lab_png.h"

int is_png(U8 *buf, size_t n) {
	U8 compare[8] = {'89', '50', '4E', '47', '0D', '0A', '1A', '0A'};
	printf("%s\n", buf);
	return memcmp(buf, compare, n);
}

int pnginfo(const char *buf) {
	FILE *img = fopen(buf, "r");
	char content;

	U8 header[8];
	fread(header, 8, 1, img);
	printf("%s\n", header);
	int isPNG = is_png(header, 8);
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