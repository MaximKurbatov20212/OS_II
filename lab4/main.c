#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define PTHREAD_CREATE_SUCCESS 0
#define ERROR 1
#define SUCCESS 0 
#define DELAY 2
#define SLEEP_ERROR -1
#define CANCEL_SUCCESS 0
#define TRUE 1

void* print_smth() {
    while (TRUE) {
        printf("New thread\n");
    }
}

int main(int argc, char* argv[]) {
    pthread_t thread;

    int create_result = pthread_create(&thread, NULL, print_smth, NULL);

    if (create_result != PTHREAD_CREATE_SUCCESS) {
        printf("pthread_create error\n");
        return create_result;
    }

    int sleep_res = sleep(DELAY);
    if (sleep_res == SLEEP_ERROR) {
        printf("sleep error\n");
        return SLEEP_ERROR;
    }

    int cancel_result = pthread_cancel(thread);
    if (cancel_result != CANCEL_SUCCESS) {
        printf("cancel error\n");
        return ERROR;
    }

    return SUCCESS;
}
