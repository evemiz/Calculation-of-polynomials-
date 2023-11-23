# Calculation-of-polynomials-
Calculation of polynomials 
Authored by Eve Mizrahi
212010359


==Description==
This program performs operations on two polynomials. It allows the user to enter polynomial expressions and perform addition, subtraction, and multiplication operations on them. The program runs until the user enters "End" to exit the program.
There are tree ways to get the resault: 
	1. Ex3q1 - the reading, calculating and writing are done by the same process
	2. Ex3q2 - made by two seperate processes: Ex3q2a is the reader/producer - he is get the input of the polynomials and put the string in a shared 	memory, the second process - Ex3q2b is the reader/coustumer - he read the polynomials from the shared memory, claculate and print the resault.
	3. Ex3q3 - same as Ex3q2, but the calculating has done by threads - each thread calculate the sum/sub of each degree.

Program DATABASE:
- Ex3q2b - Data struct - contains two intagers - deg1 and deg2(the degrees 		of the polyanomials), two intagers arrays - first and second 		(contains the coefficients of the polyanomials).
- Ex3q3b - Data struct (like the one of Ex3q2b)
	   - Poly struct - contains the dgree of the current sum/sub, the 		firstNum and the secondNum - that should sub/sum between them 
		and the op - the opperation that should done.

functions:

- Ex3q1:
1. ADD - performs addition of two polynomials. It takes two polynomial arrays along with their sizes as input. It calculates the sum of the two polynomials.
2. SUB - performs subtraction of two polynomials. It takes two polynomial arrays along with their sizes as input. It calculates the difference between the two polynomials.
3. MUL - performs multiplication of two polynomials. It takes two polynomial arrays along with their sizes as input. It calculates the product of the two polynomials.
4. calc - a callback function that accepts two polynomial arrays along with their sizes and a function pointer (callback). It calls the specified callback function with the given arguments.
5. print - prints the resulting polynomial.
6. parseInput - parses the user input to extract the coefficients of a polynomial. It takes the input string, the degree of the polynomial, and a pointer to the current index in the input string. It extracts the coefficients from the input string and stores them in the to_return array, which is then returned.
7. zeros - This function checks if the resulting polynomial is zero.

- Ex3q2b:
uses the functions : ADD, SUB, MUL, calc, print, parseInput and zeros (description above)

- Ex3q3b:
uses the functions : MUL and parseInput (description above), and the next function :
-calc_threads -  a thread function that calculates the result of a specific degree in a polynomial based on the operation (addition or subtraction).


==Program Files==
- Ex3q1.c - contains the first question
- Ex3q2a.c - contains the second question's producer
- Ex3q2b.c - contains the second question's coustomer
- Ex3q3a.c - contains the third question's producer
- Ex3q3b.c - contains the third question's coustomer
- Makefile



==How to compile?==
compile: make
run question 1 : ./Ex3q1
run question 2-a : ./Ex3q2a
run question 2-b : ./Ex3q2b
run question 3-a : ./Ex3q3a
run question 3-b : ./Ex3q3b


==Input:==
two polynomials with an opperation between them, for example: 
(2:3,2,1)SUB(3:2,4,0,-1) - you can use also MUL or ADD opperations

==Output:==
the results of the operations, for example, the input example should print:
-2x^3 - 1x^2 + 2x +2


