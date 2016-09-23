#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>  
#include <stdio.h>  
#include <pthread.h>
#include <sys/file.h>
#include "shm.h"


void 
close_shm(char* shm_name) 
{
    // unlinks the shared memory object
    // it will still exist if it's used by anything
    // as it goes for any file handle
    int retval = shm_unlink(shm_name);
    if (retval != 0)
    {
        printf("Failed to close shm\n");
    }   
}


void
send_user_input(shm_t* shm)
{
    // locking the mutex so that the reader process doesn't just read zeroes until it segfaults

    printf("Start up the reader\n");
    // getting a pointer to the heap of bytes allocated perviosly
    char* buffer = (shm->queue);
    scanf("%s", buffer);
    pthread_mutex_unlock(&shm->mutex);
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
