#include "frequency.h"

#include "text_utils.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_WORD_CAPACITY 8U

static int word_matches_normalized(const char *word, const char *start,
                                   size_t length)
{
    size_t index;

    if (strlen(word) != length) {
        return 0;
    }

    for (index = 0U; index < length; index++) {
        if ((unsigned char)word[index] !=
            text_to_lowercase((unsigned char)start[index])) {
            return 0;
        }
    }

    return 1;
}

static Status add_word(WordFrequencyTable *table, const char *start,
                       size_t length)
{
    size_t index;
    size_t new_capacity;
    WordFrequency *new_items;
    char *word_copy;

    for (index = 0U; index < table->size; index++) {
        if (word_matches_normalized(table->items[index].word, start, length)) {
            table->items[index].count++;
            return STATUS_SUCCESS;
        }
    }

    if (table->size == table->capacity) {
        new_capacity = table->capacity == 0U ? INITIAL_WORD_CAPACITY
                                              : table->capacity;
        if (new_capacity > SIZE_MAX / 2U) {
            return STATUS_ERROR_MEMORY;
        }
        new_capacity *= 2U;
        if (new_capacity > SIZE_MAX / sizeof(*new_items)) {
            return STATUS_ERROR_MEMORY;
        }

        new_items = realloc(table->items, new_capacity * sizeof(*new_items));
        if (new_items == NULL) {
            return STATUS_ERROR_MEMORY;
        }
        table->items = new_items;
        table->capacity = new_capacity;
    }

    word_copy = malloc(length + 1U);
    if (word_copy == NULL) {
        return STATUS_ERROR_MEMORY;
    }

    for (index = 0U; index < length; index++) {
        word_copy[index] = (char)text_to_lowercase((unsigned char)start[index]);
    }
    word_copy[length] = '\0';

    table->items[table->size].word = word_copy;
    table->items[table->size].count = 1U;
    table->size++;
    return STATUS_SUCCESS;
}

void word_frequency_table_init(WordFrequencyTable *table)
{
    if (table != NULL) {
        table->items = NULL;
        table->size = 0U;
        table->capacity = 0U;
    }
}

void free_word_frequency_table(WordFrequencyTable *table)
{
    size_t index;

    if (table == NULL) {
        return;
    }

    for (index = 0U; index < table->size; index++) {
        free(table->items[index].word);
    }
    free(table->items);
    word_frequency_table_init(table);
}

Status build_word_frequency(const char *text, WordFrequencyTable *table)
{
    const char *word_start = NULL;
    size_t word_length = 0U;
    size_t index;
    unsigned char character;
    Status status;

    if (text == NULL || table == NULL) {
        return STATUS_ERROR_INVALID;
    }

    word_frequency_table_init(table);
    for (index = 0U; text[index] != '\0'; index++) {
        character = (unsigned char)text[index];
        if (text_is_word_character(character)) {
            if (word_start == NULL) {
                word_start = text + index;
                word_length = 0U;
            }
            word_length++;
        } else if (word_start != NULL) {
            status = add_word(table, word_start, word_length);
            if (status != STATUS_SUCCESS) {
                free_word_frequency_table(table);
                return status;
            }
            word_start = NULL;
        }
    }

    if (word_start != NULL) {
        status = add_word(table, word_start, word_length);
        if (status != STATUS_SUCCESS) {
            free_word_frequency_table(table);
            return status;
        }
    }

    return STATUS_SUCCESS;
}

void calculate_character_frequency(
    const char *text, size_t frequency[CHARACTER_FREQUENCY_SIZE])
{
    size_t index;
    unsigned char character;

    if (text == NULL || frequency == NULL) {
        return;
    }

    for (index = 0U; index < CHARACTER_FREQUENCY_SIZE; index++) {
        frequency[index] = 0U;
    }

    for (index = 0U; text[index] != '\0'; index++) {
        character = (unsigned char)text[index];
        if (!isspace(character)) {
            if (text_is_word_character(character)) {
                character = text_to_lowercase(character);
            }
            frequency[character]++;
        }
    }
}

int get_most_frequent_character(
    const size_t frequency[CHARACTER_FREQUENCY_SIZE])
{
    size_t index;
    size_t highest = 0U;
    int result = -1;

    if (frequency == NULL) {
        return -1;
    }

    for (index = 0U; index < CHARACTER_FREQUENCY_SIZE; index++) {
        if (frequency[index] > highest) {
            highest = frequency[index];
            result = (int)index;
        }
    }

    return result;
}

size_t get_most_frequent_word_index(const WordFrequencyTable *table)
{
    size_t index;
    size_t result;

    if (table == NULL || table->size == 0U) {
        return SIZE_MAX;
    }

    result = 0U;
    for (index = 1U; index < table->size; index++) {
        if (table->items[index].count > table->items[result].count) {
            result = index;
        }
    }

    return result;
}

size_t get_top_word_indices(const WordFrequencyTable *table, size_t *indices,
                            size_t maximum)
{
    size_t rank;
    size_t candidate;
    size_t selected_count;
    size_t best;
    size_t prior;
    int already_selected;

    if (table == NULL || indices == NULL || maximum == 0U) {
        return 0U;
    }

    selected_count = table->size < maximum ? table->size : maximum;
    for (rank = 0U; rank < selected_count; rank++) {
        best = SIZE_MAX;
        for (candidate = 0U; candidate < table->size; candidate++) {
            already_selected = 0;
            for (prior = 0U; prior < rank; prior++) {
                if (indices[prior] == candidate) {
                    already_selected = 1;
                    break;
                }
            }
            if (!already_selected &&
                (best == SIZE_MAX ||
                 table->items[candidate].count > table->items[best].count)) {
                best = candidate;
            }
        }
        indices[rank] = best;
    }

    return selected_count;
}
