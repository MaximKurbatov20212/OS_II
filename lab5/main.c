#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define PTHREAD_CREATE_SUCCESS 0
#define PTHREAD_JOIN_SUCCESS 0
#define ERROR 1
#define SUCCESS 0 
#define DELAY 2
#define SLEEP_ERROR -1
#define CANCEL_SUCCESS 0
#define TRUE 1
#define NO_ARGS 0

void print_cleanup_push_handler(void* args) {
    printf("cleanup_push: %s\n" , (char *) args);
}

void* print_smth() {
    pthread_cleanup_push(print_cleanup_push_handler, "Child thread");

    while (TRUE) {
        printf("New thread\n");
    }

//        pthread_cleanup_pop(NO_ARGS);
}

int main(int argc, char* argv[]) {
    pthread_t thread;

    int create_result = pthread_create(&thread, NULL, print_smth, NULL);

    if (create_result != PTHREAD_CREATE_SUCCESS) {
        printf("pthread_create error\n");
        return create_result;
    }
    
    struct timespec tv;
    tv.tv_sec = DELAY;
    tv.tv_nsec = 0;

    int sleep_res = nanosleep(&tv, NULL);
    if (sleep_res == SLEEP_ERROR) {
        perror("nanosleep\n");
        return SLEEP_ERROR;
    }

    int cancel_result = pthread_cancel(thread);
    if (cancel_result != CANCEL_SUCCESS) {
        printf("cancel error\n");
        return ERROR;
    } 

    int join_result = pthread_join(thread, NULL);
    if (join_result != PTHREAD_JOIN_SUCCESS) {
        printf("pthread_join error\n");
        return ERROR;
    }

    return SUCCESS;
}
