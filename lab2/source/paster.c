#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "main_write_header_cb.c"

int paster(int numT, int pic) {
    printf("Use %d threads.\n", numT);
    printf("Get %d picture.\n", pic);
    download_img(pic);

    return 0;
}

int main(int argc, char **argv)
{
    int c;
    int t = 1;
    int n = 1;
    char *str = "option requires an argument";

    while ((c = getopt (argc, argv, "t:n:")) != -1) {
        switch (c) {
        case 't':
	    t = strtoul(optarg, NULL, 10);
	    printf("option -t specifies a value of %d.\n", t);
	    if (t <= 0) {
                fprintf(stderr, "%s: %s > 0 -- 't'\n", argv[0], str);
                return -1;
            }
            break;
        case 'n':
            n = strtoul(optarg, NULL, 10);
	    printf("option -n specifies a value of %d.\n", n);
            if (n <= 0 || n > 3) {
                fprintf(stderr, "%s: %s 1, 2, or 3 -- 'n'\n", argv[0], str);
                return -1;
            }
            break;
        default:
            return -1;
        }
    }
    paster(t, n);
}