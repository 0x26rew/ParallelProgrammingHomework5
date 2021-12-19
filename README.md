# Homework 5

## Count Sort

### Guide

The size of array is initialized after **user input**. The program shows`Input size:`in the beginning of execution, input the array size, and the program will randomly generate an array and start to sort.

### Questions

1. If we try to parallelize the for i loop (the outer loop), which variables should be private and which should be shared?
   * Private: `rank`, `count`, `i`,  `j`, `num`, `start` and  `size`
   * Shared: `a`,`temp` and `n`
   
2. If we parallelize the for i loop using the scoping you speciﬁed in the previous part, are there any loop-carried dependences? Explain your answer.
   * There should be no dependencies since all elements in `temp` are not dependent since each `count` of an element is calculated from `a`, and `a` does not change while looping.
   
3. Can we parallelize the call to `memcpy`? Can we modify the code so that this part of the function will be parallelizable?

   * We can assign each thread to copy a specified memory space from `temp` to `a`, the method is shown below:

     ```c
     start = (rank < n % num) ? rank * ((n / num) + 1) : (rank * (n / num)) + n % num;
     size = (n / num) + (rank < n % num);
     #pragma omp barrier
     memcpy(&a[start], &temp[start], size * sizeof(int));
     ```

4. Write a C program that includes a parallel implementation of Count sort.

5. How does the performance of your parallelization of Count sort compare to serial Count sort? How does it compare to the serial `qsort` library function?

   * The table below compares the execution times(**in second**) between the above three sorting method, the execution is on my laptop with a 6-core 12-thread CPU:

   |        | Serial Count Sort | Parallel Count Sort(12 threads) | Quick Sort |
   | ------ | ----------------- | ------------------------------- | ---------- |
   | $10^3$ | 0.004730          | 0.001897                        | 0.000212   |
   | $10^4$ | 0.406499          | 0.059188                        | 0.001300   |
   | $10^5$ | 41.354350         | 6.392387                        | 0.010904   |

   * The time complexity of **count sort** is $\Theta(n^2)$ , for the input size grows in decades the execution duration grows hundreds of time. As for **quick sort** of complexity $\Theta(nlog(n))$ the execution time do not dramatically grows like **count sort**.
   * Parallel count sort does not 12 times faster then serial count sort due to the overhead.

## Producer-Consumer

### Descriptions

#### Read File

1. Compile `h5_problem2.c`: `gcc -g -Wall -fopenmp -o p2 h5_problem2.c`
2. Execute program(n is the number of thread): `./p2 n`

`h5_problem2.c` **automatically read the input file `src.txt` **without command line argument or user input.  The input file`src.txt` is under my home directory.

#### Keywords

The keyword that `h5_problem2.c`specifies are `"China"`,`"man"` and `"virus"`.

I use another `c` program, `gen.c` , to randomly generate `src.txt`, which produce words from the set of strings,  `{"China", "man", "virus", "covid", "mask", "WHO"}`. Each line in `src.txt` has 40 words, and the number of line is `argv[1]` of  `gen.c`.

