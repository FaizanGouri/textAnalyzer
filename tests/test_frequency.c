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

int main(void)
{
    test_word_frequency();
    test_character_frequency();
    test_empty_and_ordering();

    if (failures != 0) {
        printf("%d frequency test(s) failed.\n", failures);
        return 1;
    }

    puts("All frequency tests passed.");
    return 0;
}
