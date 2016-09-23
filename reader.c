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

    //memmory mapping the shared memory file descriptor
    void* addr = mmap(0, 1024*1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t) 0);
    if (addr == MAP_FAILED || addr == (void*)-1) 
    {
        perror("Failed to mmap()");
        return (void*) -1;
    }
    return addr;
}

void
receive_user_input(void* mmap_ptr)
{
    pthread_mutex_t* lock = (pthread_mutex_t*) mmap_ptr;
    char* buffer = (char*) mmap_ptr + sizeof(pthread_mutex_t);

    printf("Enter your message in the writer\n");
    // hopefully this process will block on this mutex
    pthread_mutex_lock(lock);
    printf("Received: %s\n", buffer);
    pthread_mutex_unlock(lock);
}

int
main()
{
    char* shm_name = "/test";
    void* addr = init_shm(shm_name);

    if (addr == (void*) -1) {
        printf("Failed to initialize shared memory\n");
        return -1;
    }
    receive_user_input(addr);
    return 0;
}


