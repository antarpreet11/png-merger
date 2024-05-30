#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include "main_write_header_cb.c"

struct thread_args
{
    int pic;
};

int paster(int numT, int pic) {
    bool downloaded[50];
    int count = 0;

    printf("Use %d threads.\n", numT);
    printf("Get %d picture.\n", pic);
    //download_img(pic);

    pthread_t *p_tids = malloc(sizeof(pthread_t) * numT);
    struct thread_args in_params[numT];
    /*struct thread_ret *p_results[numT];*/
     
    for (int i=0; i<numT; i++) {
        in_params[i].pic = pic;
        pthread_create(p_tids + i, NULL, download_img, in_params + i); 
    }

    for (int i=0; i<numT; i++) {
        (void)pthread_join(p_tids[i], NULL);
        /*printf("Thread ID %lu joined.\n", p_tids[i]);
        printf("sum(%d,%d) = %d.\n", \
               in_params[i].x, in_params[i].y, p_results[i]->sum); 
        printf("product(%d,%d) = %d.\n\n", \
               in_params[i].x, in_params[i].y, p_results[i]->product);*/
    }

    /* cleaning up */

    free(p_tids);
    
    while (count < 50) {
       // count = download_img(pic, count, downloaded);
        printf("count = %d\n", count);
    }

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