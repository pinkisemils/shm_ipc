#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>  
#include <stdio.h>  

#include "shm.h"

pthread_mutexattr_t g_mutex_attr;
pthread_condattr_t g_cond_attr;

extern
shm_t*
init_main_shm(char* shm_name)
{
    mode_t old_umask = umask(0);
    int fd = shm_open(shm_name, O_RDWR | O_CREAT, 0666);
    if (fd == -1)
    {
        perror("Failed to initialize shared memory");
        return (void*)-1;
    }
    int rc = ftruncate(fd, sizeof(shm_t));
    if (rc == -1) 
    {
        perror("Failed to set size properly");
        return (void*)-1;
    }
    void* addr = mmap(0, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t) 0);
    if (addr == MAP_FAILED || addr == (void*)-1)
    {
        perror("Failed to mmap()");
        return (void*)-1;
    }


    shm_t* shm = (shm_t*) addr;
    shm->marker = true;

    int e;
    e = pthread_mutexattr_init(&g_mutex_attr);
    if (e != 0)
    {
        perror("Could set mutex attributes to PTHREAD_PROCESS_SHARED");
        return (void*) -1;
    }
    // setting the mutex attribute to allow for inter-process locking
    e = pthread_mutexattr_setpshared(&g_mutex_attr, PTHREAD_PROCESS_SHARED);
    if (e != 0)
    {
        perror("Could set mutex attributes to PTHREAD_PROCESS_SHARED");
        return (void*) -1;
    }

    e = pthread_mutex_init(&shm->mutex, &g_mutex_attr);
    if (e != 0)
    {
        perror("Could not initialize mutex");
        return (void*) -1;
    }
    pthread_mutex_lock(&shm->mutex);

    return shm;
}

extern
shm_t*
get_shm(char* shm_name)
{
    // reader expects the shared memory device to already be there;
    int fd = shm_open(shm_name, O_RDWR, 0666);
    if (fd == -1) 
    {
        perror("Failed to open shared memory");
        return (shm_t*) -1;
    }

    //memmory mapping the shared memory file descriptor
    void* addr = mmap(0, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t) 0);
    if (addr == MAP_FAILED || addr == (void*)-1) 
    {
        perror("Failed to mmap()");
        return (shm_t*) -1;
    }
    shm_t* shm = (shm_t *) addr;
    if (!shm->marker) 
    {
        printf("MMAP looks as though it's uninitialized\n");
        return (shm_t*) -1;
    }
    return shm;

}
