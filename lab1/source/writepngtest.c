#include "catpng.h"
#include "lab_png.h"
#include <stdio.h> 

int main(int argc, char **argv) {
    simple_PNG_p pngin = pnginfo(argv[1]);
    printf("Png info extracted\n");
    write_png_file("all.png", pngin);
}