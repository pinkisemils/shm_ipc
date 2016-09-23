#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdio.h>  
#include <pthread.h>

void 
close_shm(char* shm_name) 
{
    int retval = shm_unlink(shm_name);
    if (retval != 0)
    {
        printf("Failed to close shm\n");
    }   
}

void*
init_shm(char* shm_name)
{
    mode_t old_umask = umask(0);
    int size = 1024 * 1024;
    int fd = shm_open(shm_name, O_RDWR | O_CREAT, 0666);
    if (fd == -1)
    {
        perror("Failed to initialize shared memory");
        return (void*)-1;
    }
    umask(old_umask);
    int rc = ftruncate(fd, size);
    if (rc == -1) 
    {
        perror("Failed to set size properly");
    }

    int mflags =  MAP_SHARED;
    void* addr = mmap(0, size, PROT_READ | PROT_WRITE, mflags, fd, (off_t) 0);
        perror("Failed to mmap()");
    if (addr == MAP_FAILED || addr == (void*)-1)
    {
        perror("Failed to mmap()");
        return (void*)-1;
    }
    int* marker = (int*) addr;
    if (*marker == 1 << 32) {
        return addr;
    } 
    memset(addr, 0, size);
    *marker = 1 <<32;
    addr = addr + sizeof(int);


    
    int ret = pthread_mutexattr_init(addr);
    if (ret != 0)
    {
        perror("Could initialize mutex attributes");
    }
    ret = pthread_mutexattr_setpshared(addr, PTHREAD_PROCESS_SHARED);
    if (ret != 0)
    {
        perror("Could set mutex attributes to PTHREAD_PROCESS_SHARED");
    }

    pthread_mutexattr_t* attr_ptr = addr;
    pthread_mutex_t* lock = (void*) attr_ptr + sizeof(pthread_mutexattr_t);
    ret = pthread_mutex_init(lock, attr_ptr);
    if (ret != 0)
    {
        perror("Could initialize mutex");
    }

    // setting up a pthread conditional
    pthread_condattr_t* cond_attr = (void*) lock + sizeof(pthread_mutex_t);
    pthread_condattr_init(cond_attr);
    pthread_condattr_setpshared(cond_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_t* cond = (void*) cond_attr + sizeof(pthread_condattr_t);
    pthread_cond_init(cond, cond_attr);
    return addr;
}

int
send_user_input(void* mmap_ptr)
{
    // getting the lock and jumpint to uninitialized memory
    pthread_mutex_t* lock = (pthread_mutex_t*) mmap_ptr + sizeof(pthread_mutexattr_t) + sizeof(int);
    pthread_cond_t* cond = (pthread_cond_t*) lock + sizeof(pthread_condattr_t) + sizeof(pthread_mutex_t);
    int* counter = (int*) cond + sizeof(pthread_cond_t);
    int* values = counter + sizeof(int);
    printf("Diff: %d", (void*)lock - mmap_ptr);
    printf("*values=%d\n", values);
    printf("*counter=%d\n", counter);

    // if the counter is non-zero, we don't know who's done what to this memory region
    if (*counter != 0) {
        printf("Unexpected counter, did the previous process not tear down this properly ?\n");
        return 1;
    }

    printf("Enter (non-integer) garbage to stop the program, enter any integer to send it to another program\n");
    int received_value;
    while (scanf("%d", &received_value) == 1){
        pthread_mutex_lock(lock);
        *counter++;
        values[*counter] = received_value;
        pthread_cond_signal(cond);
        pthread_mutex_unlock(lock);
        printf("Successfully wrote to shm\n");
    }
    pthread_mutex_unlock(lock);

    //setting the counter to 0 makes the receiver return;
    *counter = -1;
    pthread_mutex_unlock(lock);
    pthread_cond_signal(cond);
    return 0;
}



int 
main() 
{
    char* shm_name = "/test";
    void* addr = init_shm(shm_name);
    int retval;
    if (addr == (void*) -1) {
        printf("Failed\n");
        return 1;
    }
    printf("Initialized shared memory\n");
    retval = send_user_input(addr);
    if (retval != 0 )
    {
        return 1;
    }

    close_shm(shm_name);

    return 0;
}
