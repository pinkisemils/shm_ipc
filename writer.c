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

    printf("Start up the reader\n");
    // getting a pointer to the heap of bytes allocated perviosly
    int recv;
    while(scanf("%d", &recv) != -1)
    {
        shm->queue[shm->counter] = recv;
        shm->counter += 1;
        pthread_mutex_unlock(&shm->mutex);

    }
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
