#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define ERROR 1
#define SUCCESS 0

#define PTHREAD_CREATE_ERROR 0
#define PTHREAD_JOIN_SUCCESS 0

#define STEPS 2000000000

struct Order {
    int left;
    int right;
};

void* get_pi(void* args) {
    struct Order* order = (struct Order*) args;
    double* pi = (double *) calloc(1, sizeof(double));
    if (pi == NULL) {
        perror("calloc");
        return NULL;
    }

    for (int i = order->left; i <= order->right; ++i) {
        (*pi) += 1.0 / (i * 4.0 + 1.0);
        (*pi) -= 1.0 / (i * 4.0 + 3.0);
    }

    pthread_exit((void *) pi);
}

void get_order(struct Order* order, const int i, const int threads_count) {
    int step = STEPS / threads_count;
    step += (i < STEPS % threads_count);

    order->left = i == 0 ? 0 : step * i + 1;
    order->right = i == 0 ? step : step * (i + 1);
//    printf("i = %i left = %i right = %i\n", i, order->left, order->right);
}

int main(int argc, char* argv[]) {
    if (argc == 1) return ERROR;
    int threads_count = atoi(argv[1]);
    pthread_t threads[threads_count];
    struct Order order[threads_count];

    void* sum[threads_count];
    double result = 0;

    for (int i = 0; i < threads_count; ++i) {
        get_order(&order[i], i, threads_count);
        int create_result = pthread_create(&threads[i], NULL, get_pi, &order[i]);

        if (create_result != PTHREAD_CREATE_ERROR) {
           printf("pthread_create error: couldn't create thread\n");
           return ERROR;
        }
    }

    for (int i = 0; i < threads_count; ++i) {
        int join_result = pthread_join(threads[i], sum + i);

        if (join_result != PTHREAD_JOIN_SUCCESS) {
            printf("pthread_join error\n");
            return ERROR;
        }

        result += *((double *) (sum[i]));
        free(sum[i]);
    }

    result *= 4;
    printf("%f", result);

    return SUCCESS;
}
