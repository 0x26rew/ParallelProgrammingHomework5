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

   * The time complexity of **count sort** is $\Theta(n^2)$ , for the **input size grows in decades** the execution duration grows **hundreds of time**.
   * As for **quick sort** of time complexity $\Theta(nlog(n))$, its execution time does not dramatically grows like **count sort**.
   * Parallel count sort does not 12 times faster then serial count sort due to the overhead.

## Producer-Consumer

### Descriptions

#### Guide

1. Compile `h5_problem2.c`: `gcc -g -Wall -fopenmp -o p2 h5_problem2.c`
2. Execute program(`n` is the number of thread): `./p2 n`

The program **automatically read `n/2` source files** from  `home/E24076239/keys`  **without command line argument** or user input, and there will be `n/2` producers and `n-(n/2)` consumers.

#### Organization

```html
├── home/E24076239
│   ├── h5_problem2.c
│   ├── gen.c
│   ├── set.txt
│   ├── keywords.txt
│   ├── keys
│         ├── 0.txt
│         ├── 1.txt
│         ├──  ...
│         └── 11.txt
```

#### Generate Files

| File           | Usage                                                        |
| -------------- | ------------------------------------------------------------ |
| `gen.c`        | Generate source files for `h5_problem2.c`, use the following command:  <br>`./[EXECUTABLE FILE NAME] [# OF OUTPUT FILES(12)] [# OF LINES IN A FILE(100)] [MAX # OF CHAR IN A LINE(100)]` |
| `set.txt`      | The set of words for `gen.c` to randomly generate in source files, currently are `NTU`, `NCKU`, `NYCU` and `NCHU` |
| `keywords.txt` | The keywords that consumers are going to find, currently contains `NTU` and `NCKU` |

### Concepts

#### Data

| Variable              | Usage                                                        |
| --------------------- | ------------------------------------------------------------ |
| `char **text_queue`   | Producers insert the `target`'th line in `src.txt` to `text_queue[target].` |
| `int q_front, q_rear` | Global variable which are the front and rear of `text_queue` respectively. |
| `int *insert_done`    | Initially 0. Producers set `insert_done[target] = 1` after the `target`'th line is inserted to `text_queue[target].` |
| `int *counts`         | `counts[i]` is the number of appearance of `keywords[i]`.    |
| `int *private_counts` | The number of appearance of `keywords[i]` in a single consumer thread. |

#### What Producers do

A producer of  reads text lines from its own source file and then inserts the read text to `text_queue`, once the previous operations finish, the producer set `insert_done[target]` to `1`. A producer gets the value of its `target` from `q_front`.

#### What Consumers do

If `insert_done[target]` is `1`, a consumer will search `keywords` from `text_queue[target]`. A consumer gets the value of its `target` from `q_rear`.

### Analysis

| Number of Threads | Execution Time(secs.) |
| ----------------- | --------------------- |
| **2**             | 0.000455              |
| **4**             | 0.000468              |
| **6**             | 0.000846              |
| **8**             | 0.001059              |
| **10**            | 0.001542              |
| **12**            | 0.001685              |

Since there exist **critical sections** to guarantee that only one thread can accumulate the value of `q_front` or `q_rear`, the execution time increases when the number of threads increasing.


## Difficulties

I want to reduce an array(`int *counts`) in **Producer-Consumer**, it can be compiled on my laptop which has `gcc 11.1.0`. But on the server of `gcc 4.5.1` the compiler gives the message `‘counts’ has invalid type for ‘reduction’`. So I used another private array `int *private_counts` in each thread and sum up them to `int *counts` to get the final result. 
