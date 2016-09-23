#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdio.h>  
#include <pthread.h>
#include <unistd.h>


void*
init_shm(char* shm_name)
{
    // reader expects the shared memory device to already be there;
    int size = 1024 * 1024;
    int fd = shm_open(shm_name, O_RDWR, 0666);
    if (fd == -1) 
    {
        perror("Failed to open shared memory");
        return (void*) -1;
    }

    int rc = ftruncate(fd, size);
    if (rc == -1) {
        perror("Failed to set size properly");
    }

    int mflags = MAP_SHARED;
    void* addr = mmap(0, 1024*1024, PROT_READ | PROT_WRITE, mflags, fd, (off_t) 0);
    if (addr == MAP_FAILED || addr == (void*)-1) 
    {
        perror("Failed to mmap()");
        return (void*) -1;
    }
    int* marker = (int*) addr;
    //if ( *marker != 1<<31 ){
    //    printf("Uninitialized memory!\n");
    //    return (void*)-1;
    //}
    pthread_mutex_t* lock = (void*) addr + sizeof(pthread_mutexattr_t) + sizeof(int);
    // this call will fail if the mutex isn't yet initialized.
    // It will fail in an _undefined behaviour_ kind of way.
    int try = pthread_mutex_trylock(lock);
    // if we actually lock, everything is fine
    // if we don't and we haven't blown up, everything is fine
    if (try == 0) 
    {
        pthread_mutex_unlock(lock);
    }
    return addr;
}

int
receive_user_input(void* mmap_ptr)
{
    pthread_mutex_t* lock = (pthread_mutex_t*) mmap_ptr + sizeof(pthread_mutexattr_t) + sizeof(int);
    pthread_cond_t* cond = (pthread_cond_t*) lock + sizeof(pthread_condattr_t) + sizeof(pthread_mutex_t);
    int* counter = (int*) cond + sizeof(pthread_cond_t);
    int* values = counter + sizeof(int);
    printf("Diff: %d", (void*)lock - mmap_ptr);
    printf("Receiving!\n");
    while (1) 
    {
        pthread_mutex_lock(lock);
        pthread_cond_wait(cond, lock);
        if (*counter == -1){
            pthread_mutex_unlock(lock);
            return 0;
        }
        printf("This ever happens\n");
        while (*counter > 0 )
        {
            printf("Received: %d\n", values[*counter]);
            *counter--;
        }
        pthread_mutex_unlock(lock);
    }
    return 0;
}

int
main()
{
    char* shm_name = "/test";
    void* addr = init_shm(shm_name);
    int retval;

    if (addr == (void*) -1) {
        printf("Failed");
        return -1;
    }
    printf("Initialized shared memory\n");
    retval = receive_user_input(addr);
    if (retval != 0) 
    {
        return 1;
    }
}


