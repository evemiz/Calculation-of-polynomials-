//consumer Q3

#include <stdio.h>

#include <string.h>

#include <stdlib.h>

#include <semaphore.h>

#include <sys/ipc.h>

#include <sys/shm.h>

#include <pthread.h>

#define SHM_SIZE 1280
#define SEM_MUTEX "/a_mutex"
#define SEM_FULL "/a_full"
#define SEM_EMPTY "/a_empty"
#define INPUT_SIZE 128

typedef struct {
    int deg1;
    int * first;
    int deg2;
    int * second;
}
        Data;

typedef struct {
    int deg;
    int firstNum;
    int secondNum;
    int op; //if equal 0 - ADD , if equal 1 - SUB
}
        Poly;

void * calc_threads(void * data);
int MUL(const int * a, int a_size,
        const int * b, int b_size);
int * parseInput(const char * input, int deg, int * input_index);

int main() {
    char temp_input[INPUT_SIZE]; //temp string to help with the input string
    int add=0; //if equal 1 - this is an add operation
    int sub=0; //if equal 1 - this is a sub operation
    int mul=0; //if equal 1 - this is a mul operation

    //semaphores

    //open existing semaphore - the mutex
    sem_t * mutex = sem_open(SEM_MUTEX, 0);
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        sem_close(mutex);
        sem_unlink(SEM_MUTEX);
        exit(1);
    }

    //open existing semaphore - the full - the number of the full places in the memory
    sem_t * full = sem_open(SEM_FULL, 0);
    if (full == SEM_FAILED) {
        perror("sem_open");
        sem_close(mutex);
        sem_unlink(SEM_MUTEX);
        sem_close(full);
        sem_unlink(SEM_FULL);
        exit(1);
    }

    //open existing semaphore - the empty - the number of the empty places in the memory
    sem_t * empty = sem_open(SEM_EMPTY, 0);
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
        sem_close(mutex);
        sem_unlink(SEM_MUTEX);
        sem_close(full);
        sem_unlink(SEM_FULL);
        sem_close(empty);
        sem_unlink(SEM_EMPTY);
        exit(1);
    }

    int shm_id = shmget(key, SHM_SIZE, 0600);
    if (shm_id == -1) {
        perror("shmget");
        sem_close(mutex);
        sem_unlink(SEM_MUTEX);
        sem_close(full);
        sem_unlink(SEM_FULL);
        sem_close(empty);
        sem_unlink(SEM_EMPTY);
        exit(1);
    }

    //attach the process to the share memory - shm_ptr is a pointer to the memory
    char * shm_ptr = (char * ) shmat(shm_id, NULL, SHM_RDONLY);
    if (shm_ptr == (void*)-1) {
        perror("shmat");
        sem_close(mutex);
        sem_unlink(SEM_MUTEX);
        sem_close(full);
        sem_unlink(SEM_FULL);
        sem_close(empty);
        sem_unlink(SEM_EMPTY);
        shmdt(shm_ptr);
        exit(1);
    }

    int ind = 0;
    int temp = 0;
    while (1) {
        sem_wait(full);
        sem_wait(mutex);

        // Critical section
        ind = temp;
        Data data;
        add = 0;
        sub = 0;
        mul = 0;

        //in a case the user enter "END" - exit the program
        if (shm_ptr[ind] == 'E' && shm_ptr[ind + 1] == 'N' && shm_ptr[ind + 2] == 'D')
            break;

        ind++;

        //first degree
        int l = 0;
        while (shm_ptr[ind] != ':') {
            temp_input[l++] = shm_ptr[ind++];
        }
        temp_input[l] = '\0';
        char * end;
        data.deg1 = (int) strtol(temp_input, & end, 10);

        if ( * end != '\0')
            exit(1);

        //the first polynomial numbers
        ind++; //the index after the ':'
        data.first = parseInput(shm_ptr, data.deg1, & ind);

        //the operation
        l = 0;
        while (shm_ptr[ind] != '(') {
            temp_input[l++] = shm_ptr[ind++];
        }
        temp_input[l] = '\0';
        ind++;

        if (strcmp(temp_input, "ADD") == 0)
            add = 1;
        if (strcmp(temp_input, "SUB") == 0)
            sub = 1;
        if (strcmp(temp_input, "MUL") == 0)
            mul = 1;

        //second degree
        l = 0;
        while (shm_ptr[ind] != ':') {
            temp_input[l++] = shm_ptr[ind++];
        }
        temp_input[l] = '\0';
        data.deg2 = (int) strtol(temp_input, & end, 10);

        if ( * end != '\0')
            exit(1);

        //the second polynomial numbers
        ind++; //the index after the ':'
        data.second = parseInput(shm_ptr, data.deg2, & ind);

        //if the operation is mul
        if (mul)
            MUL(data.first, data.deg1 + 1, data.second, data.deg2 + 1);
        else {
            int max; //max will be the degree of the result polynomial
            if (data.deg1 > data.deg2)
                max = data.deg1;
            else
                max = data.deg2;
            max++;
            //creat a threads array of size of the polynomial degree
            pthread_t * threads_arr = (pthread_t * ) malloc(max * sizeof(pthread_t));
            int status;
            int ** res = (int ** ) malloc(max * sizeof(int * )); //the result array
            Poly * p_arr = (Poly * ) malloc(max * sizeof(Poly)); //polynomial (struct) array

            int k1 = data.deg1; //k1 is the last index of the first polynomial (the degree is 0)
            int k2 = data.deg2; //k2 is the first index of the first polynomial (the degree is 0)

            //a loop to create all the polynomials in the array
            for (int j = 0; j < max; ++j) {
                p_arr[j].deg = j;
                if (k1 >= 0)
                    p_arr[j].firstNum = data.first[k1];
                else //if the degree is bigger then the one in the polynomial - the num will be zero
                    p_arr[j].firstNum = 0;
                if (k2 >= 0)
                    p_arr[j].secondNum = data.second[k2];
                else //if the degree is bigger then the one in the polynomial - the num will be zero
                    p_arr[j].secondNum = 0;
                if (add)
                    p_arr[j].op = 0;
                else
                    p_arr[j].op = 1;
                k1--;
                k2--;
            }

            // a loop to create threads - the amount is the degree of the result polynomial
            for (int j = 0; j < max; ++j) {
                status = pthread_create( & threads_arr[j], NULL, calc_threads, (void * ) & p_arr[j]);
                if (status != 0) {
                    fputs("pthread create failed", stderr);
                    exit(1);
                }
            }

            // a loop to join the threads
            for (int j = 0, k = max - 1; j < max; ++j) {
                pthread_join(threads_arr[j], (void ** ) & res[k--]);
            }

            //print
            int zero = 0;
            for (int i = 0; i < max; ++i) {
                if ( * res[i] != 0) {
                    zero = 1;
                    break;
                }
            }

            if (zero != 0) {
                int not_only = 0;
                int deg = max - 1;
                for (int k = 0; k < max; ++k) {
                    if ( * res[k] > 0 && k != 0 && not_only == 1) {
                        printf(" + ");
                        not_only = 0;
                    }
                    if ( * res[k] < 0 && k != 0 && not_only == 1) {
                        printf(" - ");
                        * res[k] = (( * res[k]) * (-1));
                        not_only = 0;
                    }

                    if ( * res[k] == 0) {
                        deg--;
                        continue;
                    }
                    not_only = 1;
                    if (deg == 0)
                        printf("%d", * res[k]);
                    else if (deg == 1)
                        printf("%dx", * res[k]);
                    else
                        printf("%dx^%d", * res[k], deg);
                    deg--;
                }
                printf("\n");
            }
                //if the answer is zero - print 0
            else {
                printf("%d\n", 0);
            }
            free(threads_arr);
            for (int j = 0; j < max; ++j) {
                free(res[j]);
            }
            free(res);
            free(p_arr);
        }

        free(data.first);
        free(data.second);

        temp += INPUT_SIZE;
        if (temp == SHM_SIZE)
            temp = 0;
        sem_post(mutex);
        sem_post(empty);

    }
    //cleaning...
    shmdt(shm_ptr);
    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_FULL);
    sem_unlink(SEM_EMPTY);

    return 0;
}

//the thread function - return the result of a specific degree
void * calc_threads(void * data) {
    Poly p = * ((Poly * ) data);
    int * num;
    num = (int * ) malloc(sizeof(int));
    if (p.op == 0) {
        * num = p.firstNum + p.secondNum;
    } else {
        * num = p.firstNum - p.secondNum;
    }
    pthread_exit((void * ) num);
}

//the operation - mul - function + print the result
int MUL(const int * a, int a_size,
        const int * b, int b_size) {

    int len = (a_size) + (b_size) - 1; //len is the degree of the new polynomial
    int * res = (int * ) malloc(len * sizeof(int)); //the result array

    //clear the res array
    for (int i = 0; i < len; ++i) {
        res[i] = 0;
    }

    //calculate the multiplication between the polynomial
    int arr_ind;
    for (int i = 0; i < a_size; ++i) {
        arr_ind = i;
        for (int j = 0; j < b_size; ++j) {
            res[arr_ind++] += (a[i] * b[j]);
        }
    }
    //check if the result is zero
    int zero = 0;
    for (int i = 0; i < len; ++i) {
        if (res[i] != 0) {
            zero = 1;
            break;
        }
    }
    //if the result is not zero - print it
    if (zero != 0) {
        //print

        int not_only = 0;
        int deg = len - 1;
        for (int k = 0; k < len; ++k) {
            if (res[k] > 0 && k != 0 && not_only == 1) {
                printf(" + ");
                not_only = 0;
            }
            if (res[k] < 0 && k != 0 && not_only == 1) {
                printf(" - ");
                res[k] = (res[k] * (-1));
                not_only = 0;
            }

            if (res[k] == 0) {
                deg--;
                continue;
            }
            not_only = 1;
            if (deg == 0)
                printf("%d", res[k]);
            else if (deg == 1)
                printf("%dx", res[k]);
            else
                printf("%dx^%d", res[k], deg);
            deg--;
        }
        printf("\n");
    }
        //if the result is zero - print only '0'
    else {
        printf("%d\n", 0);
    }
    free(res);
    return 0;
}

//function that return the array of the polynomial
int * parseInput(const char * input, int deg, int * input_index) {
    int * to_return = (int * ) malloc((deg + 1) * sizeof(int));
    int minus = 0;

    for (int i = 0; i <= deg; ++i) {
        int j = 0;
        char temp_input[100]; // Adjust the size as per your requirement

        while (input[ * input_index] != ',' && input[ * input_index] != ')') {
            if (input[ * input_index] == '-') {
                minus = 1;
                ( * input_index) ++;
            } else
                temp_input[j++] = input[( * input_index) ++];
        }
        temp_input[j] = '\0';
        char * end;
        to_return[i] = (int) strtol(temp_input, & end, 10);

        if ( * end != '\0')
            exit(1);
        if (minus == 1) {
            to_return[i] *= -1;
            minus = 0;
        }
        ( * input_index) ++;
    }

    return to_return;
}