#include <stdio.h>
#include <pthread.h>

#define SUCCESS 0
#define ERROR 1
#define PTHREAD_CREATE_ERROR 0
#define N 10

void* print_msg() {
    for (int i = 0; i < N; i++) {
        printf("New thread\n");
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t new_thread;

    int create_result = pthread_create(&new_thread, NULL, print_msg, NULL);

    if (create_result != PTHREAD_CREATE_ERROR) {
        printf("pthread_create error: couldn't create thread\n");
        return ERROR;
    }

    pthread_join(new_thread, NULL);

    for (int i = 0; i < N; i++) {
        printf("Main thread\n");
    }

    return SUCCESS;
}

