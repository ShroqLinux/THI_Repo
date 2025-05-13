#include <stdio.h>
#include <unistd.h>

#define FIRST 1
#define SECOND 2
#define THIRD 3

int func_crit(int id) {
    printf("Child process ID %d entering critical section\n", id);
    sleep(2);
    printf("Child process ID %d exiting critical section\n", id);
}

int main() {
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