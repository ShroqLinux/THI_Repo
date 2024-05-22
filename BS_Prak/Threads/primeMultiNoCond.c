#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

int current_number = 2; // Start from the first prime number
int max_number;
pthread_mutex_t number_lock;
int numThreads;
/*
pthread_mutex_t finish_lock;
pthread_cond_t finish_cond;
int finished_threads = 0;
*/
int cnt = 0;
pthread_mutex_t cnt_lock;

typedef struct Range
{
    double exec_time;
} Range;

bool checkPrime(int a)
{
    if (a <= 1)
    {
        return false;
    }

    for (int i = 2; i <= a - 1; i++)
    {
        if (a % i == 0)
        {
            return false;
        }
    }

    return true;
}

void *print_primes(void *arg)
{
    Range *range = (Range *)arg;
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    while (1)
    {
        
        pthread_mutex_lock(&number_lock);

        if (current_number >= max_number)
        {
            pthread_mutex_unlock(&number_lock);

            // After finishing the range
            /*
            pthread_mutex_lock(&finish_lock);
            finished_threads++;
            if (finished_threads == numThreads)
            {
                // If this is the last thread to finish, signal the main thread
                pthread_cond_signal(&finish_cond);
            }
            pthread_mutex_unlock(&finish_lock);
            */
            clock_gettime(CLOCK_MONOTONIC, &end);
            range->exec_time += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
            return NULL;
        }

        int number = current_number;
        current_number++;

        pthread_mutex_unlock(&number_lock);


        if (checkPrime(number))
        {
            pthread_mutex_lock(&cnt_lock);
            cnt++;
            pthread_mutex_unlock(&cnt_lock);
        }

        
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s (Number of threads) (Max Number)\n", argv[0]);
        return 1;
    }

    numThreads = atoi(argv[1]);
    max_number = atoi(argv[2]);

    pthread_t threads[numThreads];
    Range ranges[numThreads];

    pthread_mutex_init(&number_lock, NULL);
    pthread_mutex_init(&cnt_lock, NULL);
    /*
    // Initialize the mutex and condition variable
    pthread_mutex_init(&finish_lock, NULL);
    pthread_cond_init(&finish_cond, NULL);
    */
    for (int i = 0; i < numThreads; i++)
    {
        ranges[i].exec_time = 0.0; // Initialize exec_time to 0
        pthread_create(&threads[i], NULL, print_primes, &ranges[i]);
    }
    /*
    // Wait for all threads to finish
    pthread_mutex_lock(&finish_lock);
    while (finished_threads < numThreads)
    {
        pthread_cond_wait(&finish_cond, &finish_lock);
    }
    pthread_mutex_unlock(&finish_lock);
    */
    pthread_mutex_destroy(&number_lock);
    pthread_mutex_destroy(&cnt_lock);

    // First loop to join all threads
    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    /*
     // Destroy the mutex and condition variable
    pthread_mutex_destroy(&finish_lock);
    pthread_cond_destroy(&finish_cond);
    */
    // Second loop to print the execution times
    for (int i = 0; i < numThreads; i++)
    {
        printf("Execution time of thread %d: %f seconds\n", i, ranges[i].exec_time);
    }
    
    printf("%d Prime numbers\n", cnt);

    return 0;
}