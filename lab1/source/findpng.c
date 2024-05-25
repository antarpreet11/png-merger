#include "lab_png.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>   /* for printf().  man 3 printf */
#include <stdlib.h>  /* for exit().    man 3 exit   */
#include <string.h>  /* for strcat().  man strcat   */

void check_and_print_png(const char *filePath, int *foundPNG) {
    FILE *img = fopen(filePath, "rb");

    U8 header[PNG_SIG_SIZE];
    fread(header, 1, PNG_SIG_SIZE, img);
    int isPNG = is_png(header, PNG_SIG_SIZE);

    if (isPNG == 0) {
        printf("%s\n", filePath);
        *foundPNG = 1;
    }

    fclose(img);
}

void findpng(const char *directory, int *foundPNG) {
    DIR *dir = opendir(directory);
    char str[64];

    if (dir == NULL) {
        sprintf(str, "opendir(%s)", directory);
        perror(str);
        return;
    }

    struct dirent *p_dirent;
    while((p_dirent = readdir(dir)) != NULL) {
        char *str_path = p_dirent->d_name;

        if (strcmp(str_path, ".") == 0 || strcmp(str_path, "..") == 0) {
            continue;
        }

        char newpath[1024];
        snprintf(newpath, sizeof(newpath), "%s/%s", directory, str_path);
        
        struct stat buf;
        if (lstat(newpath, &buf) < 0) {
            perror("lstat error");
            continue;
        }

        if (S_ISDIR(buf.st_mode)) {
            findpng(newpath, foundPNG);
        } else if (S_ISREG(buf.st_mode)) {
            check_and_print_png(newpath, foundPNG);
        }  
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    const char *directory = argv[1];
    int foundPNG = 0;
    findpng(directory, &foundPNG);

    if (foundPNG == 0) {
        printf("findpng: No PNG file found\n");
    }

    return 0;
}