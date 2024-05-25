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
	/*fseek(fp, offset, whence); /*shift file pointer to beginning of IHDR chunk*/
	U32 bufInt[sizeof(U32)];
	U8 bufChar[sizeof(U8)];

	fread(bufInt, sizeof(U32), 1, fp);
	out->width = (*bufInt); /*byte order conversion messes with crc calculation*/
	fread(bufInt, sizeof(U32), 1, fp);
	out->height = (*bufInt);

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

simple_PNG_p pnginfo(const char *buf) {
	FILE *img = fopen(buf, "rb");
	U8 header[PNG_SIG_SIZE];
	fread(header, 1, PNG_SIG_SIZE, img);
	int isPNG = is_png(header, PNG_SIG_SIZE);

	if (isPNG == 0) {
		/*printf("%ld\n", sizeof(simple_PNG_p));
		printf("%ld\n", sizeof(chunk_p));
		printf("%ld\n", sizeof(data_IHDR_p));*/
		simple_PNG_p png = malloc(sizeof(chunk_p)*3); /*initialize simple png struct*/
		if (png == NULL) {
            perror("Failed to allocate memory for png");
            fclose(img);
            return NULL;
        }

		chunk_p IHDR_c = malloc(sizeof(chunk_p)+12); /*initialize IHDR chunk*/
		if (IHDR_c == NULL) {
            perror("Failed to allocate memory for IHDR_c");
            free(png);
            fclose(img);
            return NULL;
        }
		chunk_p IDAT_c = malloc(sizeof(chunk_p)+12); /*initialize IDAT chunk*/
		if (IDAT_c == NULL) {
            perror("Failed to allocate memory for IDAT_c");
            free(png);
			free(IHDR_c);
            fclose(img);
            return NULL;
        }
		chunk_p IEND_c = malloc(sizeof(chunk_p)+12); /*initialize IEND chunk*/
		if (IEND_c == NULL) {
            perror("Failed to allocate memory for IEND_c");
            free(png);
			free(IHDR_c);
			free(IDAT_c);
            fclose(img);
            return NULL;
        }

		data_IHDR_p IHDR_d = malloc(DATA_IHDR_SIZE); /*initialize IHDR data struct*/
		if (IHDR_d == NULL) {
            perror("Failed to allocate memory for IHDR_d");
            free(IHDR_c);
			free(IDAT_c);
			free(IEND_c);
            free(png);
            fclose(img);
            return NULL;
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
		/*U8 *data = malloc(IHDR_c->length);
		/*fread(data, 1, IDAT_c->length, img);*/

		get_png_data_IHDR(IHDR_d, img, -IHDR_c->length, SEEK_CUR);
		/*printf("%ld\n", sizeof(IHDR_d));*/
		IHDR_c->p_data = (U8 *)IHDR_d;

		/*Read IHDR CRC*/
		fread(bufInt, 1, CHUNK_CRC_SIZE, img);
		IHDR_c->crc = ntohl(*bufInt);

		int crcLen = CHUNK_TYPE_SIZE + IHDR_c->length;
		U8 *crcBuf = malloc(crcLen);

		for(int i=0; i<CHUNK_TYPE_SIZE; i++)
			crcBuf[i] = IHDR_c->type[i];

		for(int i=CHUNK_TYPE_SIZE; i<crcLen; i++)
			crcBuf[i] = (IHDR_c->p_data[i-4]);
//printf("%d\n", IHDR_d->width);
		IHDR_d->width = (IHDR_d->width); /*byte order conversion mismatches hex dump*/
		IHDR_d->height = (IHDR_d->height);

//printf("%d\n", IHDR_d->width);
		IHDR_c->p_data = (U8 *)IHDR_d;
		/*printf("%02X\n", IHDR_c->crc);*/
		U32 crc_calc = crc(crcBuf, crcLen);
		/*printf("%02X\n", crc_calc);*/
		if(IHDR_c->crc != crc_calc) {
			printf("IHDR CRC error: computed %02X, expected %02X\n", crc_calc, IHDR_c->crc);
		}

		png->p_IHDR = IHDR_c;

		/*Read IDAT chunk data length value*/
		fread(bufInt, 1, CHUNK_LEN_SIZE, img);
		IDAT_c->length = ntohl(*bufInt);
//printf("data length: %d\n", IDAT_c->length);

		/*Read IDAT chunk type code*/
		fread(bufChar, 1, CHUNK_TYPE_SIZE, img);
		for(int i=0; i<CHUNK_TYPE_SIZE; i++) {
			/*printf("%c", bufChar[i]);
			printf("\n");*/
			IDAT_c->type[i] = bufChar[i];
		}

		/*Read IDAT data segment*/
		U8 *data_id = malloc(IDAT_c->length);
		//fread(bufChar, 1, 1, img);
		//fseek(img, -1, SEEK_CUR); /*first byte of data not being read by next fread call*/
		fread(data_id, 1, IDAT_c->length, img);
		//*data_id = ntohl(*data_id);
		IDAT_c->p_data = data_id;
		//IDAT_c->p_data[0] = *bufChar;

//printf("%d\n", IDAT_c->p_data[0]);

		/*Read IDAT CRC*/
		fread(bufInt, 1, CHUNK_CRC_SIZE, img);
		IDAT_c->crc = ntohl(*bufInt);

		crcLen = 4 + IDAT_c->length;
		crcBuf = realloc(crcBuf, crcLen);
		for(int i=0; i<4; i++)
			crcBuf[i] = IDAT_c->type[i];
		
		for(int i=4; i<crcLen; i++)
			crcBuf[i] = IDAT_c->p_data[i-4];

		/*printf("\n%02X\n", IDAT_c->crc);*/
		crc_calc = crc(crcBuf, crcLen); /*TODO: determine input for crc function*/
		/*printf("%02X\n", crc_calc);*/
		if(IDAT_c->crc != crc_calc) {
			printf("IDAT CRC error: computed %02X, expected %02X\n", crc_calc, IDAT_c->crc);
		}

		png->p_IDAT = IDAT_c;

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
//printf("%d\n", IDAT_c->p_data[0]);
		/*Read IEND data segment*/
		U8 *data_ie = malloc(IEND_c->length+1);
		fread(data_ie, IEND_c->length, 1, img);
		*data_ie = ntohl(*data_ie);
		IEND_c->p_data = data_ie;

		/*Read IEND CRC*/
		fread(bufInt, 1, CHUNK_CRC_SIZE, img);
		IEND_c->crc = ntohl(*bufInt);
		/*printf("%02X\n", IEND_c->crc);*/
		crc_calc = crc(IEND_c->type, CHUNK_TYPE_SIZE); /*TODO: determine input for crc function*/
		/*printf("%02X\n", crc_calc);*/
		if(IEND_c->crc != crc_calc) {
			printf("IEND CRC error: computed %02X, expected %02X\n", crc_calc, IEND_c->crc);
		}

		png->p_IEND = IEND_c;

		printf("%s: %d x %d\n", buf, ntohl(IHDR_d->width), ntohl(IHDR_d->height));
		/*printf("%d %d %d %d %d\n", IHDR_d->bit_depth, IHDR_d->color_type, IHDR_d->compression, IHDR_d->filter, IHDR_d->interlace);*/

//printf("%d\n", IDAT_c->p_data[0]);
//printf("%d\n", IEND_c->p_data[0]);

/*for(int i=0; i<13; i++)
        printf("%02X ",png->p_IHDR->p_data[i]);
printf("\n");*/

		//free(data);

		free(crcBuf);
		//free(IHDR_d);
		//free(IEND_c);
		//free(IDAT_c);
		//free(IHDR_c);	
		//free(png);
		fclose(img);

		return png;
	}

	fclose(img);
	printf("Not a PNG\n");

	return NULL;
}

/*int main(int argc, char **argv) {
	simple_PNG_p png = pnginfo(argv[1]);
	
	printf("%02x ", png->p_IHDR->length);
	for(int i=0; i<CHUNK_TYPE_SIZE; i++)
		printf("%02x ", png->p_IHDR->type[i]);
	for(int i=0; i<DATA_IHDR_SIZE; i++)
		printf("%02x ", png->p_IHDR->p_data[i]);
	printf("%02x ", png->p_IHDR->crc);


	printf("\n%02x ", png->p_IDAT->length);
	for(int i=0; i<CHUNK_TYPE_SIZE; i++)
		printf("%02x ", png->p_IDAT->type[i]);
	for(int i=0; i<png->p_IDAT->length; i++)
		printf("%02x ", png->p_IDAT->p_data[i]);
	printf("%02x ", png->p_IDAT->crc);

	printf("\n%02x ", png->p_IEND->length);
	for(int i=0; i<CHUNK_TYPE_SIZE; i++)
		printf("%02x ", png->p_IEND->type[i]);
	for(int i=0; i<png->p_IEND->length; i++)
		printf("%02x ", png->p_IEND->p_data[i]);
	printf("%02x ", png->p_IEND->crc);

	return 0;
}*/