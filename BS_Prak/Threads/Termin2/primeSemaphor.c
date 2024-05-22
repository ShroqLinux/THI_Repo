#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int max;
int numThreads;
int currentVal = 2;
int totalPrimes = 0;

pthread_mutex_t value_lck;
pthread_mutex_t cnt_lck;

bool checkPrime(int a)
{
    for (int i = 2; i < a; i++)
    {
        if (a % i == 0)
        {
            return false;
        }
    }
    return true;
}

void *allPrimes(void *args)
{
    double *execTime = (double *)args;
    // clock_t start = clock();

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int thisValue;

    while (currentVal < max)
    {
        pthread_mutex_lock(&value_lck);
        thisValue = currentVal;
        currentVal++;
        pthread_mutex_unlock(&value_lck);

        if (checkPrime(thisValue))
        {
            pthread_mutex_lock(&cnt_lck);
            totalPrimes++;
            pthread_mutex_unlock(&cnt_lck);
        }
    }

    // clock_t end = clock();
    clock_gettime(CLOCK_MONOTONIC, &end);

    //*execTime = (double)(end - start) / CLOCKS_PER_SEC;
    *execTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    return NULL;
}

int main(int argc, char *argv[])
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    // clock_t start = clock();

    if (argc != 3)
    {
        printf("Usage: %s (Number of threads) (Max Number)\n", argv[0]);
        return 1;
    }

    int numThreads = atoi(argv[1]);
    max = atoi(argv[2]);

    double execTimes[numThreads];
    pthread_t threads[numThreads];

    int ret;

    ret = pthread_mutex_init(&value_lck, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to initialize value_lck: %s\n", strerror(ret));
        return 1;
    }

    ret = pthread_mutex_init(&cnt_lck, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to initialize cnt_lck: %s\n", strerror(ret));
        return 1;
    }

    for (int i = 0; i < numThreads; i++)
    {
        pthread_create(&threads[i], NULL, allPrimes, &execTimes[i]);
    }

    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    ret = pthread_mutex_destroy(&value_lck);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to destroy value_lck: %s\n", strerror(ret));
        return 1;
    }

    ret = pthread_mutex_destroy(&cnt_lck);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to destroy cnt_lck: %s\n", strerror(ret));
        return 1;
    }

    for (int i = 0; i < numThreads; i++)
    {
        printf("Execution time thread %d: %f\n", i, execTimes[i]);
    }

    // clock_t end = clock();

    clock_gettime(CLOCK_MONOTONIC, &end);
    double totalExecTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    // double totalExecTime = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Total prime numbers: %d\n", totalPrimes);
    printf("Total Execution time: %f\n", totalExecTime);

    return 0;
}