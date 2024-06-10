#include <arpa/inet.h>
#include <string.h>
#include "lab_png.h"
#include "crc.h"

int is_png(U8 *buf, size_t n) {
	U8 compare[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

	for(int i=0; i<n; i++) 
		if(buf[i] != compare[i]) return 1;

	return 0;
}

int get_png_data_IHDR(struct data_IHDR *out, char *dp, long offset, int whence) {
	U32 bufInt[sizeof(U32)];
	U8 bufChar[sizeof(U8)];
	size_t sz = sizeof(U32);

	//fread(bufInt, sizeof(U32), 1, fp);
	memcpy(bufInt, dp, sz);
	dp += sz;
	out->width = *bufInt;
	//fread(bufInt, sizeof(U32), 1, fp);
	memcpy(bufInt, dp, sz);
	dp += sz;
	out->height = *bufInt;

	//fread(bufChar, sizeof(U8), 1, fp);
	sz = sizeof(U8);
	memcpy(bufChar, dp, sz);
	dp += sz;
	out->bit_depth = *bufChar;

	//fread(bufChar, sizeof(U8), 1, fp);
	memcpy(bufChar, dp, sz);
	dp += sz;
	out->color_type = *bufChar;

	//fread(bufChar, sizeof(U8), 1, fp);
	memcpy(bufChar, dp, sz);
	dp += sz;
	out->compression = *bufChar;

	//fread(bufChar, sizeof(U8), 1, fp);
	memcpy(bufChar, dp, sz);
	dp += sz;
	out->filter = *bufChar;

	//fread(bufChar, sizeof(U8), 1, fp);
	memcpy(bufChar, dp, sz);
	dp += sz;
	out->interlace = *bufChar;

	return 0;
}

simple_PNG_p pnginfo(char *buf) {
	//FILE *img = fopen(buf, "rb");
	U8 header[PNG_SIG_SIZE];
	//fread(header, 1, PNG_SIG_SIZE, img);
	memcpy(header, buf, PNG_SIG_SIZE);
	buf += PNG_SIG_SIZE;
	int isPNG = is_png(header, PNG_SIG_SIZE);

	if (isPNG == 0) {
		simple_PNG_p png = malloc(sizeof(chunk_p)*3); /*initialize simple png struct*/
		if (png == NULL) {
            perror("Failed to allocate memory for png");
            //fclose(img);
            return NULL;
        }

		chunk_p IHDR_c = malloc(sizeof(chunk_p)+12); /*initialize IHDR chunk*/
		if (IHDR_c == NULL) {
            perror("Failed to allocate memory for IHDR_c");
            free(png);
            //fclose(img);
            return NULL;
        }

		chunk_p IDAT_c = malloc(sizeof(chunk_p)+12); /*initialize IDAT chunk*/
		if (IDAT_c == NULL) {
            perror("Failed to allocate memory for IDAT_c");
            free(png);
			free(IHDR_c);
            //fclose(img);
            return NULL;
        }

		chunk_p IEND_c = malloc(sizeof(chunk_p)+12); /*initialize IEND chunk*/
		if (IEND_c == NULL) {
            perror("Failed to allocate memory for IEND_c");
            free(png);
			free(IHDR_c);
			free(IDAT_c);
            //fclose(img);
            return NULL;
        }

		data_IHDR_p IHDR_d = malloc(DATA_IHDR_SIZE); /*initialize IHDR data struct*/
		if (IHDR_d == NULL) {
            perror("Failed to allocate memory for IHDR_d");
            free(IHDR_c);
			free(IDAT_c);
			free(IEND_c);
            free(png);
            //fclose(img);
            return NULL;
        }

		/*Read IHDR chunk data length value*/
		U32 bufInt[CHUNK_LEN_SIZE];
		//fread(bufInt, 1, CHUNK_LEN_SIZE, img);
		IHDR_c->length = ntohl(*bufInt);

		/*Read IHDR chunk type code*/
		U8 bufChar[CHUNK_TYPE_SIZE];
		//fread(bufChar, 1, CHUNK_TYPE_SIZE, img);
		memcpy(bufChar, buf, CHUNK_TYPE_SIZE);
		buf += CHUNK_TYPE_SIZE;
		for(int i=0; i<CHUNK_TYPE_SIZE; i++)
			IHDR_c->type[i] = bufChar[i];

		get_png_data_IHDR(IHDR_d, buf, IHDR_c->length, SEEK_CUR);
		IHDR_c->p_data = (U8 *)IHDR_d;

		/*Read IHDR CRC*/
		//fread(bufInt, 1, CHUNK_CRC_SIZE, img);
		memcpy(bufInt, buf, CHUNK_CRC_SIZE);
		buf += CHUNK_CRC_SIZE;
		IHDR_c->crc = ntohl(*bufInt);

		/*Fill crc calc buffer*/
		int crcLen = CHUNK_TYPE_SIZE + IHDR_c->length;
		U8 *crcBuf = malloc(crcLen);
		for(int i=0; i<CHUNK_TYPE_SIZE; i++)
			crcBuf[i] = IHDR_c->type[i];
		for(int i=CHUNK_TYPE_SIZE; i<crcLen; i++)
			crcBuf[i] = (IHDR_c->p_data[i-4]);
		/*Calculate and compare IHDR crc*/
		U32 crc_calc = crc(crcBuf, crcLen);
		if(IHDR_c->crc != crc_calc) 
			printf("IHDR CRC error: computed %02X, expected %02X\n", crc_calc, IHDR_c->crc);

		png->p_IHDR = IHDR_c;

		/*Read IDAT chunk data length value*/
		//fread(bufInt, 1, CHUNK_LEN_SIZE, img);
		memcpy(bufInt, buf, CHUNK_LEN_SIZE);
		buf += CHUNK_LEN_SIZE;
		IDAT_c->length = ntohl(*bufInt);

		/*Read IDAT chunk type code*/
		//fread(bufChar, 1, CHUNK_TYPE_SIZE, img);
		memcpy(bufChar, buf, CHUNK_TYPE_SIZE);
		buf += CHUNK_TYPE_SIZE;
		for(int i=0; i<CHUNK_TYPE_SIZE; i++)
			IDAT_c->type[i] = bufChar[i];

		/*Read IDAT data segment*/
		U8 *data_id = malloc(IDAT_c->length);
		//fread(data_id, 1, IDAT_c->length, img);
		memcpy(data_id, buf, IDAT_c->length);
		buf += IDAT_c->length;
		IDAT_c->p_data = data_id;

		/*Read IDAT crc*/
		//fread(bufInt, 1, CHUNK_CRC_SIZE, img);
		memcpy(bufInt, buf, CHUNK_CRC_SIZE);
		buf += CHUNK_CRC_SIZE;
		IDAT_c->crc = ntohl(*bufInt);

		/*Fill crc calc buffer*/
		crcLen = 4 + IDAT_c->length;
		crcBuf = realloc(crcBuf, crcLen);
		for(int i=0; i<4; i++)
			crcBuf[i] = IDAT_c->type[i];
		for(int i=4; i<crcLen; i++)
			crcBuf[i] = IDAT_c->p_data[i-4];
		/*Calculate and compare IDAT crc*/
		crc_calc = crc(crcBuf, crcLen); 
		if(IDAT_c->crc != crc_calc)
			printf("IDAT CRC error: computed %02X, expected %02X\n", crc_calc, IDAT_c->crc);

		png->p_IDAT = IDAT_c;

		/*Read IEND chunk data length value*/
		//fread(bufInt, 1, CHUNK_LEN_SIZE, img);
		memcpy(bufInt, buf, CHUNK_LEN_SIZE);
		buf += CHUNK_LEN_SIZE;
		IEND_c->length = ntohl(*bufInt);

		/*Read IEND chunk type code*/
		//fread(bufChar, 1, CHUNK_TYPE_SIZE, img);
		memcpy(bufChar, buf, CHUNK_TYPE_SIZE);
		buf += CHUNK_TYPE_SIZE;
		for(int i=0; i<CHUNK_TYPE_SIZE; i++)
			IEND_c->type[i] = bufChar[i];

		/*Read IEND data segment*/
		U8 *data_ie = malloc(IEND_c->length+1);
		//fread(data_ie, IEND_c->length, 1, img);
		memcpy(data_ie, buf, IEND_c->length);
		buf += IEND_c->length;
		*data_ie = ntohl(*data_ie);
		IEND_c->p_data = data_ie;

		/*Read IEND crc*/
		//fread(bufInt, 1, CHUNK_CRC_SIZE, img);
		memcpy(bufInt, buf, CHUNK_CRC_SIZE);
		buf += CHUNK_CRC_SIZE;
		IEND_c->crc = ntohl(*bufInt);
		/*Calculate and compare crc*/
		crc_calc = crc(IEND_c->type, CHUNK_TYPE_SIZE);
		if(IEND_c->crc != crc_calc) {
			printf("IEND CRC error: computed %02X, expected %02X\n", crc_calc, IEND_c->crc);
		}

		png->p_IEND = IEND_c;

		free(crcBuf);
		//fclose(img);

		return png;
	}

	//fclose(img);
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