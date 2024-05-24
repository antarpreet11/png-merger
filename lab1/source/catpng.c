#include "catpng.h"
#include "lab_png.h"

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