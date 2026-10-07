#include <stdio.h>
#include <string.h>

#include "file_handler.h"

#define NORMAL_FILE "build/test_file_normal.txt"
#define EMPTY_FILE "build/test_file_empty.txt"
#define LARGE_FILE "build/test_file_large.txt"
#define ROUND_TRIP_FILE "build/test_file_round_trip.txt"

static int failures = 0;

static void expect_status(const char *name, Status actual, Status expected)
{
    if (actual != expected) {
        printf("FAIL: %s (expected %d, got %d)\n", name, expected, actual);
        failures++;
    }
}

static void expect_size(const char *name, size_t actual, size_t expected)
{
    if (actual != expected) {
        printf("FAIL: %s (expected %zu, got %zu)\n", name, expected, actual);
        failures++;
    }
}

static void expect_text(const char *name, const TextBuffer *buffer,
                        const char *expected)
{
    if (buffer->data == NULL || strcmp(buffer->data, expected) != 0) {
        printf("FAIL: %s\n", name);
        failures++;
    }
}

static int write_fixture(const char *filename, const char *content)
{
    FILE *file = fopen(filename, "wb");
    size_t length = strlen(content);

    if (file == NULL) {
        return 0;
    }
    if (length > 0U && fwrite(content, 1U, length, file) != length) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

static int write_large_fixture(const char *filename, size_t length)
{
    FILE *file = fopen(filename, "wb");
    size_t index;

    if (file == NULL) {
        return 0;
    }
    for (index = 0U; index < length; index++) {
        if (fputc('x', file) == EOF) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0;
}

static void test_loading(void)
{
    TextBuffer buffer;

    if (!write_fixture(NORMAL_FILE, "First line.\nSecond line.\n")) {
        puts("FAIL: could not create normal fixture");
        failures++;
        return;
    }
    text_buffer_init(&buffer);
    expect_status("load multiline file", load_text_file(NORMAL_FILE, &buffer),
                  STATUS_SUCCESS);
    expect_text("multiline preservation", &buffer, "First line.\nSecond line.\n");
    text_buffer_free(&buffer);

    if (!write_fixture(EMPTY_FILE, "")) {
        puts("FAIL: could not create empty fixture");
        failures++;
        return;
    }
    text_buffer_init(&buffer);
    expect_status("load empty file", load_text_file(EMPTY_FILE, &buffer),
                  STATUS_SUCCESS);
    expect_size("empty file length", buffer.length, 0U);
    text_buffer_free(&buffer);
}

static void test_large_file(void)
{
    TextBuffer buffer;

    if (!write_large_fixture(LARGE_FILE, 10000U)) {
        puts("FAIL: could not create large fixture");
        failures++;
        return;
    }
    text_buffer_init(&buffer);
    expect_status("load large file", load_text_file(LARGE_FILE, &buffer),
                  STATUS_SUCCESS);
    expect_size("large file length", buffer.length, 10000U);
    text_buffer_free(&buffer);
}

static void test_failures_preserve_text(void)
{
    TextBuffer buffer;

    text_buffer_init(&buffer);
    expect_status("seed existing text", text_buffer_append(&buffer, "Keep this"),
                  STATUS_SUCCESS);
    expect_status("missing file", load_text_file("build/no_such_file.txt", &buffer),
                  STATUS_ERROR_FILE);
    expect_text("preserve text after failed load", &buffer, "Keep this");
    text_buffer_free(&buffer);
}

static void test_saving_and_round_trip(void)
{
    TextBuffer original;
    TextBuffer loaded;

    text_buffer_init(&original);
    text_buffer_init(&loaded);
    expect_status("append round-trip text",
                  text_buffer_append(&original, "Line one\nLine two\n"),
                  STATUS_SUCCESS);
    expect_status("save normal file", save_text_file(ROUND_TRIP_FILE, &original),
                  STATUS_SUCCESS);
    expect_status("load round-trip file", load_text_file(ROUND_TRIP_FILE, &loaded),
                  STATUS_SUCCESS);
    expect_text("round-trip preservation", &loaded, "Line one\nLine two\n");
    text_buffer_free(&original);
    text_buffer_free(&loaded);

    text_buffer_init(&original);
    text_buffer_init(&loaded);
    expect_status("save empty file", save_text_file(EMPTY_FILE, &original),
                  STATUS_SUCCESS);
    expect_status("load saved empty file", load_text_file(EMPTY_FILE, &loaded),
                  STATUS_SUCCESS);
    expect_size("saved empty file length", loaded.length, 0U);
    text_buffer_free(&original);
    text_buffer_free(&loaded);
}

int main(void)
{
    test_loading();
    test_large_file();
    test_failures_preserve_text();
    test_saving_and_round_trip();

    remove(NORMAL_FILE);
    remove(EMPTY_FILE);
    remove(LARGE_FILE);
    remove(ROUND_TRIP_FILE);

    if (failures != 0) {
        printf("%d file-handler test(s) failed.\n", failures);
        return 1;
    }

    puts("All file-handler tests passed.");
    return 0;
}
