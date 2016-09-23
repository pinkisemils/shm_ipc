#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdio.h>  
#include <unistd.h>
#include <pthread.h>
#include <sys/file.h>


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

pthread_mutexattr_t g_mutex_attr;

int g_shm_obj_fd;

void*
init_shm(char* shm_name)
{
    // umask is needed so that the process is allowed to create globally writeable files
    // this is here to make everything super-stupid-simple
    mode_t old_umask = umask(0);
    // plenty of bytes because memory is cheap
    int size = 1024 * 1024;
    int fd = shm_open(shm_name, O_RDWR | O_CREAT, 0666);
    if (fd == -1)
    {
        perror("Failed to initialize shared memory");
        return (void*)-1;
    }
    umask(old_umask);
    g_shm_obj_fd = fd;
    
    //lock the file descriptor until we've acquired a lock in the shm object
    flock(fd, LOCK_EX);

    // ftruncate is needed to set the proper size
    // a newly created shm object is 0 bytes long
    int rc = ftruncate(fd, size);
    if (rc == -1) 
    {
        perror("Failed to set size properly");
    }

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
    // setting the mutex attribute to allow for inter-process locking
    ret = pthread_mutexattr_setpshared(&g_mutex_attr, PTHREAD_PROCESS_SHARED);
    if (ret != 0)
    {
        perror("Could set mutex attributes to PTHREAD_PROCESS_SHARED");
        return (void*) -1;
    }

    // initializing the mutex
    pthread_mutex_t* lock = addr;
    ret = pthread_mutex_init(lock, &g_mutex_attr);
    if (ret != 0)
    {
        perror("Could not initialize mutex");
        return (void*) -1;
    }

    return addr;
}

void
send_user_input(void* mmap_ptr)
{
    // getting the lock
    pthread_mutex_t* lock = (pthread_mutex_t*) mmap_ptr;
    // locking the mutex so that the reader process doesn't just read zeroes until it segfaults
    pthread_mutex_lock(lock);

    //unlock file lock
    flock(g_shm_obj_fd, LOCK_UN);

    printf("Start up the reader\n");
    // getting a pointer to the heap of bytes allocated perviosly
    char* buffer = (char*) mmap_ptr + sizeof(pthread_mutex_t);
    scanf("%s", buffer);
    pthread_mutex_unlock(lock);
}



int 
main() 
{
    char* shm_name = "/test";
    void* addr = init_shm(shm_name);
    if (addr == (void*) -1) {
        printf("Failed\n");
        return 1;
    }
    send_user_input(addr);
    close_shm(shm_name);
    return 0;

}
