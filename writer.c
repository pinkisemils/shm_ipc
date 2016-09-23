#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>  
#include <stdio.h>  
#include <pthread.h>
#include <sys/file.h>
#include "shm.h"



void
send_user_input(shm_t* shm)
{
    // locking the mutex so that the reader process doesn't just read zeroes until it segfaults

    pthread_mutex_unlock(&shm->mutex);
    printf("Start up the reader\n");
    // getting a pointer to the heap of bytes allocated perviosly
    int recv;
    while(scanf("%d", &recv) == 1)
    {
        pthread_mutex_lock(&shm->mutex);
        shm->counter = shm->counter + 1;
        shm->queue[shm->counter] = recv;
        pthread_cond_signal(&shm->condition);
        pthread_mutex_unlock(&shm->mutex);

    }
    pthread_mutex_lock(&shm->mutex);
    shm->marker = false;
    pthread_cond_signal(&shm->condition);
    pthread_mutex_unlock(&shm->mutex);
    return;
}



int 
main() 
{
    char* shm_name = "/test";
    shm_t* shm = init_main_shm(shm_name);
    if (shm == (void*) -1) {
        printf("Failed\n");
        return 1;
    }
    send_user_input(shm);
    close_shm(shm_name);
    return 0;

}
