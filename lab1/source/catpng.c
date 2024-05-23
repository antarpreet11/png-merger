#include "catpng.h"
#include "lab_png.h"

int catpng(char **buf) {

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
    simple_PNG_p pngOut = malloc(sizeof(chunk_p)*3);
    simple_PNG_p pngIn = pnginfo(argv[1], pngOut);
    printf("%d ", pngOut->p_IDAT->type[0]);
    printf("%d\n", pngIn->p_IDAT->type[0]);

    printf("\n%s", pngIn->p_IHDR->p_data);
    printf("%02X\n", pngOut->p_IDAT->crc);
    printf("%02X\n", pngIn->p_IDAT->crc);
    
    char **validPNGs = calloc(argc-1, sizeof *argv);
    int pngCount = getValidPNGs(argc-1, argv, validPNGs);

    free(pngOut->p_IDAT);
    free(pngOut);
    free(pngIn->p_IHDR->p_data);
    free(pngIn->p_IHDR);
    free(pngIn);
    free(validPNGs);
    return 0;
}