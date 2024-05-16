#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// #define MAX 1000000

pthread_barrier_t start_barrier;
pthread_barrier_t finish_barrier;

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
    //pthread_barrier_wait(&start_barrier); // Wait for all threads to be created

    Range *range = (Range *)arg; // Get range from argument by dereferencing
    struct timespec start, end;  // Start and end time for each thread

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = range->start; i <= range->end; i++)
    {
        if (checkPrime(i))
        {
            printf("%d\n", i);
        }
    }

    // After finishing the range
    //pthread_barrier_wait(&finish_barrier);

    clock_gettime(CLOCK_MONOTONIC, &end);
    range->exec_time += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    return NULL;
}

int main(int argc, char *argv[])
{
    clock_gettime(CLOCK_MONOTONIC, &start); // Start time for total execution time

    // Check for correct number of arguments
    if (argc != 4)
    {
        printf("Usage: %s (Number of threads) (Start value) (Max Number)\n", argv[0]);
        return 1;
    }

    pthread_mutex_init(&cnt_lock, NULL); // Initialize mutex for cnt

    // Get number of threads and max number from command line arguments
    int numThreads = atoi(argv[1]);
    int begin = atoi(argv[2]);
    int max = atoi(argv[3]);

    // declare array of threads and ranges according to number of threads
    pthread_t threads[numThreads];
    Range ranges[numThreads];
    
    int interval = max - begin;
    int tInterval = interval / numThreads;

    // Initialize barriers
    //pthread_barrier_init(&start_barrier, NULL, numThreads + 1);
    //pthread_barrier_init(&finish_barrier, NULL, numThreads);

    // Create threads and assign ranges
    for (int i = 0; i < numThreads; i++)
    {
        ranges[i].exec_time = 0.0; // Initialize exec_time to 0
        ranges[i].start = i * tInterval + 1 + begin;
        ranges[i].end = (i + 1) * tInterval + begin;
        if (i == numThreads - 1)
            ranges[i].end = max - 1;                                 // Last thread checks up to num - 1, so that last range doesn't exceed max
        pthread_create(&threads[i], NULL, print_primes, &ranges[i]); // Create thread
    }

    // Wait for all threads to be created before starting
    //pthread_barrier_wait(&start_barrier);

    // Join all threads
    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // After all threads are done, destroy barriers
    //pthread_barrier_destroy(&finish_barrier);
    //pthread_barrier_destroy(&start_barrier);

    // Get execution time of each thread and print
    for (int i = 0; i < numThreads; i++)
    {
        printf("Execution time of thread %d: %f seconds\n", i, ranges[i].exec_time);
    }

    // Destroy mutex for cnt
    pthread_mutex_destroy(&cnt_lock);

    // Print total prime numbers, max number, number of threads and total execution time
    printf("Total prime numbers: %d\n", cnt);
    printf("\nThreads: %d\nStart Number: %d\nMax Number: %d\n", numThreads, begin, max);

    // Calculate and print total execution time
    clock_gettime(CLOCK_MONOTONIC, &end);
    double exec_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Total execution time: %f seconds\n", exec_time);

    return 0;
}