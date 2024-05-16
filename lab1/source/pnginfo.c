#include <stdlib.h>
#include "../starter/png_util/lab_png.h"

int pnginfo(const char *buf) {
	FILE *img = fopen(buf, "r");
	char content;

	while(1) {
		content = fgetc(img);

		if(feof(img)) break;

		printf("%c", content);	
	}

	fclose(img);

	return 0;
}

int main(const char *buf) {
  pnginfo(buf);
  return 0;
}