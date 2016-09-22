#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdio.h>  
#include <pthread.h>

void 
close_shm(char* shm_name) 
{
    int retval = shm_unlink(shm_name);
    if (retval != 0) {
        printf("Failed to close shm\n");
    }   
}
//pthread_rwlockattr_t G_MUTEX_ATTR;

void*
init(char* shm_name)
{
        
    mode_t old_umask = umask(0);
    int fd = shm_open(shm_name, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("Failed to initialize shared memory");
        return (void*)-1;
    }
    umask(old_umask);

    int mflags =  MAP_SHARED | MAP_ANONYMOUS;
    void* addr = mmap(0, 65536, PROT_READ | PROT_WRITE, mflags, fd, (off_t) 0);
    if (addr == MAP_FAILED || addr == (void*)-1) {
        perror("Failed to mmap()");
        return (void*)-1;
    }
    int ret = pthread_rwlockattr_init(addr);
    if (ret != 0){
        perror("Could initialize rwlock attributes");
    }
    ret = pthread_rwlockattr_setpshared(addr, PTHREAD_PROCESS_SHARED);
    if (ret != 0){
        perror("Could set rwlock attributes to PTHREAD_PROCESS_SHARED");
    }

    pthread_rwlockattr_t* attr_ptr = addr;
    pthread_rwlock_t* rwlock = (void*) attr_ptr + sizeof(pthread_rwlockattr_t);
    ret = pthread_rwlock_init(addr, attr_ptr);
    if (ret != 0){
        perror("Could initialize rwlock");
    }
    return addr;
}

int
send_user_input(void* mmap_ptr)
{
    // getting the lock and jumpint to uninitialized memory
    pthread_rwlock_t* lock = (pthread_rwlock_t*) mmap_ptr + sizeof(pthread_rwlockattr_t);
    pthread_rwlock_rdlock(lock);
    int* counter = mmap_ptr + sizeof(pthread_rwlock_t) + sizeof(pthread_rwlockattr_t);
    int* values = counter + sizeof(int);

    if (*counter != 0) {
        printf("Unexpected counter, did the previous process not tear down this properly ?\n");
        return 1;
    }
    pthread_rwlock_unlock(lock);

    printf("Enter (non-integer) garbage to stop the program, enter any integer to send it to another program\n");
    int received_value;
    while (scanf("%d", &received_value) == 1){
        pthread_rwlock_wrlock(lock);
        values[*counter] = received_value;
        *counter += 1;
        pthread_rwlock_unlock(lock);
    }
    return 0;
}



int 
main() 
{
    char* shm_name = "/test";
    void* addr = init(shm_name);
    int retval = 0;
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
