#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define STEPS_NUMBER 200000000
#define SUCCESS 0
#define ERROR -1
#define COMMAND_ARGUMENT_NUMBER 2
#define THREADS_MIN 1

typedef struct Limit {
    int start;
    int end;
} Limit;

void* calculatePartPI(void* args){
    if (args == NULL){
        printf("Unexpected argument value: NULL");
        return NULL;
    }

    Limit* limitsSerial = (Limit*) args;
    int start = limitsSerial->start;
    int end = limitsSerial->end;
    
    double localPI = 0;
    for (int i = start; i < end ; i++) {
        localPI += 1.0 / (i*4.0 + 1.0);
        localPI -= 1.0 / (i*4.0 + 3.0);
    }

    double* resultedLocalPI = (double*) malloc(sizeof(double));
    if (resultedLocalPI == NULL){
        perror("malloc");
        return NULL;
    }
    free(limitsSerial);
    *resultedLocalPI = localPI;
    return resultedLocalPI;
}

int createThreadsCalculatingPI(int threadsNumber, pthread_t* threads) {
    int stepsNumberThread = STEPS_NUMBER / threadsNumber;

    for (int i = 0; i < threadsNumber; i++) {
        Limit* limitsSerial = (Limit*) malloc(sizeof(Limit));
        if (limitsSerial == NULL){
            perror("malloc");
            return ERROR;
        }
        limitsSerial->start = i * stepsNumberThread;
        limitsSerial->end = (i + 1) * stepsNumberThread - 1;
        int errorCode = pthread_create(&threads[i], NULL, calculatePartPI, (void*) limitsSerial);
        if (errorCode != SUCCESS) {
            perror("pthread_create");
            free(limitsSerial);
            return errorCode;
        }
    }
    return SUCCESS;
}

long readThreadsNumber(int argc, char** argv) {
    if (argc != COMMAND_ARGUMENT_NUMBER){
        printf("Usage: <program> <threads_number>\n");
        return ERROR;
    }

    long threadsNumber = atol(argv[1]);
    if (threadsNumber < THREADS_MIN) {
        printf("Invalid threads number. Minimal number: %d", THREADS_MIN);
        return ERROR;
    }

    return threadsNumber;
}

int main(int argc, char** argv) {
    long threadsNumber = readThreadsNumber(argc, argv);
    if (threadsNumber == ERROR) return ERROR;
    pthread_t threads[threadsNumber];

    double pi = 0;
    int errorCode = createThreadsCalculatingPI(threadsNumber, threads);
    if (errorCode != SUCCESS) return errorCode;
    for (int i = 0; i < threadsNumber; i++) {
        void* partPI;
        errorCode = pthread_join(threads[i], &partPI);
        if (errorCode != SUCCESS) {
            perror("pthread_join");
            return errorCode;
        }
        pi = pi + *((double*) partPI);
        free(partPI);
    }

    pi = pi * 4.0;
    printf("pi done - %.15g \n", pi);
    return SUCCESS;
}
