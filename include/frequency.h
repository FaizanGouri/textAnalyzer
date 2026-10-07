#ifndef TEXT_ANALYZER_FREQUENCY_H
#define TEXT_ANALYZER_FREQUENCY_H

#include <stddef.h>

#include "common.h"

#define CHARACTER_FREQUENCY_SIZE 256U

typedef struct {
    char *word;
    size_t count;
} WordFrequency;

typedef struct {
    WordFrequency *items;
    size_t size;
    size_t capacity;
} WordFrequencyTable;

void word_frequency_table_init(WordFrequencyTable *table);
void free_word_frequency_table(WordFrequencyTable *table);
Status build_word_frequency(const char *text, WordFrequencyTable *table);
void calculate_character_frequency(
    const char *text, size_t frequency[CHARACTER_FREQUENCY_SIZE]);
int get_most_frequent_character(
    const size_t frequency[CHARACTER_FREQUENCY_SIZE]);
size_t get_most_frequent_word_index(const WordFrequencyTable *table);
size_t get_top_word_indices(const WordFrequencyTable *table, size_t *indices,
                            size_t maximum);

#endif
