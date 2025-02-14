#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#define MAX_WORD_LENGTH 100
#define MAX_WORDS 10000
#define MAX_THREADS 4

typedef struct {
    char word[MAX_WORD_LENGTH];
    int count;
} WordCount;

typedef struct {
    FILE *file;
    long start_pos;
    long end_pos;
    WordCount word_counts[MAX_WORDS];
    int word_count_size;
} ThreadData;

long find_word_boundary(FILE *file, long pos) {
    fseek(file, pos, SEEK_SET);
    int c;
    while ((c = fgetc(file)) != EOF && !isspace(c)) {
    }
    return ftell(file);
}

void *count_words_in_segment(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    FILE *file = data->file;
    fseek(file, data->start_pos, SEEK_SET);

    char word[MAX_WORD_LENGTH];
    while (fscanf(file, "%99s", word) != EOF) {
        
        if (ftell(file) > data->end_pos) {
            break;
        }

        for (int i = 0; word[i]; i++) {
            word[i] = tolower(word[i]);
        }

        if (strlen(word) == 0) {
            continue;
        }

        int found = 0;
        for (int i = 0; i < data->word_count_size; i++) {
            if (strcmp(data->word_counts[i].word, word) == 0) {
                data->word_counts[i].count++;
                found = 1;
                break;
            }
        }

        if (!found && data->word_count_size < MAX_WORDS) {
            strcpy(data->word_counts[data->word_count_size].word, word);
            data->word_counts[data->word_count_size].count = 1;
            data->word_count_size++;
        }
    }
    pthread_exit(NULL);
}

void consolidate_word_counts(WordCount *global_word_counts, int *global_word_count_size, WordCount *local_word_counts, int local_word_count_size) {
    for (int i = 0; i < local_word_count_size; i++) {
        int found = 0;
        for (int j = 0; j < *global_word_count_size; j++) {
            if (strcmp(global_word_counts[j].word, local_word_counts[i].word) == 0) {
                global_word_counts[j].count += local_word_counts[i].count;
                found = 1;
                break;
            }
        }
        if (!found && *global_word_count_size < MAX_WORDS) {
            strcpy(global_word_counts[*global_word_count_size].word, local_word_counts[i].word);
            global_word_counts[*global_word_count_size].count = local_word_counts[i].count;
            (*global_word_count_size)++;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    int num_threads = atoi(argv[2]);

    if (num_threads <= 0 || num_threads > MAX_THREADS) {
        fprintf(stderr, "Invalid number of threads. Must be between 1 and %d.\n", MAX_THREADS);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    WordCount global_word_counts[MAX_WORDS];
    int global_word_count_size = 0;

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    long segment_size = file_size / num_threads;
    long remaining = file_size % num_threads;

    long current_pos = 0;
    for (int i = 0; i < num_threads; i++) {
        FILE *thread_file = fopen(filename, "r");
        if (!thread_file) {
            perror("Error opening file for thread");
            fclose(file);
            return EXIT_FAILURE;
        }

        long start_pos = current_pos;
        long end_pos = (i + 1) * segment_size + (i + 1 < remaining ? i + 1 : remaining);

       
        end_pos = find_word_boundary(thread_file, end_pos);

        
        if (end_pos < start_pos) {
            end_pos = file_size;
        }

        thread_data[i].file = thread_file;
        thread_data[i].start_pos = start_pos;
        thread_data[i].end_pos = end_pos;
        thread_data[i].word_count_size = 0;

        if (pthread_create(&threads[i], NULL, count_words_in_segment, &thread_data[i]) != 0) {
            perror("Error creating thread");
            fclose(thread_file);
            fclose(file);
            return EXIT_FAILURE;
        }

        current_pos = end_pos;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        consolidate_word_counts(global_word_counts, &global_word_count_size, thread_data[i].word_counts, thread_data[i].word_count_size);
        fclose(thread_data[i].file);
    }

    printf("Word Frequencies:\n");
    for (int i = 0; i < global_word_count_size; i++) {
        printf("%s: %d\n", global_word_counts[i].word, global_word_counts[i].count);
    }

    fclose(file);
    return EXIT_SUCCESS;
}
