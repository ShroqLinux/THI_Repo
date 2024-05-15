#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// #define MAX 1000000

int numThreads;

pthread_barrier_t start_barrier;

pthread_mutex_t finish_lock;
pthread_cond_t finish_cond;
int finished_threads = 0;

int cnt = 0;
pthread_mutex_t cnt_lock;

struct timespec start, end;

typedef struct Range
{
    int start;
    int end;
    double exec_time;
} Range;


bool checkPrime(int a)
{
    if (a <= 1)
    {
        return false;
    }

    // int sqrtA = sqrt(a); -> optimization is too fast for run time difference
    // between single and multi-threaded version

    // a/2 can be used for time difference between single and multi-threaded version
    for (int i = 2; i <= a - 1; i++)
    {
        if (a % i == 0)
        {
            return false;
        }
    }

    pthread_mutex_lock(&cnt_lock);
    cnt++;
    pthread_mutex_unlock(&cnt_lock);

    return true;
}

void *print_primes(void *arg)
{
    pthread_barrier_wait(&start_barrier);

    Range *range = (Range *)arg;
    struct timespec start, end;

    for (int i = range->start; i <= range->end; i++)
    {
        if (checkPrime(i))
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
            printf("%d\n", i);
            clock_gettime(CLOCK_MONOTONIC, &end);
            range->exec_time += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
        }
    }

    // After finishing the range
    pthread_mutex_lock(&finish_lock);
    finished_threads++;
    if (finished_threads == numThreads)
    {
        // If this is the last thread to finish, signal the main thread
        pthread_cond_signal(&finish_cond);
    }
    pthread_mutex_unlock(&finish_lock);

    return NULL;
}

int main(int argc, char *argv[])
{
    clock_gettime(CLOCK_MONOTONIC, &start); // Start time
    pthread_mutex_init(&cnt_lock, NULL);

    if (argc != 3)
    {
        printf("Usage: %s (Number of threads) (Max Number)\n", argv[0]);
        return 1;
    }

    numThreads = atoi(argv[1]);
    int max = atoi(argv[2]);

    pthread_t threads[numThreads];
    Range ranges[numThreads];

    int interval = max / numThreads;

    // Initialize the mutex and condition variable
    pthread_mutex_init(&finish_lock, NULL);
    pthread_cond_init(&finish_cond, NULL);

    pthread_barrier_init(&start_barrier, NULL, numThreads + 1);

    for (int i = 0; i < numThreads; i++)
    {
        ranges[i].exec_time = 0.0; // Initialize exec_time to 0
        ranges[i].start = i * interval + 1;
        ranges[i].end = (i + 1) * interval;
        if (i == numThreads - 1)
            ranges[i].end = max - 1; // Last thread checks up to num - 1
        pthread_create(&threads[i], NULL, print_primes, &ranges[i]);
    }

    pthread_barrier_wait(&start_barrier);

    // Wait for all threads to finish
    pthread_mutex_lock(&finish_lock);
    while (finished_threads < numThreads)
    {
        pthread_cond_wait(&finish_cond, &finish_lock);
    }
    pthread_mutex_unlock(&finish_lock);

    // Join all threads
    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&start_barrier);

    for (int i = 0; i < numThreads; i++)
    {
        printf("Execution time of thread %d: %f seconds\n", i, ranges[i].exec_time);
    }

    
    pthread_mutex_destroy(&cnt_lock);
    printf("Total prime numbers: %d\n", cnt);
    printf("\nThreads: %d, Max Number: %d\n", numThreads, max);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double exec_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Total execution time: %f seconds\n", exec_time);

    // Destroy the mutex and condition variable
    pthread_mutex_destroy(&finish_lock);
    pthread_cond_destroy(&finish_cond);

    return 0;
}