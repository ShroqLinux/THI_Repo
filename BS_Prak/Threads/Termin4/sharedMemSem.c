#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("Usage: %s (Iterations)\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    int iter = atoi(argv[1]);

    const char *name = "/int_shared";
    const char *semName = "/sem1";
    const int SIZE = 4096;

    int shm_fd;
    void *ptr;

    sem_t *sem;

    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open failed\n");
        return 1;
    }

    if (ftruncate(shm_fd, SIZE) == -1)
    {
        perror("ftruncate failed\n");
        return 1;
    }

    ptr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("Map failed\n");
        return 1;
    }

    sem = sem_open(semName, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED)
    {
        perror("sem failed\n");
        return 1;
    }

    if (fork() == 0)
    {
        for (size_t i = 0; i < iter; i++)
        {
            int value = (rand() % 1000);

            int* payload = (int*) ptr;
            // addition: add read with second semaphore
            // printf("Read in write process (PID: %d): %d\n", getpid(), *payload);

            printf("Write process (PID: %d): %d\n", getpid(), value);
            sem_wait(sem);

            memcpy(ptr, &value, sizeof(value));
            sem_post(sem);
            usleep(1000 * 1500);
            // sleep(1);
        }
    }
    else
    {
        for (size_t i = 0; i < iter; i++)
        {

            // additional: add write with second semaphore
            usleep(1000 * 1500);
            // sleep(1);
            sem_wait(sem);

            int* payload = (int*) ptr;
            printf("Read process (PID: %d): %d\n", getpid(), *payload);
            /*
            *payload = *(payload) * 2;
            memcpy(ptr, &payload, sizeof(payload));
            */

            sem_post(sem);
            fflush(stdout);
        }

        if (shm_unlink(name) == -1)
        {
            perror("shm_unlink failed\n");
            return 1;
        }

        if (munmap(ptr, SIZE) == -1)
        {
            perror("munmap failed\n");
            return 1;
        }

        if (sem_close(sem) == -1)
        {
            perror("sem_close failed\n");
            return 1;
        }

        if (sem_unlink(semName) == -1)
        {
            perror("sem_unlink failed\n");
            return 1;
        }
    }

    return 0;
}