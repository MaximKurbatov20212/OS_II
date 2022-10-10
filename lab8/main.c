#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define ERROR 1
#define SUCCESS 0

#define PTHREAD_CREATE_ERROR 0
#define PTHREAD_JOIN_SUCCESS 0
const long long STEPS = 1000000000;

struct Order {
    long long left;
    long long right;
};

struct Info {
    double chunk_sum;
    struct Order order;
};

void* get_pi(void* args) {
    struct Info* info = (struct Info*) args;

    for (long long i = info->order.left; i <= info->order.right; ++i) {
        (info->chunk_sum) += 1.0 / (i * 4.0 + 1.0);
        (info->chunk_sum) -= 1.0 / (i * 4.0 + 3.0);
    }
}

void get_order(struct Order* order, const int i, const int threads_count) {
    long long step = STEPS / threads_count;   // 
    step += (i < STEPS % threads_count); // количество итераций у потока
    long long count = STEPS % threads_count; // количество потоков, у которых больше интераций, чем у остальныx
        
    if (i < count) { // если наш поток получил больше итераций
        order->left = step * i;
        order->right = step * (i + 1) - 1;
//        printf("i = %i left = %i right = %i\n", i, order->left, order->right);
        return;
    }

    order->left = ((step + 1) * count) + (step * (i - count));
    order->right = order->left + step - 1;
//    printf("i = %i left = %i right = %i\n", i, order->left, order->right);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Invalid number of args. Usage: ./*.exe <number of threads>") ;
        return ERROR;
    }

    int threads_count = atoi(argv[1]);
    if (threads_count == 0) {
        printf("atoi error") ;
        return ERROR;
    }

    pthread_t threads[threads_count];
    struct Info* info = (struct Info*) malloc(threads_count * sizeof(struct Info));
    double result = 0;

    for (int i = 0; i < threads_count; ++i) {
        get_order(&(info[i].order), i, threads_count);
        info[i].chunk_sum = 0;

        int create_result = pthread_create(&threads[i], NULL, get_pi, &info[i]);

        if (create_result != PTHREAD_CREATE_ERROR) {
           printf("pthread_create error: couldn't create thread\n");
           return ERROR;
        }
    }

    for (int i = 0; i < threads_count; ++i) {
        int join_result = pthread_join(threads[i], NULL);

        if (join_result != PTHREAD_JOIN_SUCCESS) {
            printf("pthread_join error\n");
            return ERROR;
        }

        result += info[i].chunk_sum;
    }

    result *= 4;
    printf("%f", result);
    free(info);
    return SUCCESS;
}
