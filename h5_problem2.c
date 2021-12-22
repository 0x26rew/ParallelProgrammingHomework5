#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <sys/time.h>

#define BUFF_SIZE 1000  /* The maximum number of characters in a single line */

#define MAX_LINE 10000  /* The maximum number of text lines of input file */

#define WORD_LEN 50     /* The maximum length of a single word */

int thread_count;       /* Number of threads */

char **srcs;            /* The path of source files */

char **text_queue;      /* Where the read data is stored */

int q_front, q_rear;    /* The front and rear of text_queue */

int end_file;           /* The number of line of input file,
                        set to 0 initially until producer read up the file */

int end_length;         /* The sum of number of lines of all source files */

int *insert_done;       /* insert_done[i] is set to 1
                        if the read data is inserted to text_queue[i],
                        or 0 otherwise */

char **keywords;        /* Keywords which will be searched */

int num_of_keys;        /* Number of keywords */

int *counts;            /* The number of occurences of each keywords */

void Produce(char *src, int rank);

void Consume(int rank, char **keys, int *counts);

/*****************************************************************************/

int main(int argc, char *argv[]) {

    
    thread_count = strtol(argv[1], NULL, 10);

    /* Read keywords.txt to get keywords */
    FILE *keyword_file = fopen("keywords.txt", "r");
    char *temp = malloc(WORD_LEN * sizeof(char));
    keywords = malloc(10 * sizeof(char*));
    num_of_keys = 0;
    while(fgets(temp, WORD_LEN, keyword_file)) {
        keywords[num_of_keys] = malloc(WORD_LEN * sizeof(char));
        strncpy(keywords[num_of_keys], temp, strlen(temp) - 1);
        num_of_keys++;
    }

    /* Open (threads_count / 2) files */
    srcs = malloc((thread_count / 2) * sizeof(char*));
    int i = 0;
    for (i = 0; i < thread_count / 2; i++) {
        srcs[i] = malloc(20 * sizeof(char));
        sprintf(srcs[i], "keys/%d.txt", i);
    }

    /* Initialization */
    q_front = 0;
    q_rear = 0;
    end_length = 0;
    end_file = 0;
    counts = calloc(num_of_keys, sizeof(int));
    insert_done = calloc(MAX_LINE, sizeof(int));
    text_queue = malloc(MAX_LINE * sizeof(char*));

    struct timeval start, end, diff;
    gettimeofday(&start, NULL);

    /* Start the Producer-Consumer progress */
    #pragma omp parallel num_threads(thread_count)
    {
        int my_rank = omp_get_thread_num();
        /* Since  */
        int *private_counts = calloc(num_of_keys, sizeof(int));
        /* Dispatch each thread to produce or consume */
        if  (my_rank < thread_count / 2) {
            Produce(srcs[my_rank], my_rank);
        } else
            Consume(my_rank, keywords, private_counts);
        int i;
        #pragma omp critical(sum)
        for (i = 0; i < num_of_keys; i++) {
            counts[i] += private_counts[i];
        }
    }

    /* Show the result */
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

int CriticalRead(int *target, char *line, FILE *my_file) {
    /* There is only one thread can read the input file at a time */
    int flag;

    if (fgets(line, BUFF_SIZE, my_file)) {
        #pragma omp critical(read)
        {
            *target = q_front;
            q_front++;
        }
        flag =  1;
    } else
        flag =  0;

    return flag;
}

void Produce(char *src, int rank) {

    FILE *my_file = fopen(src, "r");
    int target = 0;
    int my_length = 0;
    char *line = malloc(BUFF_SIZE * sizeof(char));
    
    while (CriticalRead(&target, line, my_file)) {
        /* Insert a new line to text_queue, 
           and set insert_done[target] to 1 */
        text_queue[target] = malloc(BUFF_SIZE * sizeof(char));
        strncpy(text_queue[target], line, BUFF_SIZE);
        my_length++;
        insert_done[target] = 1;
    }
    /* All line in input file is read */
    #pragma omp critical(end)
    {
        end_length += my_length;
        end_file++;
    }
    fclose(my_file);
    return;
}

/*****************************************************************************/
int CriticalCheck(int *target, int rank) {
    int flag;
    #pragma omp critical(consume)
    {
        /* Check if their exist a line has not been consumed yet */
        if (q_rear < q_front || end_file < thread_count / 2) {
            *target = q_rear;
            q_rear++;
            flag = 1;
        } else
            flag = 0;
    }
    return flag;
}

void Consume(int rank, char **keys, int *counts) {
    
    /* The line that this thread want to consume */
    int target;
    char *my_str = malloc(BUFF_SIZE * sizeof(char));

    while (CriticalCheck(&target, rank)) {        
        /* Busy waiting until target is in text_queue
           or target is actually exceed the file */
        while (insert_done[target]  == 0 && 
            (end_file < thread_count / 2 || end_length > target));
        /* Do the following section if the file is still being read
           or some lines are not consumed */
        if (end_file < thread_count / 2 || end_length > target) {

            strncpy(my_str, text_queue[target], BUFF_SIZE);
            int my_len = strlen(my_str);
            int i, j;
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
                                //#pragma omp critical(count)
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