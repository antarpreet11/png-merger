#include <arpa/inet.h>
#include "lab_png.h"
#include "crc.h"
#include "catpng.h"
#include "zutil.h"

int get_png_height(struct data_IHDR *buf) {
    return ntohl(buf->height);
}

void set_png_height(struct data_IHDR *buf, U32 h) {
    buf->height = (h);
}

int get_png_width(struct data_IHDR *buf) {
    return ntohl(buf->width);
}

void write_chunk(FILE *fp, chunk_p new_chunk) {
    if (new_chunk == NULL || new_chunk->p_data == NULL) {
        fprintf(stderr, "write_chunk: NULL chunk or chunk data\n");
        return;
    }

    U32 length = htonl(new_chunk->length);
    size_t written = 0;

    written = fwrite(&length, sizeof(U32), 1, fp);
    if (written != 1) {
        fprintf(stderr, "write_chunk: Failed to write chunk length\n");
        return;
    }

    written = fwrite(new_chunk->type, sizeof(U8), CHUNK_TYPE_SIZE, fp);
    if (written != CHUNK_TYPE_SIZE) {
        fprintf(stderr, "write_chunk: Failed to write chunk type\n");
        return;
    }

    written = fwrite(new_chunk->p_data, sizeof(U8), new_chunk->length, fp);
    if (written != new_chunk->length) {
        fprintf(stderr, "write_chunk: Failed to write chunk data\n");
        return;
    }

    U32 crc_value = htonl(new_chunk->crc);
    written = fwrite(&crc_value, sizeof(U32), 1, fp);
    if (written != 1) {
        fprintf(stderr, "write_chunk: Failed to write chunk CRC\n");
    }
}

void write_png_file(const char *filename, simple_PNG_p new_png) {
    if (new_png == NULL) {
        fprintf(stderr, "write_png_file: NULL PNG structure\n");
        return;
    }

    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "write_png_file: Could not open file %s for writing\n", filename);
        return;
    }

    U8 header[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

    /* Write PNG signature */
    size_t written = fwrite(header, sizeof(U8), PNG_SIG_SIZE, fp);
    if (written != PNG_SIG_SIZE) {
        fprintf(stderr, "write_png_file: Failed to write PNG header\n");
        fclose(fp);
        return;
    }

    /* Write IHDR chunk */
    if (new_png->p_IHDR == NULL) {
        fprintf(stderr, "write_png_file: IHDR chunk is NULL\n");
        fclose(fp);
        return;
    }
    write_chunk(fp, new_png->p_IHDR);

    /* Write IDAT chunk */
    if (new_png->p_IDAT == NULL) {
        fprintf(stderr, "write_png_file: IDAT chunk is NULL\n");
        fclose(fp);
        return;
    }
    write_chunk(fp, new_png->p_IDAT);

    /* Write IEND chunk */
    if (new_png->p_IEND == NULL) {
        fprintf(stderr, "write_png_file: IEND chunk is NULL\n");
        fclose(fp);
        return;
    }
    write_chunk(fp, new_png->p_IEND);

    fclose(fp);  
}

simple_PNG_p catpng(char **buf, int count) {
    simple_PNG_p pngIn[count];
    simple_PNG_p pngOut = malloc(sizeof(chunk_p)*3);

    for(int i=0; i<count; i++)
        pngIn[i] = pnginfo(buf[i]);

    /*inflate IDAT data segments
    mcpy inflated data segments into buffer
    deflate buffer
    copy chunks from one of original png to write to new file
    update IDAT data length, IHDR height, IDAT and IHDR crc values
    fopen blank file, fwrite each struct data element to file */

    U8 *inf_IDAT_data = malloc(1);
    U64 inf_dataLengths[count];
    U64 inf_totalDataLength = 0;
    int newPNGheight, pngWidth, pngHeight = 0;
    int ret = 0;
/*for(int i=0; i<36; i++)
printf("%02X", pngIn[0]->p_IDAT->p_data[i]);*/
    for(int i=0; i<count; i++) {
//printf(" data length: %d\n", pngIn[i]->p_IDAT->length);
        pngHeight = get_png_height((data_IHDR_p)pngIn[i]->p_IHDR->p_data);
        pngWidth = get_png_width((data_IHDR_p)pngIn[i]->p_IHDR->p_data);
        inf_IDAT_data = realloc(inf_IDAT_data, inf_totalDataLength+(4*pngWidth+1)*pngHeight);

        ret = mem_inf(inf_IDAT_data+inf_totalDataLength, &inf_dataLengths[i], pngIn[i]->p_IDAT->p_data, (U64)pngIn[i]->p_IDAT->length);

        newPNGheight += pngHeight;
        inf_totalDataLength += inf_dataLengths[i];
    }

    free(pngIn[0]->p_IDAT->p_data);
//printf("new datalength: %ld\n", inf_dataLengths[0]);
//printf("new height: %d\n", newPNGheight);

    U8 *def_IDAT_data = malloc(inf_totalDataLength);
    U64 def_totalDataLength = 0;
    ret = mem_def(def_IDAT_data, &def_totalDataLength, inf_IDAT_data, inf_totalDataLength, Z_DEFAULT_COMPRESSION);
//printf("deflated length: %ld\n", def_totalDataLength);
    def_IDAT_data = realloc(def_IDAT_data, def_totalDataLength);
/*for(int i=0; i<36; i++)
printf("%02X ", def_IDAT_data[i]);
printf("\n");
    /**def_IDAT_data = ntohl(*def_IDAT_data); /*if wrong, maybe try converting each chunk during concatenation
for(int i=0; i<36; i++)
printf("%02X ", def_IDAT_data[i]);
printf("\n");
printf("deflated length: %ld\n", def_totalDataLength);*/

    /*Write out IHDR*/
    pngOut->p_IHDR = pngIn[0]->p_IHDR;
    pngOut->p_IHDR->p_data = pngIn[0]->p_IHDR->p_data;

//printf("%d\n", get_png_height((data_IHDR_p)pngOut->p_IHDR->p_data));
    set_png_height((data_IHDR_p)pngOut->p_IHDR->p_data, newPNGheight);
//printf("%d\n", get_png_height((data_IHDR_p)pngOut->p_IHDR->p_data));

    int crcLen = CHUNK_TYPE_SIZE + DATA_IHDR_SIZE;//pngOut->p_IHDR->length;
    U8 *crcBuf = malloc(crcLen);

    for(int i=0; i<CHUNK_TYPE_SIZE; i++)
        crcBuf[i] = pngOut->p_IHDR->type[i];

    for(int i=CHUNK_TYPE_SIZE; i<crcLen; i++)
        crcBuf[i] = (pngOut->p_IHDR->p_data[i-4]);
/*for(int i=0;i<crcLen; i++)
printf("%02X ", crcBuf[i]);*/

    U32 crc_calc = crc(crcBuf, crcLen);
    pngOut->p_IHDR->crc = crc_calc;

    /*Write out IDAT*/
    pngOut->p_IDAT = pngIn[0]->p_IDAT;
    pngOut->p_IDAT->p_data = def_IDAT_data;
    pngOut->p_IDAT->length = def_totalDataLength;

    crcLen = 4 + pngOut->p_IDAT->length;
    crcBuf = realloc(crcBuf, crcLen);
    for(int i=0; i<4; i++)
        crcBuf[i] = pngOut->p_IDAT->type[i];
    
    for(int i=4; i<crcLen; i++)
        crcBuf[i] = pngOut->p_IDAT->p_data[i-4];

    crc_calc = crc(crcBuf, crcLen);
    pngOut->p_IDAT->crc = crc_calc;

    /*Write out IEND*/
    pngOut->p_IEND = pngIn[0]->p_IEND;
    pngOut->p_IEND->p_data = pngIn[0]->p_IEND->p_data;

    //free(pngIn[0]->p_IDAT->p_data);
    
    if(pngIn != NULL) {
        for(int i=1; i<count; i++) {
            free(pngIn[i]->p_IEND->p_data);
            free(pngIn[i]->p_IEND);
            free(pngIn[i]->p_IDAT->p_data);
            free(pngIn[i]->p_IDAT);
            free(pngIn[i]->p_IHDR->p_data);
            free(pngIn[i]->p_IHDR);
            free(pngIn[i]);
        }
    }

    free(inf_IDAT_data);
    //free(pngIn[0]->p_IDAT->p_data);
    free(pngIn[0]);
    free(crcBuf);
    return pngOut;
}

int getValidPNGs (int pathCount, char **paths, char **validPNGs) {
    int pngCount = 0;

    for(int i = 1; i<=pathCount; i++) {
        FILE *img = fopen(paths[i], "rb");
	    U8 header[PNG_SIG_SIZE];
	    fread(header, 1, PNG_SIG_SIZE, img);

        if(is_png(header, PNG_SIG_SIZE) == 0) {
            validPNGs[i-1] = paths[i];
            pngCount++;
        }

        fclose(img);
    }

    return pngCount;
}

int main(int argc, char **argv) {
    char **validPNGs = calloc(argc-1, sizeof *argv);
    int pngCount = getValidPNGs(argc-1, argv, validPNGs);

    if(pngCount > 0) {
        simple_PNG_p pngOut = catpng(validPNGs, pngCount);
/*for(int i=0;i<36;i++)
printf("%02x", pngOut->p_IDAT->p_data[i]);*/
        write_png_file("all.png", pngOut);

        free(pngOut->p_IEND->p_data);
        free(pngOut->p_IEND);
        free(pngOut->p_IDAT->p_data);
        free(pngOut->p_IDAT);
        free(pngOut->p_IHDR->p_data);
        free(pngOut->p_IHDR);
        free(pngOut);
    }
    free(validPNGs);
    return 0;
} 