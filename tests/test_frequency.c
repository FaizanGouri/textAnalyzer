#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "frequency.h"

static int failures = 0;

static void expect_size(const char *name, size_t actual, size_t expected)
{
    if (actual != expected) {
        printf("FAIL: %s (expected %zu, got %zu)\n", name, expected, actual);
        failures++;
    }
}

static void expect_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        printf("FAIL: %s (expected %d, got %d)\n", name, expected, actual);
        failures++;
    }
}

static size_t count_for(const WordFrequencyTable *table, const char *word)
{
    size_t index;

    for (index = 0U; index < table->size; index++) {
        if (strcmp(table->items[index].word, word) == 0) {
            return table->items[index].count;
        }
    }
    return 0U;
}

static WordFrequencyTable table_for(const char *text)
{
    WordFrequencyTable table;
    Status status;

    word_frequency_table_init(&table);
    status = build_word_frequency(text, &table);
    if (status != STATUS_SUCCESS) {
        printf("FAIL: build_word_frequency returned %d\n", status);
        failures++;
    }
    return table;
}

static void test_word_frequency(void)
{
    WordFrequencyTable table = table_for("apple apple banana");

    expect_size("apple frequency", count_for(&table, "apple"), 2U);
    expect_size("banana frequency", count_for(&table, "banana"), 1U);
    free_word_frequency_table(&table);

    table = table_for("Apple APPLE apple");
    expect_size("case-normalized apple", count_for(&table, "apple"), 3U);
    free_word_frequency_table(&table);

    table = table_for("Hello, hello! HELLO.");
    expect_size("punctuation-separated hello", count_for(&table, "hello"), 3U);
    free_word_frequency_table(&table);
}

static void test_character_frequency(void)
{
    size_t frequency[CHARACTER_FREQUENCY_SIZE];

    calculate_character_frequency("aaabb!", frequency);
    expect_size("character a", frequency[(unsigned char)'a'], 3U);
    expect_size("character b", frequency[(unsigned char)'b'], 2U);
    expect_size("character exclamation", frequency[(unsigned char)'!'], 1U);
    expect_int("most frequent character", get_most_frequent_character(frequency),
               (int)'a');

    calculate_character_frequency("AaA Bb", frequency);
    expect_size("normalized character a", frequency[(unsigned char)'a'], 3U);
    expect_size("normalized character b", frequency[(unsigned char)'b'], 2U);
}

static void test_empty_and_ordering(void)
{
    WordFrequencyTable table = table_for("");
    size_t indices[10];

    expect_size("empty table", table.size, 0U);
    expect_size("empty top words", get_top_word_indices(&table, indices, 10U), 0U);
    free_word_frequency_table(&table);

    table = table_for("one two three four five six seven eight nine ten eleven");
    expect_size("top ten count", get_top_word_indices(&table, indices, 10U), 10U);
    expect_size("all unique words", table.size, 11U);
    free_word_frequency_table(&table);

    table = table_for("beta alpha beta alpha gamma");
    expect_size("few top words", get_top_word_indices(&table, indices, 10U), 3U);
    expect_size("deterministic tied word", get_most_frequent_word_index(&table), 0U);
    expect_size("first top word", indices[0], 0U);
    free_word_frequency_table(&table);
}

static void test_frequency_edge_cases(void)
{
    size_t frequency[CHARACTER_FREQUENCY_SIZE];
    WordFrequencyTable table;
    char buffer[2048];
    size_t i;

    /* 1. Empty character frequency */
    calculate_character_frequency("", frequency);
    expect_int("empty character frequency most frequent",
               get_most_frequent_character(frequency), -1);

    /* 2. Only whitespace character frequency */
    calculate_character_frequency("   \t\n  \r\n ", frequency);
    expect_int("whitespace only character frequency",
               get_most_frequent_character(frequency), -1);

    /* 3. Empty word table most frequent */
    word_frequency_table_init(&table);
    expect_size("empty table most frequent index",
                get_most_frequent_word_index(&table), SIZE_MAX);
    free_word_frequency_table(&table);

    /* 4. Digits only: no words */
    table = table_for("12345 67890 999");
    expect_size("digits only word count", table.size, 0U);
    free_word_frequency_table(&table);

    /* 5. Single word repeated 50 times */
    buffer[0] = '\0';
    for (i = 0; i < 50; i++) {
        strcat(buffer, "repeat ");
    }
    table = table_for(buffer);
    expect_size("single repeated word table size", table.size, 1U);
    expect_size("single repeated word count", count_for(&table, "repeat"), 50U);
    free_word_frequency_table(&table);

    /* 6. Capacity growth: 60 unique words (initial capacity is 16) */
    buffer[0] = '\0';
    for (i = 0; i < 60; i++) {
        char word[16];
        sprintf(word, "%c%c ", 'a' + (int)(i / 26), 'a' + (int)(i % 26));
        strcat(buffer, word);
    }
    table = table_for(buffer);
    expect_size("60 unique words size", table.size, 60U);
    for (i = 0; i < 60; i++) {
        char word[16];
        sprintf(word, "%c%c", 'a' + (int)(i / 26), 'a' + (int)(i % 26));
        if (count_for(&table, word) != 1U) {
            printf("FAIL: missing or wrong count for %s\n", word);
            failures++;
            break;
        }
    }
    free_word_frequency_table(&table);

    /* 7. Words at boundaries with various symbols */
    table = table_for("[start] middle. (end)");
    expect_size("boundary words count", table.size, 3U);
    expect_size("start word count", count_for(&table, "start"), 1U);
    expect_size("middle word count", count_for(&table, "middle"), 1U);
    expect_size("end word count", count_for(&table, "end"), 1U);
    free_word_frequency_table(&table);
}

int main(void)
{
    test_word_frequency();
    test_character_frequency();
    test_empty_and_ordering();
    test_frequency_edge_cases();

    if (failures != 0) {
        printf("%d frequency test(s) failed.\n", failures);
        return 1;
    }

    puts("All frequency tests passed.");
    return 0;
}
