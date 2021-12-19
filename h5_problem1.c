#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>

int thread_count;

void Count_sort(int a[], int n);

void Check(int a[], int n);

int main(int argc, char *argv[]) {

    srand(time(NULL));

    thread_count = strtol(argv[1], NULL, 10);

    /* User inputs the length of array */
    int len;  
    printf("Input size:");
    scanf("%d", &len);

    int range = 10 * len;
    int *arr = malloc(len * sizeof(int));

    /* Randomly generate each elements */
    int i;
    for(i = 0; i < len; i++) {
        arr[i] = rand() % range;
    }

    struct timeval start, end, diff; 
    gettimeofday(&start, NULL);

    Count_sort(arr, len);
    //printf("Result: ");
    /*
    for (i = 0; i < len; i++) {
        printf("%d ", arr[i]);
    }
    puts("");
    */
    /* Caculate the total execution time */
    gettimeofday(&end, NULL);
    timersub(&end, &start, &diff);
    double time_used = diff.tv_sec + (double) diff.tv_usec / 1000000.0;

    Check(arr, len);

    printf("Execution time: %f secs.\n", time_used);
    return 0;
}

void Count_sort(int a[], int n) {

    int num, rank;
    int i, j, count;
    int* temp = malloc(n * sizeof(int));

    int start, size;
    #pragma omp parallel num_threads(thread_count) \
        default(none) shared(a, temp, n) private(rank, count, i, j, num, start, size)
    {
        num = omp_get_num_threads();
        rank = omp_get_thread_num();
        #pragma omp for schedule(static, 1)
        for (i = 0; i < n; i++) {
            count = 0;
            for (j = 0; j < n; j++) 
                if (a[j] < a[i]) count++; 
            else if (a[j] == a[i] && j < i)
                count++;
            temp[count] = a[i];
        }
                
        start = (rank < n % num) ? rank * ((n / num) + 1) : (rank * (n / num)) + n % num;
        size = (n / num) + (rank < n % num);
        
        #pragma omp barrier
        memcpy(&a[start], &temp[start], size * sizeof(int));
    }
    free(temp);
}

void Check(int a[], int n) {
    /* FUnction to check the result */
    int i;
    #pragma omp parallel for schedule(static, 1)
    for (i = 1; i < n; i++) {
        if (a[i] < a[i - 1]) 
            printf("Found error at index %d!\n", i);
    }

}
