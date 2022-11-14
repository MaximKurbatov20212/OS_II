#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define SUCCESS 0
#define ERROR 1
#define ERROR_CODE (void*) -1
#define N 10
#define THREADS_ONLY 0

typedef struct Semaphores {
    sem_t* sem1;
    sem_t* sem2;
} semaphores_t;

void* child_print(void* args) {

    semaphores_t* sems = (semaphores_t *) args;

    for (int i = 0; i < N; i++) {
        int wait_res = sem_wait(sems->sem1); 
        if (wait_res != SUCCESS) {
            perror("sem_wait");
            return ERROR_CODE;
        }

        printf("child\n");
        int post_res = sem_post(sems->sem2);
        if (post_res != SUCCESS) {
            perror("sem_post");
            return ERROR_CODE;
        }
    }
    pthread_exit(NULL);
}

int parent_print(semaphores_t* sems) {
    for (int i = 0; i < N; i++) {
        int wait_res = sem_wait(sems->sem2); 
        if (wait_res != SUCCESS) {
            perror("sem_wait");
            return wait_res;
        }

        printf("parent\n");
        int post_res = sem_post(sems->sem1);
        if (post_res != SUCCESS) {
            perror("sem_post");
            return post_res;
        }
    }
    return SUCCESS;
}

int main() {
    pthread_t thread;
    sem_t sem1, sem2;

    int init_res = sem_init(&sem1, THREADS_ONLY, 0);
    if (init_res != SUCCESS) {
        perror("sem_init 1");
        return init_res;
    }

    init_res = sem_init(&sem2, THREADS_ONLY, 1);
    if (init_res != SUCCESS) {
        perror("sem_init 2");
        return init_res;
    }

    semaphores_t sems = {&sem1, &sem2};
    int create_res = pthread_create(&thread, NULL, child_print, (void *) (&sems));
    if (create_res != SUCCESS){
        perror("pthread_create");
        return create_res;
    }

    int print_res = parent_print(&sems);

    if (print_res != SUCCESS) return print_res;

    pthread_exit(NULL);
}

// wait -1
// post +1

// sem1 0
// sem2 1
