#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>

int max;
int numThreads;
int currentVal = 2;
int totalPrimes = 0;

sem_t cnt_sem;
sem_t val_sem;

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
        sem_wait(&val_sem);
        thisValue = currentVal;
        currentVal++;
        sem_post(&val_sem);

        if (checkPrime(thisValue))
        {
            sem_wait(&cnt_sem);
            totalPrimes++;
            sem_post(&cnt_sem);
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

    ret = sem_init(&val_sem, 0, 1);

    if (ret == -1)
    {
        perror("val_sem failed");
        return 1;
    }

    ret = sem_init(&cnt_sem, 0, 1);

    if (ret == -1)
    {
        perror("cnt_sem failed");
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

    for (int i = 0; i < numThreads; i++)
    {
        printf("Execution time thread %d: %f\n", i, execTimes[i]);
    }

    ret = sem_destroy(&val_sem);

    if (ret == -1)
    {
        perror("Failed to destroy val_sem");
        return 1;
    }

    ret = sem_destroy(&cnt_sem);

    if (ret == -1)
    {
        perror("Failed to destroy cnt_sem");
        return 1;
    }

    // clock_t end = clock();

    clock_gettime(CLOCK_MONOTONIC, &end);
    double totalExecTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    // double totalExecTime = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Total prime numbers: %d\n", totalPrimes);
    printf("Total Execution time: %f\n", totalExecTime);

    return 0;
}