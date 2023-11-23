//consumer Q2

#include <stdio.h>

#include <string.h>

#include <stdlib.h>

#include <semaphore.h>

#include <sys/ipc.h>

#include <sys/shm.h>

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

int ADD(const int * a, int a_size,
        const int * b, int b_size);
int SUB(const int * a, int a_size,
        const int * b, int b_size);
int MUL(const int * a, int a_size,
        const int * b, int b_size);
int calc(const int * a, int a_size,
         const int * b, int b_size, int( * callback)(const int * , int,
                                                     const int * , int));
void print(int max, int * res);
int * parseInput(const char * input, int deg, int * input_index);
int zeros(int max,
          const int * res);

int main() {
    char temp_input[INPUT_SIZE]; //temp string to help with the input string
    int add; //if equal 1 - this is an add operation
    int sub; //if equal 1 - this is a sub operation
    int mul; //if equal 1 - this is a mul operation

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
        int i = 0;
        while (shm_ptr[ind] != ':') {
            temp_input[i++] = shm_ptr[ind++];
        }
        temp_input[i] = '\0';
        char * end;
        data.deg1 = (int) strtol(temp_input, & end, 10);

        if ( * end != '\0')
            exit(1);

        //the first polynomial numbers
        ind++; //the index after the ':'
        data.first = parseInput(shm_ptr, data.deg1, & ind);

        //the operation
        i = 0;
        while (shm_ptr[ind] != '(') {
            temp_input[i++] = shm_ptr[ind++];
        }
        temp_input[i] = '\0';
        ind++;

        if (strcmp(temp_input, "ADD") == 0)
            add = 1;
        if (strcmp(temp_input, "SUB") == 0)
            sub = 1;
        if (strcmp(temp_input, "MUL") == 0)
            mul = 1;

        //second degree
        i = 0;
        while (shm_ptr[ind] != ':') {
            temp_input[i++] = shm_ptr[ind++];
        }
        temp_input[i] = '\0';
        data.deg2 = (int) strtol(temp_input, & end, 10);

        if ( * end != '\0')
            exit(1);

        //the second polynomial numbers
        ind++; //the index after the ':'
        data.second = parseInput(shm_ptr, data.deg2, & ind);

        //goto the function of the operation
        if (add)
            calc(data.first, data.deg1 + 1, data.second, data.deg2 + 1, ADD);
        else if (sub)
            calc(data.first, data.deg1 + 1, data.second, data.deg2 + 1, SUB);
        else if (mul)
            calc(data.first, data.deg1 + 1, data.second, data.deg2 + 1, MUL);

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

//the operation - add - function
int ADD(const int * a, int a_size,
        const int * b, int b_size) {
    int max; //max will be the degree of the result polynomial
    if (a_size > b_size)
        max = a_size;
    else
        max = b_size;

    int * res = (int * ) malloc(max * sizeof(int)); //the result array

    int index = max - 1;
    a_size--;
    b_size--;
    while (1) {
        if (a_size >= 0 && b_size >= 0) //if the two indexes are bigger or equal to zero - add
            res[index--] = a[a_size--] + b[b_size--];
        else if (a_size >= 0) //if only one index is bigger or equal to zero - add only this one
            res[index--] = a[a_size--];
        else if (b_size >= 0)
            res[index--] = b[b_size--];
        else
            break;
    }

    //check if the result is zero
    int zero = zeros(max, res);
    //if the result is not zero - print it
    if (zero != 0) {
        print(max, res);
    }
        //if the result is zero - print only '0'
    else {
        printf("%d\n", 0);
    }
    free(res);
    return 0;
}

//the operation - sub - function
int SUB(const int * a, int a_size,
        const int * b, int b_size) {
    int max; //max will be the degree of the result polynomial
    if (a_size > b_size)
        max = a_size;
    else
        max = b_size;
    int * res = (int * ) malloc(max * sizeof(int)); //the result array

    int index = max - 1;
    a_size--;
    b_size--;
    while (1) {
        if (a_size >= 0 && b_size >= 0) //if the two indexes are bigger or equal to zero - sub
            res[index--] = a[a_size--] - b[b_size--];
            //if only one index is bigger or equal to zero
        else if (a_size >= 0) // add only this one
            res[index--] = a[a_size--];
        else if (b_size >= 0) // sub only this one
            res[index--] = -b[b_size--];
        else
            break;
    }
    //check if the result is zero
    int zero = zeros(max, res);
    //if the result is not zero - print it
    if (zero != 0) {
        print(max, res);
    }
        //if the result is zero - print only '0'
    else {
        printf("%d\n", 0);
    }
    free(res);
    return 0;
}

//the operation - mul - function
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
    int zero = zeros(len, res);
    //if the result is not zero - print it
    if (zero != 0) {
        print(len, res);
    }
        //if the result is zero - print only '0'
    else {
        printf("%d\n", 0);
    }
    free(res);
    return 0;
}

//a callback function
int calc(const int * a, int a_size,
         const int * b, int b_size, int( * callback)(const int * , int,
                                                     const int * , int)) {
    return callback(a, a_size, b, b_size);
}

//function that print the result
void print(int max, int * res) {
    int not_only = 0; //equal to 0 - if there is not only one operand in the result
    int deg = max - 1;
    for (int k = 0; k < max; ++k) {
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

//function that return the array of the polynomial
int * parseInput(const char * input, int deg, int * input_index) {
    int * to_return = (int * ) malloc((deg + 1) * sizeof(int)); //array of polynomial to return
    int minus = 0;

    for (int i = 0; i <= deg; ++i) {
        int j = 0;
        char temp_input[INPUT_SIZE];

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

//function that return 1 - if the result is not zero , and 0 - else
int zeros(int max,
          const int * res) {
    for (int i = 0; i < max; ++i) {
        if (res[i] != 0) {
            return 1;
        }
    }
    return 0;
}