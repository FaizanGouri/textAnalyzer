#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "search.h"

static int failures = 0;

static void expect_size(const char *name, size_t actual, size_t expected)
{
    if (actual != expected) {
        printf("FAIL: %s (expected %zu, got %zu)\n", name, expected, actual);
        failures++;
    }
}

static void expect_status(const char *name, Status actual, Status expected)
{
    if (actual != expected) {
        printf("FAIL: %s (expected %d, got %d)\n", name, expected, actual);
        failures++;
    }
}

static void expect_string(const char *name, const char *actual,
                          const char *expected)
{
    if (actual == NULL || strcmp(actual, expected) != 0) {
        printf("FAIL: %s\n", name);
        failures++;
    }
}

static void test_search_counts(void)
{
    expect_size("basic word search",
                count_occurrences("hello world", "hello", 1, 1), 1U);
    expect_size("missing search",
                count_occurrences("hello world", "xyz", 1, 1), 0U);
    expect_size("case-sensitive search",
                count_occurrences("Computer computer COMPUTER", "computer", 1,
                                  1),
                1U);
    expect_size("case-insensitive search",
                count_occurrences("Computer computer COMPUTER", "computer", 0,
                                  1),
                3U);
    expect_size("whole-word search",
                count_occurrences("cat catch bobcat", "cat", 1, 1), 1U);
    expect_size("substring search",
                count_occurrences("cat catch bobcat", "cat", 1, 0), 3U);
    expect_size("non-overlapping search", count_occurrences("aaaa", "aa", 1, 0),
                2U);
    expect_size("repeated word search",
                count_occurrences("one two one two one", "one", 1, 1), 3U);
    expect_size("phrase search",
                count_occurrences("machine learning is useful. Machine Learning is powerful.",
                                  "machine learning", 0, 0),
                2U);
}

static void test_replacement(void)
{
    const char *original = "I like C. C is powerful.";
    char *result = NULL;
    size_t replacements = 0U;
    Status status;

    status = replace_text(original, "C", "C language", 1, 1, &result,
                          &replacements);
    expect_status("replacement status", status, STATUS_SUCCESS);
    expect_size("replacement count", replacements, 2U);
    expect_string("replacement result",
                  result, "I like C language. C language is powerful.");
    free(result);

    result = NULL;
    replacements = 99U;
    status = replace_text(original, "missing", "new", 1, 1, &result,
                          &replacements);
    expect_status("missing replacement status", status, STATUS_SUCCESS);
    expect_size("missing replacement count", replacements, 0U);
    expect_string("original remains unchanged", original,
                  "I like C. C is powerful.");
    if (result != NULL) {
        puts("FAIL: missing replacement returned a result");
        failures++;
        free(result);
    }

    result = NULL;
    replacements = 0U;
    status = replace_text("cat catch", "cat", "X", 1, 0, &result,
                          &replacements);
    expect_status("substring replacement status", status, STATUS_SUCCESS);
    expect_size("substring replacement count", replacements, 2U);
    expect_string("substring replacement result", result, "X Xch");
    free(result);
}

static void test_invalid_input(void)
{
    char *result = NULL;
    size_t replacements = 0U;

    expect_size("empty pattern count",
                count_occurrences("hello", "", 1, 1), 0U);
    expect_status("empty replacement pattern",
                  replace_text("hello", "", "new", 1, 1, &result,
                               &replacements),
                  STATUS_ERROR_INPUT);
}

static void test_search_edge_cases(void)
{
    char *result = NULL;
    size_t replacements = 0U;
    Status status;

    /* 1. Pattern longer than text */
    expect_size("pattern longer than text",
                count_occurrences("short", "verylongpattern", 1, 0), 0U);

    /* 2. Pattern matches entire text */
    status = replace_text("exact", "exact", "replaced", 1, 1, &result,
                          &replacements);
    expect_status("match entire text status", status, STATUS_SUCCESS);
    expect_size("match entire text count", replacements, 1U);
    expect_string("match entire text result", result, "replaced");
    free(result);
    result = NULL;

    /* 3. Replacement with empty string (deletion) */
    status = replace_text("one, two, three", ", ", "", 1, 0, &result,
                          &replacements);
    expect_status("deletion status", status, STATUS_SUCCESS);
    expect_size("deletion count", replacements, 2U);
    expect_string("deletion result", result, "onetwothree");
    free(result);
    result = NULL;

    /* 4. Delete entire text */
    status = replace_text("clear", "clear", "", 1, 1, &result,
                          &replacements);
    expect_status("delete entire status", status, STATUS_SUCCESS);
    expect_size("delete entire count", replacements, 1U);
    expect_string("delete entire result", result, "");
    free(result);
    result = NULL;

    /* 5. Pattern at very beginning and end */
    status = replace_text("alpha beta alpha", "alpha", "omega", 1, 1, &result,
                          &replacements);
    expect_status("begin and end status", status, STATUS_SUCCESS);
    expect_size("begin and end count", replacements, 2U);
    expect_string("begin and end result", result, "omega beta omega");
    free(result);
    result = NULL;

    /* 6. Consecutive matches */
    status = replace_text("ababab", "ab", "z", 1, 0, &result,
                          &replacements);
    expect_status("consecutive status", status, STATUS_SUCCESS);
    expect_size("consecutive count", replacements, 3U);
    expect_string("consecutive result", result, "zzz");
    free(result);
    result = NULL;

    /* 7. Non-overlapping advance */
    status = replace_text("aaaa", "aa", "b", 1, 0, &result,
                          &replacements);
    expect_status("non-overlapping replacement status", status, STATUS_SUCCESS);
    expect_size("non-overlapping replacement count", replacements, 2U);
    expect_string("non-overlapping replacement result", result, "bb");
    free(result);
    result = NULL;

    /* 8. Replacement with special characters and symbols */
    status = replace_text("var = val", "=", "<===>", 1, 0, &result,
                          &replacements);
    expect_status("special chars replacement status", status, STATUS_SUCCESS);
    expect_size("special chars replacement count", replacements, 1U);
    expect_string("special chars replacement result", result, "var <===> val");
    free(result);
    result = NULL;

    /* 9. Case-insensitive replace */
    status = replace_text("Foo foo FOO", "foo", "bar", 0, 1, &result,
                          &replacements);
    expect_status("case-insensitive replacement status", status, STATUS_SUCCESS);
    expect_size("case-insensitive replacement count", replacements, 3U);
    expect_string("case-insensitive replacement result", result, "bar bar bar");
    free(result);
    result = NULL;

    /* 10. Large expansion */
    status = replace_text("1 2", " ", " --- SPACER --- ", 1, 0, &result,
                          &replacements);
    expect_status("expansion status", status, STATUS_SUCCESS);
    expect_size("expansion count", replacements, 1U);
    expect_string("expansion result", result, "1 --- SPACER --- 2");
    free(result);
    result = NULL;
}

int main(void)
{
    test_search_counts();
    test_replacement();
    test_invalid_input();
    test_search_edge_cases();

    if (failures != 0) {
        printf("%d search test(s) failed.\n", failures);
        return 1;
    }

    puts("All search tests passed.");
    return 0;
}
