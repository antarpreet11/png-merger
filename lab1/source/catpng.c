#include "catpng.h"
#include "crc.h"
#include "zutil.h"

int get_png_height(struct data_IHDR *buf) {
    return buf->height;
}

int get_png_width(struct data_IHDR *buf) {
    return buf->width;
}

simple_PNG_p catpng(char **buf, int count) {
    simple_PNG_p pngIn[count];
    simple_PNG_p pngOut = malloc(sizeof(chunk_p)*3);

    for(int i=0; i<count; i++)
        pngIn[i] = pnginfo(buf[i]);

    /*inflate IDAT data segments
    mcpy inflated data segments into buffer
    deflate buffer
    update IDAT data length, IHDR height, IDAT and IHDR crc values
    copy chunks from one of original png to write to new file
    fopen blank file, fwrite each struct data element to file */

    U8 *inf_IDAT_data = malloc(1);
    U64 inf_dataLengths[count];
    U64 inf_totalDataLength = 0;
    int newPNGheight, pngWidth, pngHeight = 0;
    int ret = 0;

    for(int i=0; i<count; i++) {
printf(" data length: %d\n", pngIn[i]->p_IDAT->length);
        pngHeight = get_png_height((data_IHDR_p)pngIn[i]->p_IHDR->p_data);
        pngWidth = get_png_width((data_IHDR_p)pngIn[i]->p_IHDR->p_data);
        inf_IDAT_data = realloc(inf_IDAT_data, inf_totalDataLength+(4*pngWidth+1)*pngHeight);

        ret = mem_inf(inf_IDAT_data+inf_totalDataLength, &inf_dataLengths[i], pngIn[i]->p_IDAT->p_data, (U64)pngIn[i]->p_IDAT->length);

        newPNGheight += pngHeight;
        inf_totalDataLength += inf_dataLengths[i];
    }
printf("new datalength: %ld\n", inf_dataLengths[0]);
printf("new height: %d\n", newPNGheight);

    U8 *def_IDAT_data = malloc(inf_totalDataLength);
    U64 def_totalDataLength = 0;
    ret = mem_def(def_IDAT_data, &def_totalDataLength, inf_IDAT_data, inf_totalDataLength, Z_DEFAULT_COMPRESSION);
printf("deflated length: %ld\n", def_totalDataLength);
    def_IDAT_data = realloc(def_IDAT_data, def_totalDataLength);
printf("deflated length: %ld\n", def_totalDataLength);

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

    simple_PNG_p pngOut = catpng(validPNGs, pngCount);

    free(pngOut);
    free(validPNGs);
    return 0;
}