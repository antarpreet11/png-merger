#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "main_write_header_cb.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    int pic;
    int *count;
    bool *downloaded[50];
    pthread_t id;
}thread_args;
thread_args *args;

void *download_imgs(void *args) {
    thread_args *progress = args;
    while(1) {
        pthread_mutex_lock(&mutex);
        int localCount = *progress->count;
//printf("localCount: %d ", localCount);
        if(localCount < 50) {
//printf("calling download image\n");
//printf("%d\n", *progress->count);
            *progress->count = download_img(progress->pic, *progress->count, progress->downloaded); }
        pthread_mutex_unlock(&mutex);
        
        if(localCount >= 50)
            break;
    }

    return NULL;
}

int paster(int numT, int pic) {
    /*bool downloaded[50] = {false};
    int count = 0;*/
    args = malloc(sizeof(thread_args));
    pthread_t *threadIDs = malloc(sizeof(pthread_t) * numT);
    int count = 0;

    args->pic = pic;
    args->count = &count;

        for(int j=0; j<50; j++) {
            args->downloaded[j] = malloc(sizeof(bool));
            *(args->downloaded[j]) = false;
        }

    for(int i=0; i<numT; i++){

        pthread_create(&threadIDs[i], NULL, &download_imgs, (void *)args);
    }
/*for(int i=0; i<numT; i++)
for(int j=0; j<50; j++)
printf("Thread: %d Index: %d Value: %d\n",i,j, args[i].downloaded[j]);*/

    char **file_paths = malloc(50 * sizeof(char *));

    printf("Use %d threads.\n", numT);
    printf("Get picture %d.\n", pic);

    for(int i=0; i<numT; i++)
        pthread_join(threadIDs[i], NULL);

printf("%d\n", count);

    /*for (int i = 0; i < 50; i++) {
        file_paths[i] = malloc(20 * sizeof(char));
        if (file_paths[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return -1; 
        }
        sprintf(file_paths[i], "./source/img/img%d_%d.png", pic, i);
    }*/

    //catpngmain(file_paths);
    pthread_mutex_destroy(&mutex);

    for(int j=0; j<50; j++)
        free(args->downloaded[j]);

    free(args);
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