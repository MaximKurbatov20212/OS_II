#include <stdio.h>
#include <pthread.h>

#define SUCCESS 0
#define ERROR 1
#define PTHREAD_CREATE_ERROR 0
#define N 10
#define PTHREAD_JOIN_SUCCESS 0
#define NUM_OF_MUT 3

pthread_mutex_t mutex;

int lock_mutex(pthread_mutex_t* mutex) {
    int lock_res = pthread_mutex_lock(mutex);
    if (lock_res != SUCCESS) {
        perror("lock_mutex");
        return ERROR;
    }
    return SUCCESS;
}

int unlock_mutex(pthread_mutex_t* mutex) {
    int unlock_res = pthread_mutex_unlock(mutex);
    if (unlock_res != SUCCESS) {
        perror("unlock_mutex");
        return ERROR;
    }
    return SUCCESS;
}

void* child_print(void* args) {
    pthread_mutex_t* mutexes = (pthread_mutex_t*) args;

    int current_mutex = 2;
    lock_mutex(&mutexes[current_mutex]);
    for (int i = 0; i < N; i++) {
        lock_mutex(&mutexes[(current_mutex + 1) % NUM_OF_MUT]);
        printf("CHILD\n");
        unlock_mutex(&mutexes[current_mutex]);
        current_mutex = (current_mutex + 1) % NUM_OF_MUT;
    }

    unlock_mutex(&mutexes[current_mutex]);
    pthread_exit(args);
}

int parent_print(pthread_mutex_t* mutexes) {
    int current_mutex = 0;
    for (int i = 0; i < N; i++) {
        lock_mutex(&mutexes[(current_mutex + 1) % NUM_OF_MUT]);
        printf("PARENT\n");
        unlock_mutex(&mutexes[current_mutex]);
        current_mutex = (current_mutex + 1) % NUM_OF_MUT;
    }

    unlock_mutex(&mutexes[current_mutex]);
    return SUCCESS;
}

int init_mutexes(pthread_mutex_t* mutexes) {
    pthread_mutexattr_t mutex_attr;
    int error_code = pthread_mutexattr_init(&mutex_attr);
    if (error_code != SUCCESS) {
        perror("pthread_mutexattr_init");
        return ERROR;
    }

    // check errors and returns it 
    int settype_res = pthread_mutexattr_settype(&mutex_attr, 
                                                PTHREAD_MUTEX_ERRORCHECK);
    if (settype_res != SUCCESS) {
        perror("pthread_mutexattr_settype");
        pthread_mutexattr_destroy(&mutex_attr); // ignore return value, because return ERROR
        return ERROR;
    }

    for (int i = 0; i < NUM_OF_MUT; i++) {
        int init_res = pthread_mutex_init(&mutexes[i], &mutex_attr);
        if (init_res != SUCCESS) {
            perror("pthread_mutex_init");
            return ERROR;
        }
    }
    return SUCCESS;
}

int destroy_mutexes(pthread_mutex_t* mutexes) {
    for (int i = 0; i < NUM_OF_MUT; i++) {
        int destroy_res = pthread_mutex_destroy(&mutexes[i]);
        if (destroy_res != SUCCESS) {
            perror("pthread_mutex_destroy");
            return ERROR;
        }
    }
    return SUCCESS;
}

int main() {
    pthread_t new_thread;

    pthread_mutex_t mutexes[NUM_OF_MUT];
    int init_result = init_mutexes(mutexes);

    if (init_result == ERROR) {
        return ERROR;
    }

    lock_mutex(&mutexes[0]);


    int create_result = pthread_create(&new_thread, 
                                        NULL, 
                                        child_print, 
                                        &mutexes);

    if (create_result != PTHREAD_CREATE_ERROR) {
        printf("pthread_create error: couldn't create thread\n");
        return ERROR;
    }

    
    struct timespec ts = {1, 0};
    nanosleep(&ts, NULL);

    int print_res = parent_print(mutexes);

    if (print_res == ERROR) {
        return ERROR;
    }

    int join_result = pthread_join(new_thread, NULL);
    if (join_result != PTHREAD_JOIN_SUCCESS) {
        fprintf(stderr, "join error");
        return join_result;
    }

    int destroy_mutexes_res = destroy_mutexes(mutexes);

    if (destroy_mutexes_res != SUCCESS) {
        return ERROR;
    } 

    return SUCCESS;
}
