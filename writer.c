#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdio.h>  
//#include <unistd.h>
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
pthread_mutexattr_t g_mutex_attr;
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

    //int mflags =  MAP_SHARED;
    void* addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t) 0);
    if (addr == MAP_FAILED || addr == (void*)-1)
    {
        perror("Failed to mmap()");
        return (void*)-1;
    }
    
    int ret = pthread_mutexattr_init(&g_mutex_attr);
    if (ret != 0)
    {
        perror("Could initialize mutex attributes");
        return (void*) -1;
    }
    // setting the mutex attribute to make mutexes shareable between processes
    ret = pthread_mutexattr_setpshared(&g_mutex_attr, PTHREAD_PROCESS_SHARED);
    if (ret != 0)
    {
        perror("Could set mutex attributes to PTHREAD_PROCESS_SHARED");
        return (void*) -1;
    }

    pthread_mutex_t* lock = addr;
    ret = pthread_mutex_init(lock, &g_mutex_attr);
    if (ret != 0)
    {
        perror("Could not initialize mutex");
        return (void*) -1;
    }

    // setting up a pthread conditional
    return addr;
}

int
send_user_input(void* mmap_ptr)
{
    // getting the lock and jumpint to uninitialized memory
    pthread_mutex_t* lock = (pthread_mutex_t*) mmap_ptr;
    // locking the mutex so that the reader process doesn't just read zeroes until it segfaults
    pthread_mutex_lock(lock);
    printf("Start up the reader\n");
    char* buffer = (char*) mmap_ptr + sizeof(pthread_mutex_t);
    scanf("%s", buffer);
    pthread_mutex_unlock(lock);
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
    retval = send_user_input(addr);
    //removes the shm 'file'
    close_shm(shm_name);
    return retval;
}
