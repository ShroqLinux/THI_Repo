#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    const char *name = "/my_mem"; // Name of shared memory
    const int SIZE = 4096;        // Size of shared memory to be set with ftruncate

    int shm_fd; // Shared mem file descriptor
    void *ptr;  // Pointer to shared memory object, read and write access
                // done through this pointer

    // Create shared mem object
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open call failed!\n");
        return 1;
    }

    // Configure size of shared memory object
    if (ftruncate(shm_fd, SIZE) == -1)
    {
        perror("ftruncate: Size of shared memory could not be changed!\n");
        return 1;
    }

    // Memory map to shared object
    ptr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap failed!\n");
        return 1;
    }

    // Write to shared memory object, ptr specifies which address to write to
    sprintf(ptr, "Hello World");

    // Read from shared memory object
    printf("%s\n", (char *)ptr);

    // Remove shared mem object
    if (shm_unlink(name) == -1)
    {
        perror("shm_unlink failed!\n");
        return 1;
    }

    if (munmap(ptr, SIZE) == -1)
    {
        perror("munmap failed!\n");
        return 1;
    }

    return 0;
}