#include <stdlib.h>
#include <arpa/inet.h>
#include "lab_png.h"
#include "crc.h"

int is_png(U8 *buf, size_t n) {
	U8 compare[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

	for(int i=0; i<n; i++) 
		if(buf[i] != compare[i]) return 1;

	return 0;
}

int get_png_data_IHDR(struct data_IHDR *out, FILE *fp, long offset, int whence) {
	fseek(fp, offset, whence); /*shift file pointer to beginning of IHDR chunk*/
	U32 bufInt[sizeof(U32)];
	U8 bufChar[sizeof(U8)];

	fread(bufInt, sizeof(U32), 1, fp);
	out->width = ntohl(*bufInt);
	fread(bufInt, sizeof(U32), 1, fp);
	out->height = ntohl(*bufInt);

	fread(bufChar, sizeof(U8), 1, fp);
	out->bit_depth = *bufChar;

	fread(bufChar, sizeof(U8), 1, fp);
	out->color_type = *bufChar;

	fread(bufChar, sizeof(U8), 1, fp);
	out->compression = *bufChar;

	fread(bufChar, sizeof(U8), 1, fp);
	out->filter = *bufChar;

	fread(bufChar, sizeof(U8), 1, fp);
	out->interlace = *bufChar;

	return 0;
}

int pnginfo(const char *buf) {
	FILE *img = fopen(buf, "rb");
	U8 header[PNG_SIG_SIZE];
	fread(header, 1, PNG_SIG_SIZE, img);
	int isPNG = is_png(header, PNG_SIG_SIZE);
	printf("%s: ", buf);

	if (isPNG == 0) {
		/*printf("%ld\n", sizeof(simple_PNG_p));
		printf("%ld\n", sizeof(chunk_p));
		printf("%ld\n", sizeof(data_IHDR_p));*/
		simple_PNG_p png = malloc(sizeof(chunk_p)*3); /*initialize simple png struct*/
		if (png == NULL) {
            perror("Failed to allocate memory for png");
            fclose(img);
            return -1;
        }

		chunk_p IHDR_c = malloc(sizeof(chunk_p)+12); /*initialize IHDR chunk*/
		if (IHDR_c == NULL) {
            perror("Failed to allocate memory for IHDR_c");
            free(png);
            fclose(img);
            return -1;
        }
		chunk_p IDAT_c = malloc(sizeof(chunk_p)+12); /*initialize IDAT chunk*/
		if (IDAT_c == NULL) {
            perror("Failed to allocate memory for IDAT_c");
            free(png);
			free(IHDR_c);
            fclose(img);
            return -1;
        }
		chunk_p IEND_c = malloc(sizeof(chunk_p)+12); /*initialize IEND chunk*/
		if (IEND_c == NULL) {
            perror("Failed to allocate memory for IEND_c");
            free(png);
			free(IHDR_c);
			free(IDAT_c);
            fclose(img);
            return -1;
        }

		png->p_IHDR = IHDR_c;
		data_IHDR_p IHDR_d = malloc(DATA_IHDR_SIZE); /*initialize IHDR data struct*/
		if (IHDR_d == NULL) {
            perror("Failed to allocate memory for IHDR_d");
            free(IHDR_c);
			free(IDAT_c);
			free(IEND_c);
            free(png);
            fclose(img);
            return -1;
        }

		/*Read IHDR chunk data length value*/
		U32 bufInt[CHUNK_LEN_SIZE];
		fread(bufInt, 1, CHUNK_LEN_SIZE, img);
		IHDR_c->length = ntohl(*bufInt);
		/*printf("%d\n", IHDR_c->length);*/

		/*Read IHDR chunk type code*/
		U8 bufChar[CHUNK_TYPE_SIZE];
		fread(bufChar, 1, CHUNK_TYPE_SIZE, img);
		for(int i=0; i<CHUNK_TYPE_SIZE; i++) {
			/*printf("%c", bufChar[i]);
			printf("\n")*/
			IHDR_c->type[i] = bufChar[i];
		}
		U8 *data = malloc(IHDR_c->length);
		fread(data, 1, IDAT_c->length, img);

		get_png_data_IHDR(IHDR_d, img, -IHDR_c->length, SEEK_CUR);
		/*printf("%ld\n", sizeof(IHDR_d));*/
		IHDR_c->p_data = (U8 *)IHDR_d;
		

		/*Read IHDR CRC*/
		fread(bufInt, 1, CHUNK_CRC_SIZE, img);
		IHDR_c->crc = ntohl(*bufInt);

		int crcLen = 4 + IHDR_c->length;
		U8 *crcBuf = malloc(crcLen);

		for(int i=0; i<4; i++)
			crcBuf[i] = IHDR_c->type[i];
		
		for(int i=4; i<crcLen; i++)
			crcBuf[i] = data[i-4];

		for(int i=0; i<IHDR_c->length; i++)
			printf("%02X ", data[i]);
		printf("\n");
		for(int i=0; i<crcLen; i++)
			printf("%02X ", crcBuf[i]);
		printf("\n");

		printf("%02X\n", IHDR_c->crc);
		U32 crc_calc = crc(crcBuf, crcLen); /*TODO: determine input for crc function*/
		printf("%02X\n", crc_calc);
		/*if(IHDR_c->crc != crc_calc) {
			printf("%d x %d\n", IHDR_d->width, IHDR_d->height);
			printf("IHDR CRC error: computed %02X, expected %02X", crc_calc, IHDR_c->crc);
			return 1;
		}*/

		/*Read IDAT chunk data length value*/
		fread(bufInt, 1, CHUNK_LEN_SIZE, img);
		IDAT_c->length = ntohl(*bufInt);

		/*Read IDAT chunk type code*/
		fread(bufChar, 1, CHUNK_TYPE_SIZE, img);
		for(int i=0; i<CHUNK_TYPE_SIZE; i++) {
			/*printf("%c", bufChar[i]);
			printf("\n");*/
			IDAT_c->type[i] = bufChar[i];
		}

		/*Read IDAT data segment*/
		data = realloc(data, IDAT_c->length);
		fread(bufChar, 1, 1, img);
		fseek(img, -1, SEEK_CUR); /*first byte of data not being read by next fread call*/
		fread(data, 1, IDAT_c->length, img);
		*data = ntohl(*data);
		IDAT_c->p_data = data;
		IDAT_c->p_data[0] = *bufChar;

		/*Read IDAT CRC*/
		fread(bufInt, 1, CHUNK_CRC_SIZE, img);
		IDAT_c->crc = ntohl(*bufInt);

		crcLen = 4 + IDAT_c->length;
		crcBuf = realloc(crcBuf, crcLen);
		for(int i=0; i<4; i++)
			crcBuf[i] = IDAT_c->type[i];
		
		for(int i=4; i<crcLen; i++)
			crcBuf[i] = IDAT_c->p_data[i-4];

		printf("\n%02X\n", IDAT_c->crc);
		crc_calc = crc(crcBuf, crcLen); /*TODO: determine input for crc function*/
		printf("%02X\n", crc_calc);
		if(IDAT_c->crc != crc_calc) {
			printf("%d x %d\n", IHDR_d->width, IHDR_d->height);
			printf("IDAT CRC error: computed %02X, expected %02X", crc_calc, IDAT_c->crc);
			return 1;
		}

		/*Read IEND chunk data length value*/
		fread(bufInt, 1, CHUNK_LEN_SIZE, img);
		IEND_c->length = ntohl(*bufInt);
		/*printf("%d\n", IHDR_c->length);*/

		/*Read IEND chunk type code*/
		fread(bufChar, 1, CHUNK_TYPE_SIZE, img);
		for(int i=0; i<CHUNK_TYPE_SIZE; i++) {
			/*printf("%c", bufChar[i]);
			printf("\n");*/
			IEND_c->type[i] = bufChar[i];
		}

		/*Read IEND data segment*/
		data = realloc(data, IEND_c->length+1);
		fread(data, IEND_c->length, 1, img);
		*data = ntohl(*data);
		IEND_c->p_data = data;

		/*Read IEND CRC*/
		fread(bufInt, 1, CHUNK_CRC_SIZE, img);
		IEND_c->crc = ntohl(*bufInt);
		printf("%02X\n", IEND_c->crc);
		crc_calc = crc(IEND_c->type, CHUNK_TYPE_SIZE); /*TODO: determine input for crc function*/
		printf("%02X\n", crc_calc);
		if(IEND_c->crc != crc_calc) {
			printf("%d x %d\n", IHDR_d->width, IHDR_d->height);
			printf("IEND CRC error: computed %02X, expected %02X", crc_calc, IEND_c->crc);
			return 1;
		}

		printf("%d x %d\n", IHDR_d->width, IHDR_d->height);
		/*printf("%d %d %d %d %d\n", IHDR_d->bit_depth, IHDR_d->color_type, IHDR_d->compression, IHDR_d->filter, IHDR_d->interlace);*/

		fclose(img);
		free(data);
		free(crcBuf);
		free(IHDR_d);
		free(IEND_c);
		free(IDAT_c);
		free(IHDR_c);	
		free(png);

		return 0;
	}

	fclose(img);
	printf("Not a PNG\n");

	return 0;
}

int main(int argc, char **argv) {
	pnginfo(argv[1]);

	return 0;
}