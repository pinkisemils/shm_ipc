#include <pthread.h>
#include <stdbool.h>

#define G_QUEUE_SIZE 2048
typedef struct shm_t {
    bool marker; //Indiciates whether the shm object is initialized
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    bool written_to;
    volatile int counter;
    int queue[G_QUEUE_SIZE];
} shm_t;


shm_t*
get_shm(char* shm_name);

shm_t*
init_main_shm(char* shm_name);

void 
close_shm(char* shm_name);

