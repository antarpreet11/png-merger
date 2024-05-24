#include <arpa/inet.h>
#include "lab_png.h"
#include "crc.h"
#include "catpng.h"

void write_chunk(FILE *fp, chunk_p new_chunk) {
    new_chunk->length = htonl(new_chunk->length);

    fwrite(&(new_chunk->length), sizeof(U32), 1, fp);
    fwrite(new_chunk->type, sizeof(U8), CHUNK_TYPE_SIZE, fp);
    if (new_chunk->p_data != NULL && new_chunk->length > 0) {
        fwrite(new_chunk->p_data, sizeof(U8), new_chunk->length, fp);
    }

    unsigned long crc_set = crc(new_chunk->type, CHUNK_TYPE_SIZE);
    if (new_chunk->p_data != NULL && new_chunk->length > 0) {
        crc_set = crc(new_chunk->p_data, new_chunk->length);
    }
    new_chunk->crc = htonl(crc_set); // Convert to network byte order
    fwrite(&(new_chunk->crc), sizeof(U32), 1, fp);
}

void write_png_file(const char *filename, simple_PNG_p new_png) {
    FILE *fp = fopen(filename, "wb+");
    if (fp == NULL) {
        fprintf(stderr, "write_png_file: Could not open file %s for writing\n", filename);
        return;
    }

    U8 header[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

    /* Write PNG signature */
    fwrite(header, 1, PNG_SIG_SIZE, fp);

    /* Write IHDR chunk */
    write_chunk(fp, new_png->p_IHDR);

    /* Write IDAT chunk */
    write_chunk(fp, new_png->p_IDAT);

    /* Write IEND chunk */
    write_chunk(fp, new_png->p_IEND);

    fclose(fp);  
}

int catpng(char **buf, int count) {
    simple_PNG_p pngIn[count];

    for(int i=0; i<count; i++) {
        pngIn[i] = pnginfo(buf[i]);
    }

    if(pngIn != NULL) {
        for(int i=0; i<count; i++) {
            free(pngIn[i]->p_IEND->p_data);
            free(pngIn[i]->p_IEND);
            free(pngIn[i]->p_IDAT->p_data);
            free(pngIn[i]->p_IDAT);
            free(pngIn[i]->p_IHDR->p_data);
            free(pngIn[i]->p_IHDR);
            free(pngIn[i]);
        }
    }

    return 0;
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

    catpng(validPNGs, pngCount);

    free(validPNGs);
    return 0;
}