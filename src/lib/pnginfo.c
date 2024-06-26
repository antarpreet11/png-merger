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

	memcpy(bufInt, dp, sz);
	dp += sz;
	out->width = *bufInt;
	memcpy(bufInt, dp, sz);
	dp += sz;
	out->height = *bufInt;

	sz = sizeof(U8);
	memcpy(bufChar, dp, sz);
	dp += sz;
	out->bit_depth = *bufChar;

	memcpy(bufChar, dp, sz);
	dp += sz;
	out->color_type = *bufChar;

	memcpy(bufChar, dp, sz);
	dp += sz;
	out->compression = *bufChar;

	memcpy(bufChar, dp, sz);
	dp += sz;
	out->filter = *bufChar;

	memcpy(bufChar, dp, sz);
	dp += sz;
	out->interlace = *bufChar;

	return 0;
}

simple_PNG_p pnginfo(char *buf) {
	U8 header[PNG_SIG_SIZE];
	memcpy(header, buf, PNG_SIG_SIZE);
	buf += PNG_SIG_SIZE;
	int isPNG = is_png(header, PNG_SIG_SIZE);

	if (isPNG == 0) {
		simple_PNG_p png = malloc(sizeof(chunk_p)*3); /*initialize simple png struct*/
		if (png == NULL) {
            perror("Failed to allocate memory for png");
            return NULL;
        }

		chunk_p IHDR_c = malloc(sizeof(chunk_p)+12); /*initialize IHDR chunk*/
		if (IHDR_c == NULL) {
            perror("Failed to allocate memory for IHDR_c");
            free(png);
            return NULL;
        }

		chunk_p IDAT_c = malloc(sizeof(chunk_p)+12); /*initialize IDAT chunk*/
		if (IDAT_c == NULL) {
            perror("Failed to allocate memory for IDAT_c");
            free(png);
			free(IHDR_c);
            return NULL;
        }

		chunk_p IEND_c = malloc(sizeof(chunk_p)+12); /*initialize IEND chunk*/
		if (IEND_c == NULL) {
            perror("Failed to allocate memory for IEND_c");
            free(png);
			free(IHDR_c);
			free(IDAT_c);
            return NULL;
        }

		data_IHDR_p IHDR_d = malloc(DATA_IHDR_SIZE); /*initialize IHDR data struct*/
		if (IHDR_d == NULL) {
            perror("Failed to allocate memory for IHDR_d");
            free(IHDR_c);
			free(IDAT_c);
			free(IEND_c);
            free(png);
            return NULL;
        }

		/*Read IHDR chunk data length value*/
		U32 bufInt[CHUNK_LEN_SIZE];
		memcpy(bufInt, buf, CHUNK_LEN_SIZE);
		buf += CHUNK_LEN_SIZE;
		IHDR_c->length = ntohl(*bufInt);

		/*Read IHDR chunk type code*/
		U8 bufChar[CHUNK_TYPE_SIZE];
		memcpy(bufChar, buf, CHUNK_TYPE_SIZE);
		buf += CHUNK_TYPE_SIZE;
		for(int i=0; i<CHUNK_TYPE_SIZE; i++)
			IHDR_c->type[i] = bufChar[i];

		get_png_data_IHDR(IHDR_d, buf, IHDR_c->length, SEEK_CUR);
		IHDR_c->p_data = (U8 *)IHDR_d;
		buf += (sizeof(U32)*2 + sizeof(U8)*5);

		/*Read IHDR CRC*/
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
		memcpy(bufInt, buf, CHUNK_LEN_SIZE);
		buf += CHUNK_LEN_SIZE;
		IDAT_c->length = ntohl(*bufInt);

		/*Read IDAT chunk type code*/
		memcpy(bufChar, buf, CHUNK_TYPE_SIZE);
		buf += CHUNK_TYPE_SIZE;
		for(int i=0; i<CHUNK_TYPE_SIZE; i++)
			IDAT_c->type[i] = bufChar[i];

		/*Read IDAT data segment*/
		U8 *data_id = malloc(IDAT_c->length);
		memcpy(data_id, buf, IDAT_c->length);
		buf += IDAT_c->length;
		IDAT_c->p_data = data_id;

		/*Read IDAT crc*/
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
		memcpy(bufInt, buf, CHUNK_LEN_SIZE);
		buf += CHUNK_LEN_SIZE;
		IEND_c->length = ntohl(*bufInt);

		/*Read IEND chunk type code*/
		memcpy(bufChar, buf, CHUNK_TYPE_SIZE);
		buf += CHUNK_TYPE_SIZE;
		for(int i=0; i<CHUNK_TYPE_SIZE; i++)
			IEND_c->type[i] = bufChar[i];

		/*Read IEND data segment*/
		U8 *data_ie = malloc(IEND_c->length+1);
		memcpy(data_ie, buf, IEND_c->length);
		buf += IEND_c->length;
		*data_ie = ntohl(*data_ie);
		IEND_c->p_data = data_ie;

		/*Read IEND crc*/
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

		return png;
	}

	printf("Not a PNG\n");

	return NULL;
}