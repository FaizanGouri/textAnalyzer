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

static void test_statistics_edge_cases(void)
{
    TextStatistics statistics;
    char long_word[600];

    /* Only spaces */
    statistics = statistics_for("     ");
    expect_size("only spaces chars", statistics.characters, 5U);
    expect_size("only spaces words", statistics.words, 0U);
    expect_size("only spaces spaces", statistics.spaces, 5U);
    expect_double("only spaces avg word length", statistics.average_word_length, 0.0);
    expect_double("only spaces avg sent length", statistics.average_sentence_length, 0.0);
    expect_double("only spaces reading time", statistics.estimated_reading_time, 0.0);
    free_statistics(&statistics);

    /* Only tabs */
    statistics = statistics_for("\t\t\t");
    expect_size("only tabs count", statistics.tabs, 3U);
    expect_size("only tabs words", statistics.words, 0U);
    free_statistics(&statistics);

    /* Only newlines */
    statistics = statistics_for("\n\n\n");
    expect_size("only newlines lines", statistics.lines, 3U);
    expect_size("only newlines words", statistics.words, 0U);
    expect_size("only newlines paragraphs", statistics.paragraphs, 0U);
    free_statistics(&statistics);

    /* Only digits */
    statistics = statistics_for("12345 67890");
    expect_size("only digits count", statistics.digits, 10U);
    expect_size("only digits words", statistics.words, 0U);
    free_statistics(&statistics);

    /* Only punctuation */
    statistics = statistics_for("!?,.:;");
    expect_size("only punct special", statistics.special_chars, 6U);
    expect_size("only punct words", statistics.words, 0U);
    free_statistics(&statistics);

    /* Uppercase only */
    statistics = statistics_for("HELLO WORLD");
    expect_size("uppercase count", statistics.uppercase, 10U);
    expect_size("lowercase count", statistics.lowercase, 0U);
    expect_size("uppercase words", statistics.words, 2U);
    free_statistics(&statistics);

    /* Lowercase only */
    statistics = statistics_for("hello world");
    expect_size("lowercase count", statistics.lowercase, 10U);
    expect_size("uppercase count", statistics.uppercase, 0U);
    expect_size("lowercase words", statistics.words, 2U);
    free_statistics(&statistics);

    /* Sentences without spaces */
    statistics = statistics_for("Sentence one.Sentence two!Sentence three?");
    expect_size("sentences without spaces", statistics.sentences, 3U);
    expect_size("words without spaces", statistics.words, 6U);
    free_statistics(&statistics);

    /* Very long word */
    memset(long_word, 'a', sizeof(long_word) - 1U);
    long_word[sizeof(long_word) - 1U] = '\0';
    statistics = statistics_for(long_word);
    expect_size("long word words", statistics.words, 1U);
    expect_size("long word len", strlen(statistics.longest_word), 599U);
    expect_size("long word shortest len", strlen(statistics.shortest_word), 599U);
    expect_double("long word avg len", statistics.average_word_length, 599.0);
    free_statistics(&statistics);
}

int main(void)
{
    test_basic_statistics();
    test_sentence_and_character_counts();
    test_multiline_and_case();
    test_paragraphs_and_edge_cases();
    test_word_extremes();
    test_statistics_edge_cases();

    if (failures != 0) {
        printf("%d analyzer test(s) failed.\n", failures);
        return 1;
    }

    puts("All analyzer tests passed.");
    return 0;
}
