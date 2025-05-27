#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>

#define ONE 1
#define TWO 2
#define THREE 3
#define FOUR 4
#define FIVE 5

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
    int nsems = 5;

    key_t key = ftok(sem_handle, 5);
    if(key == -1) {
        perror("ftok failed");
        exit(1);
    }

    sem_id = semget(key, nsems, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget failed");
        exit(1);
    }

    union semun sem_union;
    sem_union.val = 1;

    for (int i = 0; i < nsems; i++) {
        if (semctl(sem_id, i, SETVAL, 1) == -1) {
            perror("semctl SETVAL failed");
            exit(1);
        } 
    }
    

    return 0;
}

int P (int left_fork, int right_fork) {
    struct sembuf semaphore[2];

    semaphore[0].sem_num = left_fork;
    semaphore[0].sem_op=-1;  //P
    semaphore[0].sem_flg =~ (IPC_NOWAIT|SEM_UNDO);

    semaphore[0].sem_num = right_fork;
    semaphore[0].sem_op=-1;  //P
    semaphore[0].sem_flg =~ (IPC_NOWAIT|SEM_UNDO);

    if (semop(sem_id, semaphore, 2) < 0) { // sem_id provided by return value of semget()
        perror("Error in semop() at V()");
        exit(1);
    }
}

int V (int left_fork, int right_fork) {
    struct sembuf semaphore[2];

    semaphore[0].sem_num = left_fork;
    semaphore[0].sem_op=1;  //V
    semaphore[0].sem_flg =~ (IPC_NOWAIT|SEM_UNDO);

    semaphore[0].sem_num = right_fork;
    semaphore[0].sem_op=1;  //V
    semaphore[0].sem_flg =~ (IPC_NOWAIT|SEM_UNDO);

    if (semop(sem_id, semaphore, 2) < 0) { // sem_id provided by return value of semget()
        perror("Error in semop() at V()");
        exit(1);
    }
}

void eat(int time) {
    int i = 0;

    while (i < time * 100) {
        i++;
    }
}

void think(int time) {
    int i = 0;

    while (i < time * 100) {
        i++;
    }
}

void philosopher(int id, int left_fork, int right_fork) {
    int eat_time, think_time = rand();

    while (1) {
        think(think_time);
        printf("Phil %d is thinking\n", id);
        sleep(rand() % 5);

        P(left_fork, right_fork);
        printf("Phil %d got forks\n", id);
        eat(eat_time);
        V(left_fork, right_fork);
        sleep(rand() % 5);
    }
}

int main() {
    srand(1);

    if (init_sem() != 0) {
        perror("init_sem failed");
        exit(1);
    }
    
    int res = fork();

    if (res == -1) {
        printf("Fork failed with return -1\n");
    }
    else if (res > 0){
        printf("Philsopher with ID: %d\n", ONE);
        philosopher(ONE, 1, 2);
        
    }

    else {
        printf("Philsopher with ID: %d\n", TWO);
        philosopher(TWO, 2, 3);

        res = fork();


        if (res == -1) {
            printf("Second Fork failed with return -1\n");
        }
    
        if (res > 0) {
            printf("Philsopher with ID: %d\n", THREE);
            philosopher(THREE, 3, 4);

        }
        else {
            printf("Philsopher with ID: %d\n", FOUR);
            philosopher(ONE, 4, 0);



            // func_crit(SECOND);

            res = fork();

            if (res == -1) {
                printf("Third fork failed with return -1\n");
            }
            if (res > 0) {
                printf("Philsopher with ID: %d\n", FIVE);


            }
            else {
                // do nothing

            }
        }
    }
}