#include <stdio.h>
#include <pthread.h>

#define N 4
#define SUCCESS 0
#define ERROR 1 
#define PTHREAD_CREATE_ERROR 0 


void* print_msg(void* arg) {
    const char** args = (const char **) arg;
    for (int i = 1; i <= args[0][0] - '0'; i++) {
        printf("%s", args[i]); 
    }
    printf("\n");
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    pthread_t threads[4];

    // First elem is lenght of array - 1
    char* arg1[8] = {"7", "t", "h", "r", "e", "a", "d", "1"};
    char* arg2[8] = {"7", "t", "h", "r", "e", "a", "d", "2"};
    char* arg3[8] = {"7", "t", "h", "r", "e", "a", "d", "3"};
    char* arg4[9] = {"8", "t", "h", "r", "e", "a", "d", "4", "5"};

    char** args[4] = {arg1, arg2, arg3, arg4};

    for (int i = 0; i < N; i++) {
        int create_result = pthread_create(&threads[i], NULL, print_msg, args[i]);
        if (create_result != PTHREAD_CREATE_ERROR) {
            printf("pthread_create error: couldn't create thread\n");
            return ERROR;
        }
    }

    pthread_exit(NULL);
    return SUCCESS;
}
