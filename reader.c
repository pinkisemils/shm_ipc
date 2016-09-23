#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdio.h>  
#include <pthread.h>
#include <unistd.h>

#include "shm.h"

void
receive_user_input(shm_t* shm)
{
    printf("Enter your message in the writer\n");
    // hopefully this process will block on this mutex
    while(shm->marker)
    {
        pthread_mutex_lock(&shm->mutex);
        while(shm->counter < 0 && shm->marker) 
        {
            pthread_cond_wait(&shm->condition, &shm->mutex);
        }
        if (!shm->marker) {
            pthread_mutex_unlock(&shm->mutex);
            break;
        }
        while(shm->counter >= 0)
        {
            printf("Received: %d\n", shm->queue[shm->counter]);
            shm->counter--;
        }
        pthread_mutex_unlock(&shm->mutex);
    }
    return;
}

int
main()
{
    char* shm_name = "/test";
    shm_t* shm = get_shm(shm_name);

    if (shm == (void*) -1) {
        printf("Failed to initialize shared memory\n");
        return -1;
    }
    receive_user_input(shm);
    return 0;
}


