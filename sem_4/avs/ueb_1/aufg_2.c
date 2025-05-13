#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define FIRST 1
#define SECOND 2
#define THIRD 3

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};


char* sem_handle = "./sem_key";
int sem_id = 0;

int init_sem() {
    key_t key = ftok(sem_handle, 5);
    if(key == -1) {
        perror("ftok failed");
        exit(1);
    }

    sem_id = semget(key, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget failed");
        exit(1);
    }

    union semun sem_union;
    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
        perror("semctl SETVAL failed");
        exit(1);
    }

    return 0;
}


int P () {
    struct sembuf semaphore;

    semaphore.sem_num = 0;
    semaphore.sem_op=-1;  //P
    semaphore.sem_flg =~ (IPC_NOWAIT|SEM_UNDO);

    if (semop(sem_id, &semaphore, 1) < 0) { // sem_id provided by return value of semget()
        perror("Error in semop() at V()");
        exit(1);
    }
}

int V () {
    struct sembuf semaphore;

    semaphore.sem_num = 0;
    semaphore.sem_op=1;  //V
    semaphore.sem_flg =~ (IPC_NOWAIT|SEM_UNDO);

    if (semop(sem_id, &semaphore, 1) < 0) { // sem_id provided by return value of semget()
        perror("Error in semop() at V()");
        exit(1);
    }
}

void func_crit(int id) {
    P();
    printf("Child process ID %d entering critical section\n", id);
    sleep(1);
    printf("Child process ID %d exiting critical section\n", id);
    V();
}

int main() {

    if (init_sem() != 0) {
        perror("init_sem failed");
        exit(1);
    }
    
    int res = fork();

    if (res == -1) {
        printf("Fork failed with return -1\n");
    }
    else if (res > 0){
        printf("Parent process with PID: %d\n", getpid());
    }

    else {
        printf("Child process with PID: %d\n", FIRST);

        func_crit(FIRST);

        res = fork();


        if (res == -1) {
            printf("Second Fork failed with return -1\n");
        }
    
        if (res > 0) {
            printf("Parent process with PID: %d\n", getpid());
            
        }
        else {
            printf("Second child process with PID: %d\n", SECOND);

            func_crit(SECOND);

            res = fork();

            if (res == -1) {
                printf("Third fork failed with return -1\n");
            }
            if (res > 0) {
                printf("Parent process with PID: %d\n", getpid());
            }
            else {
                printf("Third child process with PID: %d\n", THIRD);
                func_crit(THIRD);

            }
        }
    }
}