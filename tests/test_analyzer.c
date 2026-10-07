#include <stdio.h>
#include <string.h>

#include "analyzer.h"

static int failures = 0;

static void expect_size(const char *name, size_t actual, size_t expected)
{
    if (actual != expected) {
        printf("FAIL: %s (expected %zu, got %zu)\n", name, expected, actual);
        failures++;
    }
}

static void expect_double(const char *name, double actual, double expected)
{
    if (actual != expected) {
        printf("FAIL: %s (expected %.2f, got %.2f)\n", name, expected, actual);
        failures++;
    }
}

static void expect_word(const char *name, const char *actual, const char *expected)
{
    if (actual == NULL || strcmp(actual, expected) != 0) {
        printf("FAIL: %s\n", name);
        failures++;
    }
}

static TextStatistics statistics_for(const char *text)
{
    TextStatistics statistics;
    Status status;

    text_statistics_init(&statistics);
    status = analyze_text(text, &statistics);
    if (status != STATUS_SUCCESS) {
        printf("FAIL: analyze_text returned %d\n", status);
        failures++;
    }
    return statistics;
}

static void test_basic_statistics(void)
{
    TextStatistics statistics = statistics_for("Hello world");

    expect_size("basic characters", statistics.characters, 11U);
    expect_size("basic words", statistics.words, 2U);
    expect_size("basic lines", statistics.lines, 1U);
    expect_size("basic sentences", statistics.sentences, 0U);
    expect_size("basic paragraphs", statistics.paragraphs, 1U);
    free_statistics(&statistics);
}

static void test_sentence_and_character_counts(void)
{
    TextStatistics statistics = statistics_for("Hello world!");

    expect_size("sentence words", statistics.words, 2U);
    expect_size("sentence count", statistics.sentences, 1U);
    expect_size("sentence paragraphs", statistics.paragraphs, 1U);
    free_statistics(&statistics);

    statistics = statistics_for("Hello 123!");

    expect_size("digits", statistics.digits, 3U);
    expect_size("words with digits", statistics.words, 1U);
    expect_size("digit sentence count", statistics.sentences, 1U);
    free_statistics(&statistics);
}

static void test_multiline_and_case(void)
{
    TextStatistics statistics = statistics_for("Hello world!\nThis is C.");

    expect_size("multiline words", statistics.words, 5U);
    expect_size("multiline lines", statistics.lines, 2U);
    expect_size("multiline sentences", statistics.sentences, 2U);
    expect_size("multiline paragraphs", statistics.paragraphs, 1U);
    free_statistics(&statistics);

    statistics = statistics_for("ABC abc");
    expect_size("uppercase", statistics.uppercase, 3U);
    expect_size("lowercase", statistics.lowercase, 3U);
    expect_size("vowels", statistics.vowels, 2U);
    expect_size("consonants", statistics.consonants, 4U);
    free_statistics(&statistics);
}

static void test_paragraphs_and_edge_cases(void)
{
    TextStatistics statistics = statistics_for("First paragraph.\n\nSecond paragraph.");

    expect_size("paragraphs", statistics.paragraphs, 2U);
    free_statistics(&statistics);

    statistics = statistics_for("");
    expect_size("empty characters", statistics.characters, 0U);
    expect_size("empty words", statistics.words, 0U);
    expect_double("empty average word length", statistics.average_word_length, 0.0);
    free_statistics(&statistics);

    statistics = statistics_for("Hello!!! ... ???");
    expect_size("punctuation words", statistics.words, 1U);
    expect_size("punctuation sentence runs", statistics.sentences, 3U);
    free_statistics(&statistics);

    statistics = statistics_for("Hello!!!");
    expect_size("repeated punctuation sentence", statistics.sentences, 1U);
    free_statistics(&statistics);
}

static void test_word_extremes(void)
{
    TextStatistics statistics = statistics_for("small extraordinary!");

    expect_word("longest word", statistics.longest_word, "extraordinary");
    expect_word("shortest word", statistics.shortest_word, "small");
    expect_double("average word length", statistics.average_word_length, 9.0);
    free_statistics(&statistics);
}

int main(void)
{
    test_basic_statistics();
    test_sentence_and_character_counts();
    test_multiline_and_case();
    test_paragraphs_and_edge_cases();
    test_word_extremes();

    if (failures != 0) {
        printf("%d analyzer test(s) failed.\n", failures);
        return 1;
    }

    puts("All analyzer tests passed.");
    return 0;
}
