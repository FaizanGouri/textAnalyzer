#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "report.h"
#include "text_utils.h"

#define NORMAL_REPORT "build/test_report_normal.txt"
#define MULTILINE_REPORT "build/test_report_multiline.txt"
#define EMPTY_REPORT "build/test_report_empty.txt"
#define TOP_WORDS_REPORT "build/test_report_top_words.txt"
#define INVALID_REPORT_PATH "nonexistent_directory_xyz123/report.txt"

static int failures = 0;

static void expect_status(const char *name, Status actual, Status expected)
{
    if (actual != expected) {
        printf("FAIL: %s (expected %d, got %d)\n", name, expected, actual);
        failures++;
    }
}

static void expect_true(const char *name, int condition)
{
    if (!condition) {
        printf("FAIL: %s\n", name);
        failures++;
    }
}

static char *read_entire_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    long file_size;
    char *buffer;
    size_t read_bytes;

    if (file == NULL) {
        return NULL;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    buffer = malloc((size_t)file_size + 1U);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    read_bytes = fread(buffer, 1U, (size_t)file_size, file);
    buffer[read_bytes] = '\0';
    fclose(file);
    return buffer;
}

static void test_normal_report(void)
{
    TextBuffer buffer;
    char *content;

    text_buffer_init(&buffer);
    expect_status("append normal text",
                  text_buffer_append(&buffer,
                                     "The quick brown fox jumps over the lazy dog."),
                  STATUS_SUCCESS);

    expect_status("generate normal report",
                  generate_report_file(&buffer, NORMAL_REPORT, "normal_source.txt"),
                  STATUS_SUCCESS);

    content = read_entire_file(NORMAL_REPORT);
    expect_true("normal report file exists and is readable", content != NULL);

    if (content != NULL) {
        expect_true("header exists", strstr(content, "TEXT ANALYSIS REPORT") != NULL);
        expect_true("source recorded", strstr(content, "normal_source.txt") != NULL);
        expect_true("total characters", strstr(content, "Total Characters       : 44") != NULL);
        expect_true("characters without space",
                    strstr(content, "Characters Excl Spaces : 36") != NULL);
        expect_true("word count", strstr(content, "Words                  : 9") != NULL);
        expect_true("sentence count", strstr(content, "Sentences              : 1") != NULL);
        expect_true("most frequent word is 'the'",
                    strstr(content, "Most Frequent Word     : the (2)") != NULL);
        expect_true("character frequency table exists",
                    strstr(content, "Character Frequency Table:") != NULL);
        expect_true("word frequency table exists",
                    strstr(content, "Word Frequency Table:") != NULL);
        free(content);
    }

    text_buffer_free(&buffer);
}

static void test_multiline_report(void)
{
    TextBuffer buffer;
    char *content;

    text_buffer_init(&buffer);
    expect_status("append multiline text",
                  text_buffer_append(&buffer,
                                     "First line.\nSecond line has words.\nThird line."),
                  STATUS_SUCCESS);

    expect_status("generate multiline report",
                  generate_report_file(&buffer, MULTILINE_REPORT, "multiline_source.txt"),
                  STATUS_SUCCESS);

    content = read_entire_file(MULTILINE_REPORT);
    expect_true("multiline report file exists and is readable", content != NULL);

    if (content != NULL) {
        expect_true("lines counted correctly",
                    strstr(content, "Lines                  : 3") != NULL);
        expect_true("sentences counted correctly",
                    strstr(content, "Sentences              : 3") != NULL);
        expect_true("paragraphs counted correctly",
                    strstr(content, "Paragraphs             : 1") != NULL);
        free(content);
    }

    text_buffer_free(&buffer);
}

static void test_empty_report(void)
{
    TextBuffer buffer;
    char *content;

    text_buffer_init(&buffer);

    expect_status("generate empty report",
                  generate_report_file(&buffer, EMPTY_REPORT, NULL),
                  STATUS_SUCCESS);

    content = read_entire_file(EMPTY_REPORT);
    expect_true("empty report file exists and is readable", content != NULL);

    if (content != NULL) {
        expect_true("empty source is N/A", strstr(content, "Source                 : N/A") != NULL);
        expect_true("zero characters", strstr(content, "Total Characters       : 0") != NULL);
        expect_true("zero words", strstr(content, "Words                  : 0") != NULL);
        expect_true("zero lines", strstr(content, "Lines                  : 0") != NULL);
        expect_true("longest word N/A", strstr(content, "Longest Word           : N/A") != NULL);
        expect_true("most frequent char N/A",
                    strstr(content, "Most Frequent Character: N/A") != NULL);
        expect_true("most frequent word N/A",
                    strstr(content, "Most Frequent Word     : N/A") != NULL);
        expect_true("none in tables", strstr(content, "(None)") != NULL);
        free(content);
    }

    text_buffer_free(&buffer);
}

static void test_top_words_report(void)
{
    TextBuffer buffer;
    char *content;

    text_buffer_init(&buffer);
    expect_status("append many distinct words",
                  text_buffer_append(&buffer,
                                     "alpha alpha alpha alpha alpha "
                                     "beta beta beta beta "
                                     "gamma gamma gamma "
                                     "delta delta "
                                     "epsilon zeta eta theta iota kappa lambda mu"),
                  STATUS_SUCCESS);

    expect_status("generate top words report",
                  generate_report_file(&buffer, TOP_WORDS_REPORT, "vocab_test"),
                  STATUS_SUCCESS);

    content = read_entire_file(TOP_WORDS_REPORT);
    expect_true("top words report exists and is readable", content != NULL);

    if (content != NULL) {
        expect_true("top words section exists", strstr(content, "Top 10 Words:") != NULL);
        expect_true("top 1 word is alpha", strstr(content, "1. alpha : 5") != NULL);
        expect_true("top 2 word is beta", strstr(content, "2. beta : 4") != NULL);
        expect_true("top 3 word is gamma", strstr(content, "3. gamma : 3") != NULL);
        expect_true("top 4 word is delta", strstr(content, "4. delta : 2") != NULL);
        free(content);
    }

    text_buffer_free(&buffer);
}

static void test_timestamped_default_report(void)
{
    TextBuffer buffer;
    char generated_path[REPORT_DEFAULT_MAX_PATH];
    FILE *file;

    text_buffer_init(&buffer);
    expect_status("append text for timestamp test",
                  text_buffer_append(&buffer, "Sample text for timestamp test."),
                  STATUS_SUCCESS);

    expect_status("generate timestamped report",
                  generate_report(&buffer, "timestamp_test", generated_path,
                                  sizeof(generated_path)),
                  STATUS_SUCCESS);

    expect_true("generated path starts with reports directory",
                strncmp(generated_path, "reports/analysis_report_", 24) == 0);

    file = fopen(generated_path, "rb");
    expect_true("timestamped report file can be opened", file != NULL);
    if (file != NULL) {
        fclose(file);
        remove(generated_path);
    }

    text_buffer_free(&buffer);
}

static void test_error_handling(void)
{
    TextBuffer buffer;
    char path[REPORT_DEFAULT_MAX_PATH];

    text_buffer_init(&buffer);
    text_buffer_append(&buffer, "Some text");

    expect_status("null buffer returns error",
                  generate_report_file(NULL, "build/out.txt", "source"),
                  STATUS_ERROR_INVALID);

    expect_status("null filepath returns error",
                  generate_report_file(&buffer, NULL, "source"),
                  STATUS_ERROR_INVALID);

    expect_status("empty filepath returns error",
                  generate_report_file(&buffer, "", "source"),
                  STATUS_ERROR_INVALID);

    expect_status("invalid directory returns file error",
                  generate_report_file(&buffer, INVALID_REPORT_PATH, "source"),
                  STATUS_ERROR_FILE);

    expect_status("generate_report with null buffer returns error",
                  generate_report(NULL, "source", path, sizeof(path)),
                  STATUS_ERROR_INVALID);

    text_buffer_free(&buffer);
}

int main(void)
{
    test_normal_report();
    test_multiline_report();
    test_empty_report();
    test_top_words_report();
    test_timestamped_default_report();
    test_error_handling();

    remove(NORMAL_REPORT);
    remove(MULTILINE_REPORT);
    remove(EMPTY_REPORT);
    remove(TOP_WORDS_REPORT);

    if (failures != 0) {
        printf("%d report test(s) failed.\n", failures);
        return 1;
    }

    puts("All report tests passed.");
    return 0;
}
