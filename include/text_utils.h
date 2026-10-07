#ifndef TEXT_ANALYZER_TEXT_UTILS_H
#define TEXT_ANALYZER_TEXT_UTILS_H

#include <stddef.h>
#include <stdio.h>

#include "common.h"

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} TextBuffer;

void text_buffer_init(TextBuffer *buffer);
void text_buffer_clear(TextBuffer *buffer);
void text_buffer_free(TextBuffer *buffer);
Status text_buffer_append(TextBuffer *buffer, const char *text);
Status text_input_multiline(TextBuffer *buffer, FILE *input);
void text_buffer_display(const TextBuffer *buffer, FILE *output);
void text_buffer_replace_data(TextBuffer *buffer, char *replacement);
int text_is_word_character(unsigned char character);
unsigned char text_to_lowercase(unsigned char character);

#endif
