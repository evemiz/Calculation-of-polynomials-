//Q1
#include <stdio.h>

#include <string.h>

#include <stdlib.h>

#define SIZE 128

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
    int deg1; //the degree of the first polynomial
    int * first; //an array of the first polynomial
    int deg2; //the degree of the second polynomial
    int * second; //the array of the second polynomial
    char input[SIZE]; //the input from user
    char temp_input[SIZE]; //temp string to help with the input string
    int add; //if equal 1 - this is an add operation
    int sub; //if equal 1 - this is a sub operation
    int mul; //if equal 1 - this is a mul operation

    while (1) {
        add = 0;
        sub = 0;
        mul = 0;

        //get the input
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';

        //in a case the user enter "END" - exit the program
        if (strcmp(input, "END") == 0)
            break;

        int input_index = 1; //after the first 'closer'

        //first degree
        int i = 0;
        while (input[input_index] != ':') {
            temp_input[i++] = input[input_index++];
        }
        temp_input[i] = '\0';
        char * end;
        deg1 = (int) strtol(temp_input, & end, 10);

        if ( * end != '\0')
            exit(1);

        //the first polynomial numbers
        input_index++; //the index after the ':'
        first = parseInput(input, deg1, & input_index);

        //the operation
        i = 0;
        while (input[input_index] != '(') {
            temp_input[i++] = input[input_index++];
        }
        temp_input[i] = '\0';
        input_index++;

        if (strcmp(temp_input, "ADD") == 0)
            add = 1;
        if (strcmp(temp_input, "SUB") == 0)
            sub = 1;
        if (strcmp(temp_input, "MUL") == 0)
            mul = 1;

        //second degree
        i = 0;
        while (input[input_index] != ':') {
            temp_input[i++] = input[input_index++];
        }
        temp_input[i] = '\0';
        deg2 = (int) strtol(temp_input, & end, 10);

        if ( * end != '\0')
            exit(1);

        //the second polynomial numbers
        input_index++; //the index after the ':'
        second = parseInput(input, deg2, & input_index);

        //goto the function of the operation
        if (add)
            calc(first, deg1 + 1, second, deg2 + 1, ADD);
        else if (sub)
            calc(first, deg1 + 1, second, deg2 + 1, SUB);
        else if (mul)
            calc(first, deg1 + 1, second, deg2 + 1, MUL);

        free(first);
        free(second);
    }

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
        char temp_input[SIZE];

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