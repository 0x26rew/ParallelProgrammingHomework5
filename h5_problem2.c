#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <sys/time.h>

/* The maximum number of characters in a single line */
#define BUFF_SIZE 1000

/* The maximum number of text lines of input file */
#define MAX_LINE 10000

/* Where the read data is stored */
char **text_queue;

/* The front and rear of text_queue */
int q_front, q_rear;

/* The number of line of input file,
   set to 0 initially until producer read up the file */
int end_line;

/* insert_done[i] is set to 1
   if the read data is inserted to text_queue[i], or 0 otherwise */
int *insert_done;

/* Keywords which will be searched */
char* keywords[] = {"China", "man", "virus"};

/* The number of apearance of each keywords */
int *counts;

void Produce(FILE *src, int rank);

void Consume(int rank, char **keys, int *counts);

/*****************************************************************************/

int main(int argc, char *argv[]) {

    /* Initialization */
    int thread_count = strtol(argv[1], NULL, 10);
    FILE *src;
    src = fopen("src.txt", "r");
    q_front = 0;
    q_rear = 0;
    end_line = 0;
    int num_of_keys = (sizeof(keywords) / sizeof(keywords[0]));
    counts = calloc(num_of_keys, sizeof(int));
    insert_done = calloc(MAX_LINE, sizeof(int));
    text_queue = malloc(MAX_LINE * sizeof(char*));

    struct timeval start, end, diff;
    gettimeofday(&start, NULL);

    /* Start the Producer-Consumer progress */
    #pragma omp parallel num_threads(thread_count)
    {
        int my_rank = omp_get_thread_num();
        /* Dispatch each thread to produce or consume */
        if  (my_rank < thread_count / 2) {
            Produce(src, my_rank);
        } else
            Consume(my_rank, keywords, counts);
    }

    /* Showing the result */
    int i;
    for (i = 0; i < num_of_keys; i++) {
        printf("%s: %d\n", keywords[i], counts[i]);
    }
    gettimeofday(&end, NULL);
    timersub(&end, &start, &diff);
    double time_used = diff.tv_sec + (double) diff.tv_usec / 1000000.0;
    printf("Execution time: %f secs.\n", time_used);
    return 0;
}

/*****************************************************************************/

static int CriticalRead(int *my_line, char *line, FILE *src) {
    /* There is only one thread can read the input file at a time */
    int flag;
    #pragma omp critical(read)
    {
        if (fgets(line, BUFF_SIZE, src)) {
            *my_line = q_front;
            q_front++;
            flag =  1;
        } else
            flag =  0;
    }
    return flag;
}

void Produce(FILE *src, int rank) {

    int my_line = 0;
    char *line = malloc(BUFF_SIZE * sizeof(char));

    while (CriticalRead(&my_line, line, src)) {
        /* Insert a new line to text_queue, 
           and set insert_done[my_line] to 1 */
        text_queue[my_line] = malloc(BUFF_SIZE * sizeof(char));
        strncpy(text_queue[my_line], line, BUFF_SIZE);
        insert_done[my_line] = 1;
    }
    if (rank == 0) {
        /* All line in input file is read */
        #pragma omp critical(end)
        {
            end_line = q_front;
        }
    }
    return;
}

/*****************************************************************************/
static int CriticalCheck(int *my_line, int rank) {
    int flag;
    #pragma omp critical(consume)
    {
        /* Check if their exist a line has not been consumed yet */
        if (q_rear < q_front || end_line == 0) {
            *my_line = q_rear;
            q_rear++;
            flag = 1;
        } else
            flag = 0;
    }
    return flag;
}

void Consume(int rank, char **keys, int *counts) {

    /* The line that this thread want to consume */
    int my_line;
    char *my_str = malloc(BUFF_SIZE * sizeof(char));

    while (CriticalCheck(&my_line, rank)) {

        /* Busy waiting until my_line is in text_queue
           or my_line is actually exceed the file */
        while (insert_done[my_line]  == 0 && (end_line == 0 || end_line > my_line));

        /* Do the following section if the file is still being read
           or some lines are not consumed */
        if (end_line == 0 || end_line > my_line) {

            strncpy(my_str, text_queue[my_line], BUFF_SIZE);
            int my_len = strlen(my_str);
            int i, j;
            int num_of_keys = sizeof(keywords) / sizeof(keywords[0]);
            int head = 0;
            int tail = 0;
            
            for (j = 0; j < my_len; j++) {
                /* A new word is found wheile encountering a space or a new line */
                if (my_str[j] == ' ' || my_str[j] == '\n') {
                    head = j;
                    for (i = 0; i < num_of_keys; i++) {
                        int key_len = strlen(keywords[i]);
                        /* The new word has length (head - tail) */
                        if (head - tail == key_len) {
                            if (strncmp(keywords[i], &my_str[tail], head - tail) == 0) {
                                #pragma omp critical(count)
                                counts[i]++;
                            }
                        }
                    }
                    /* Start to find next word */
                    if (head < my_len - 1)
                        tail = head + 1;
                }
            }
        }
    }
    return;
}