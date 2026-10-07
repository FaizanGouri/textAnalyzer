#include "text_utils.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_CHUNK_SIZE 256U

void text_buffer_init(TextBuffer *buffer)
{
    if (buffer == NULL) {
        return;
    }

    buffer->data = NULL;
    buffer->length = 0U;
    buffer->capacity = 0U;
}

void text_buffer_clear(TextBuffer *buffer)
{
    if (buffer == NULL) {
        return;
    }

    free(buffer->data);
    text_buffer_init(buffer);
}

void text_buffer_free(TextBuffer *buffer)
{
    text_buffer_clear(buffer);
}

Status text_buffer_append(TextBuffer *buffer, const char *text)
{
    if (text == NULL) {
        return STATUS_ERROR_INVALID;
    }

    return text_buffer_append_bytes(buffer, text, strlen(text));
}

Status text_buffer_append_bytes(TextBuffer *buffer, const char *data,
                                size_t length)
{
    size_t required_capacity;
    size_t new_capacity;
    char *new_data;

    if (buffer == NULL || (data == NULL && length > 0U)) {
        return STATUS_ERROR_INVALID;
    }

    if (length > SIZE_MAX - buffer->length - 1U) {
        return STATUS_ERROR_MEMORY;
    }

    required_capacity = buffer->length + length + 1U;
    if (required_capacity > buffer->capacity) {
        new_capacity = buffer->capacity == 0U ? INPUT_CHUNK_SIZE : buffer->capacity;
        while (new_capacity < required_capacity) {
            if (new_capacity > SIZE_MAX / 2U) {
                new_capacity = required_capacity;
                break;
            }
            new_capacity *= 2U;
        }

        new_data = realloc(buffer->data, new_capacity);
        if (new_data == NULL) {
            return STATUS_ERROR_MEMORY;
        }

        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }

    if (length > 0U) {
        memcpy(buffer->data + buffer->length, data, length);
    }
    buffer->length += length;
    buffer->data[buffer->length] = '\0';
    return STATUS_SUCCESS;
}

Status text_input_multiline(TextBuffer *buffer, FILE *input)
{
    char line[INPUT_CHUNK_SIZE];
    size_t line_length;
    Status status;

    if (buffer == NULL || input == NULL) {
        return STATUS_ERROR_INVALID;
    }

    text_buffer_clear(buffer);

    while (fgets(line, sizeof(line), input) != NULL) {
        line_length = strlen(line);
        if ((line_length == 1U && line[0] == '\n') ||
            (line_length == 2U && line[0] == '\r' && line[1] == '\n')) {
            return STATUS_SUCCESS;
        }

        status = text_buffer_append(buffer, line);
        if (status != STATUS_SUCCESS) {
            text_buffer_clear(buffer);
            return status;
        }
    }

    if (ferror(input) != 0) {
        text_buffer_clear(buffer);
        return STATUS_ERROR_INPUT;
    }

    return STATUS_INPUT_EOF;
}

void text_buffer_display(const TextBuffer *buffer, FILE *output)
{
    if (buffer == NULL || output == NULL || buffer->data == NULL ||
        buffer->length == 0U) {
        fputs("[No text available.]\n", output);
        return;
    }

    fputs(buffer->data, output);
    if (buffer->data[buffer->length - 1U] != '\n') {
        fputc('\n', output);
    }
}

void text_buffer_replace_data(TextBuffer *buffer, char *replacement)
{
    size_t replacement_length;

    if (buffer == NULL || replacement == NULL) {
        return;
    }

    replacement_length = strlen(replacement);
    text_buffer_clear(buffer);
    buffer->data = replacement;
    buffer->length = replacement_length;
    buffer->capacity = replacement_length + 1U;
}

int text_is_word_character(unsigned char character)
{
    return isalpha(character) != 0;
}

unsigned char text_to_lowercase(unsigned char character)
{
    return (unsigned char)tolower(character);
}
