#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define NUM_OF_THREADS 4
#define NUM_OF_STRINGS 2
#define BUF_SIZE 10
#define SUCCESS 0
#define ERROR 1 
#define PTHREAD_CREATE_ERROR 0 
#define PTHREAD_JOIN_SUCCESS 0 

struct msg {
    char** string;
    int num;
};

void fill(char* to, const char* from, const int len) {
    for (int i = 0; i < len; ++i) {
        to[i] = from[i];
    }
    to[len] = 0;
}

struct msg* generate_strings() {
    struct msg* msgs = (struct msg*) malloc(NUM_OF_THREADS * sizeof(struct msg));

    for (int i = 0; i < NUM_OF_THREADS; ++i) {
        msgs[i].string = (char**) malloc(NUM_OF_THREADS * sizeof(char*));
        msgs[i].num = NUM_OF_STRINGS;
        for (int j = 0; j < msgs[i].num; ++j) {
            msgs[i].string[j] = (char*) malloc(BUF_SIZE * sizeof(char));
        }
    }

    fill(msgs[0].string[0], "thread1_1", 9);
    fill(msgs[0].string[1], "thread1_2", 9);

    fill(msgs[1].string[0], "thread2_1", 9);
    fill(msgs[1].string[1], "thread2_2", 9);

    fill(msgs[2].string[0], "thread3_1", 9);
    fill(msgs[2].string[1], "thread3_2", 9);

    fill(msgs[3].string[0], "thread4_1", 9);
    fill(msgs[3].string[1], "thread4_2", 9);
    return msgs;
}

void* print_msg(void* arg) {
    struct msg* args = (struct msg*) arg;

    for (int i = 0; i < args->num; i++) {
        printf("%s\n", args->string[i]); 
    }
    printf("\n");
}


int main(int argc, char* argv[]) {
    pthread_t threads[NUM_OF_THREADS];
    struct msg* msgs = generate_strings();
    

    for (int i = 0; i < NUM_OF_THREADS; i++) {
        int create_result = pthread_create(&threads[i], NULL, print_msg, &msgs[i]);
        if (create_result != PTHREAD_CREATE_ERROR) {
            printf("pthread_create error: couldn't create thread\n");
            return ERROR;
        }
    }

    for (int i = 0; i < NUM_OF_THREADS; ++i) {
        int join_result = pthread_join(threads[i], NULL);

        if (join_result != PTHREAD_JOIN_SUCCESS) {
            printf("pthread_join error\n");
            return ERROR;
        }
    }
    
    for (int i = 0; i < NUM_OF_THREADS; ++i) {
        for (int j = 0; j < msgs[i].num; ++j) {
            free(msgs[i].string[j]);
        }
        free(msgs[i].string);
    }
    free(msgs);

    pthread_exit(NULL);
}
