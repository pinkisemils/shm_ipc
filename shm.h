#include <pthread.h>
#include <stdbool.h>

#define G_QUEUE_SIZE 2048
typedef struct shm_t {
    bool marker; //should always be true if the memory region is ready
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    bool written_to;
    int counter;
    int queue[G_QUEUE_SIZE];
} shm_t;


shm_t*
get_shm(char* shm_name);
//pthread_condattr_t g_cond_attr;

//by default, the mutex is locked
shm_t*
init_main_shm(char* shm_name);


