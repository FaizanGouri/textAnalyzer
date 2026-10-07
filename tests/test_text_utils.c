#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "text_utils.h"

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

static void expect_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        printf("FAIL: %s (expected %d, got %d)\n", name, expected, actual);
        failures++;
    }
}

static void test_initialization_and_clear(void)
{
    TextBuffer buffer;

    text_buffer_init(&buffer);
    expect_size("init length", buffer.length, 0U);
    expect_size("init capacity", buffer.capacity, 0U);
    expect_int("init data is NULL", buffer.data == NULL, 1);

    expect_status("append empty text", text_buffer_append(&buffer, ""), STATUS_SUCCESS);
    expect_size("empty append length", buffer.length, 0U);
    expect_int("empty append data null-terminated", buffer.data[0] == '\0', 1);

    text_buffer_clear(&buffer);
    expect_size("cleared length", buffer.length, 0U);
    expect_size("cleared capacity", buffer.capacity, 0U);
    expect_int("cleared data is NULL", buffer.data == NULL, 1);
}

static void test_small_appends(void)
{
    TextBuffer buffer;

    text_buffer_init(&buffer);

    expect_status("append 1 char", text_buffer_append_bytes(&buffer, "x", 1U), STATUS_SUCCESS);
    expect_size("1 char length", buffer.length, 1U);
    expect_int("1 char content", buffer.data[0] == 'x' && buffer.data[1] == '\0', 1);

    expect_status("append word", text_buffer_append(&buffer, " word"), STATUS_SUCCESS);
    expect_size("word length", buffer.length, 6U);
    expect_int("word content", strcmp(buffer.data, "x word") == 0, 1);

    expect_status("append newline", text_buffer_append(&buffer, "\nline2"), STATUS_SUCCESS);
    expect_int("multiline content", strcmp(buffer.data, "x word\nline2") == 0, 1);

    text_buffer_free(&buffer);
}

static void test_large_stress_growth(void)
{
    TextBuffer buffer;
    char chunk[500];
    size_t iteration;
    size_t i;

    text_buffer_init(&buffer);

    memset(chunk, 'A', sizeof(chunk) - 1U);
    chunk[sizeof(chunk) - 1U] = '\0';

    /* Append chunk 30 times (14,970 characters) */
    for (iteration = 0U; iteration < 30U; iteration++) {
        expect_status("stress append chunk", text_buffer_append(&buffer, chunk), STATUS_SUCCESS);
        expect_size("chunk iteration length", buffer.length, (iteration + 1U) * (sizeof(chunk) - 1U));
    }

    expect_size("final stress length", buffer.length, 14970U);
    expect_int("null-terminated at end", buffer.data[buffer.length] == '\0', 1);

    /* Verify content */
    for (i = 0U; i < buffer.length; i++) {
        if (buffer.data[i] != 'A') {
            printf("FAIL: stress buffer corrupted at index %zu\n", i);
            failures++;
            break;
        }
    }

    text_buffer_clear(&buffer);
    expect_size("stress cleared length", buffer.length, 0U);
    expect_int("stress cleared data NULL", buffer.data == NULL, 1);
}

static void test_replace_data(void)
{
    TextBuffer buffer;
    char *replacement_small;
    char *replacement_large;

    text_buffer_init(&buffer);
    expect_status("seed buffer", text_buffer_append(&buffer, "Initial text"), STATUS_SUCCESS);

    /* Replace with large dynamic text */
    replacement_large = malloc(5000U);
    expect_int("large alloc success", replacement_large != NULL, 1);
    if (replacement_large != NULL) {
        memset(replacement_large, 'Z', 4999U);
        replacement_large[4999U] = '\0';
        text_buffer_replace_data(&buffer, replacement_large);
        expect_size("replaced large length", buffer.length, 4999U);
        expect_int("replaced large data ptr", buffer.data == replacement_large, 1);
    }

    /* Replace with small dynamic text */
    replacement_small = malloc(6U);
    expect_int("small alloc success", replacement_small != NULL, 1);
    if (replacement_small != NULL) {
        strcpy(replacement_small, "Small");
        text_buffer_replace_data(&buffer, replacement_small);
        expect_size("replaced small length", buffer.length, 5U);
        expect_int("replaced small content", strcmp(buffer.data, "Small") == 0, 1);
    }

    text_buffer_free(&buffer);
}

static void test_repeated_cycles(void)
{
    TextBuffer buffer;
    int cycle;

    text_buffer_init(&buffer);

    for (cycle = 0; cycle < 100; cycle++) {
        expect_status("cycle append", text_buffer_append(&buffer, "Repeated cycle text line."),
                      STATUS_SUCCESS);
        expect_size("cycle length", buffer.length, 25U);
        text_buffer_clear(&buffer);
        expect_size("cycle cleared length", buffer.length, 0U);
    }

    text_buffer_free(&buffer);
}

static void test_character_helpers(void)
{
    expect_int("alpha is word char", text_is_word_character('A'), 1);
    expect_int("lower is word char", text_is_word_character('z'), 1);
    expect_int("digit is not word char", text_is_word_character('5'), 0);
    expect_int("space is not word char", text_is_word_character(' '), 0);
    expect_int("punct is not word char", text_is_word_character('!'), 0);

    expect_int("to lower A", text_to_lowercase('A'), (unsigned char)'a');
    expect_int("to lower a", text_to_lowercase('a'), (unsigned char)'a');
    expect_int("to lower punct", text_to_lowercase('!'), (unsigned char)'!');
}

static void test_multiline_input_simulation(void)
{
    TextBuffer buffer;
    FILE *f;
    Status status;

    text_buffer_init(&buffer);

    /* Test multiline stop on empty line */
    f = fopen("build/test_multiline_input.txt", "w");
    if (f != NULL) {
        fputs("Line one\nLine two\n\nIgnored line three\n", f);
        fclose(f);

        f = fopen("build/test_multiline_input.txt", "r");
        expect_int("input file open", f != NULL, 1);
        if (f != NULL) {
            status = text_input_multiline(&buffer, f);
            expect_status("multiline read status", status, STATUS_SUCCESS);
            expect_int("multiline content",
                       strcmp(buffer.data, "Line one\nLine two\n") == 0, 1);
            fclose(f);
        }
        remove("build/test_multiline_input.txt");
    }

    text_buffer_free(&buffer);
}

int main(void)
{
    test_initialization_and_clear();
    test_small_appends();
    test_large_stress_growth();
    test_replace_data();
    test_repeated_cycles();
    test_character_helpers();
    test_multiline_input_simulation();

    if (failures != 0) {
        printf("%d text_utils test(s) failed.\n", failures);
        return 1;
    }

    puts("All text-utils tests passed.");
    return 0;
}
