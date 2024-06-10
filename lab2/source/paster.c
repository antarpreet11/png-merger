#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "main_write_header_cb.h"
#include "lib/catpng.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

thread_args *args;

void *download_imgs(void *args) {
    thread_args *progress = (thread_args*)args;
    thread_args *temp;

    while(1) {
        pthread_mutex_lock(&mutex);
        int localCount = *progress->count;
        temp = progress;
        pthread_mutex_unlock(&mutex);

        if(localCount < 50) {
            if(download_img(temp) != 0 || progress->noError == false){
                progress->noError = false; 
                break;
            }
        }
        
        if(localCount >= 50)
            break;
    }
    
    return NULL;
}

void cleanMemory (thread_args *a, pthread_t *t) {
    free(t);
    free(a);
    pthread_mutex_destroy(&mutex);
}

int paster(int numT, int pic) {
    args = malloc(sizeof(thread_args));
    pthread_t *threadIDs = malloc(sizeof(pthread_t) * numT);
    int count = 0;

    args->pic = pic;
    args->count = &count;
    args->noError = true;

    for(int j=0; j<50; j++) {
        args->downloaded[j] = NULL;
    }

    for(int i=0; i<numT; i++){

        pthread_create(&threadIDs[i], NULL, &download_imgs, (void *)args);
    }

    printf("Use %d threads.\n", numT);
    printf("Get picture %d.\n", pic);

    for(int i=0; i<numT; i++)
        pthread_join(threadIDs[i], NULL);

    if(args->noError) {
        catpngmain(args->downloaded);
        cleanMemory(args, threadIDs);

        return 0;
    }
    else {
        cleanMemory(args, threadIDs);

        return -2;
    }

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
    //add error statement
    //10 threads = 8x faster
    return paster(t, n);
}