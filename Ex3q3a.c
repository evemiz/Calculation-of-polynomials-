//producer Q2

#include <stdio.h>

#include <string.h>

#include <stdlib.h>

#include <fcntl.h>

#include <semaphore.h>

#include <sys/ipc.h>

#include <sys/shm.h>

#include <unistd.h>

#define SHM_SIZE 1280
#define INPUT_SIZE 128
#define SEM_MUTEX "/a_mutex"
#define SEM_FULL "/a_full"
#define SEM_EMPTY "/a_empty"

struct shmid_ds shm_desc;

int main() {
    char input[INPUT_SIZE]; //the input from user

    //semaphores
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_FULL);
    sem_unlink(SEM_EMPTY);

    //create new semaphore - the mutex
    sem_t * mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        sem_close(mutex);
        sem_unlink(SEM_MUTEX);
        exit(1);
    }

    //create new semaphore - the full - the number of the full places in the memory
    sem_t * full = sem_open(SEM_FULL, O_CREAT, 0666, 0);
    if (full == SEM_FAILED) {
        perror("sem_open");
        sem_close(mutex);
        sem_unlink(SEM_MUTEX);
        sem_close(full);
        sem_unlink(SEM_FULL);
        exit(1);
    }

    //create new semaphore - the empty - the number of the empty places in the memory
    sem_t * empty = sem_open(SEM_EMPTY, O_CREAT, 0666, 10);
    if (empty == SEM_FAILED) {
        perror("sem_open");
        sem_close(mutex);
        sem_unlink(SEM_MUTEX);
        sem_close(full);
        sem_unlink(SEM_FULL);
        sem_close(empty);
        sem_unlink(SEM_EMPTY);
        exit(1);
    }

    //shared memory

    //create the outer key
    key_t key = ftok("/tmp", 'Z');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    //create the share memory
    int shm_id = shmget(key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0600);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    //attach the process to the share memory - shm_ptr is a pointer to the memory
    char * shm_ptr = (char * ) shmat(shm_id, NULL, 0);
    if (shm_ptr == (void*)-1) {
        perror("shmat");
        shmdt(shm_ptr);
        shmctl(shm_id, IPC_RMID, & shm_desc);
        exit(1);
    }

    int i = 0;
    while (1) {
        sem_wait(empty);
        sem_wait(mutex);

        //critical section
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';
        strcpy( & shm_ptr[i], input);

        sem_post(mutex);
        sem_post(full);

        //check if the input is END - then finish
        if (strcmp( & shm_ptr[i], "END") == 0)
            break;
        i += INPUT_SIZE;
        if (i == SHM_SIZE)
            i = 0;
        sleep(1);
    }

    //detach the pointer
    shmdt(shm_ptr);

    //free the shared memory
    if (shmctl(shm_id, IPC_RMID, & shm_desc) == -1) {
        perror("shmctl");
        exit(1);
    }

    //cleaning...
    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_FULL);
    sem_unlink(SEM_EMPTY);

    return 0;
}