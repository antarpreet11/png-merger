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
    int count;
    bool *downloaded[50];
}typedef thread_args;

void *download_imgs(void *args);
void *download_imgs(void *args) {
printf("download_imgs");
    thread_args *progress = args;
    while(progress->count<50) {
printf("while");
        download_img(progress->pic, progress->count, *progress->downloaded);
    }
    return NULL;
}

int paster(int numT, int pic) {
    /*bool downloaded[50] = {false};
    int count = 0;*/
    thread_args *args = malloc(sizeof(thread_args));
    args->count = 0;
    args->pic = pic;
    for(int i=0; i<50; i++);
       // args->downloaded[i] = false;

    char **file_paths = malloc(50 * sizeof(char *));

    printf("Use %d threads.\n", numT);
    printf("Get %d picture.\n", pic);

    pthread_t *p_tids = malloc(sizeof(pthread_t) * numT);
    if (p_tids == NULL) {
        printf("Thread ID malloc failed");
        return -1;
    }
printf("before thread loop");
    for (int i=0; i<numT; i++){
printf("thread loop");
        pthread_create(p_tids + i, NULL, download_imgs, args);
    }
    for (int i=0; i<numT; i++)
        pthread_join(p_tids[i], NULL);

    /*while (count < 50) {
        count = download_img(pic, count, downloaded);
        printf("count = %d\n", count);
    }*/

    /*for (int i = 0; i < 50; i++) {
        file_paths[i] = malloc(20 * sizeof(char));
        if (file_paths[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return -1; 
        }
        sprintf(file_paths[i], "./source/img/img%d_%d.png", pic, i);
    }*/

    //catpngmain(file_paths);
    free(p_tids);
    free(file_paths);
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

    return 0;
}