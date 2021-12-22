#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SET_SIZE 50
#define WORD_LEN  50

int main (int argc, char *argv[]) {
    srand(time(NULL));
    FILE *set = fopen("set.txt", "r");
    char **words = malloc(MAX_SET_SIZE * sizeof(char*));
    int num_of_words = 0;
    char *temp = malloc(WORD_LEN * sizeof(char));
    while(fgets(temp, WORD_LEN, set)) {
        printf("!!!%s\n", temp);
        words[num_of_words] = malloc(WORD_LEN * sizeof(char));
        strncpy(words[num_of_words], temp, strlen(temp) - 1);
        num_of_words++;
    }
    fclose(set);

    char *path = malloc(50 * sizeof(char));

    int i;
    printf("%s/%d\n", argv[1], atoi(argv[1]));
    for (i = 0; i < atoi(argv[1]); i++) {
        sprintf(path, "%s%d.txt", "keys/", i);
        FILE *out = fopen(path, "w");
        int i;
        int *counts = calloc(3, sizeof(int));
        for (i = 0; i < strtol(argv[2], NULL, 10); i++) {
            int j = 0;
            while (j < strtol(argv[3], NULL, 10) - 6) {
                int k = rand() % num_of_words;
                fprintf(out, "%s ", words[k]);
                j += strlen(words[k]);
            }
            fprintf(out, "\n");
        }
        fclose(out);
    }

}