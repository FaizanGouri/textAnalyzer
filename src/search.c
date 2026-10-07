#include "search.h"

#include "text_utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int characters_match(unsigned char left, unsigned char right,
                            int case_sensitive)
{
    if (!case_sensitive) {
        left = text_to_lowercase(left);
        right = text_to_lowercase(right);
    }
    return left == right;
}

static int pattern_matches_at(const char *text, const char *pattern,
                              size_t position, int case_sensitive)
{
    size_t index;

    for (index = 0U; pattern[index] != '\0'; index++) {
        if (text[position + index] == '\0' ||
            !characters_match((unsigned char)text[position + index],
                              (unsigned char)pattern[index], case_sensitive)) {
            return 0;
        }
    }
    return 1;
}

static int has_whole_word_boundaries(const char *text, size_t position,
                                     size_t pattern_length)
{
    if (position > 0U &&
        text_is_word_character((unsigned char)text[position - 1U])) {
        return 0;
    }
    if (text[position + pattern_length] != '\0' &&
        text_is_word_character((unsigned char)text[position + pattern_length])) {
        return 0;
    }
    return 1;
}

static int is_match(const char *text, const char *pattern, size_t position,
                    size_t pattern_length, int case_sensitive, int whole_word)
{
    return pattern_matches_at(text, pattern, position, case_sensitive) &&
           (!whole_word ||
            has_whole_word_boundaries(text, position, pattern_length));
}

int search_text(const char *text, const char *pattern, int case_sensitive,
                int whole_word)
{
    return count_occurrences(text, pattern, case_sensitive, whole_word) > 0U;
}

size_t count_occurrences(const char *text, const char *pattern,
                         int case_sensitive, int whole_word)
{
    size_t position;
    size_t pattern_length;
    size_t count = 0U;

    if (text == NULL || pattern == NULL || pattern[0] == '\0') {
        return 0U;
    }

    pattern_length = strlen(pattern);
    position = 0U;
    while (text[position] != '\0') {
        if (is_match(text, pattern, position, pattern_length, case_sensitive,
                     whole_word)) {
            count++;
            position += pattern_length;
        } else {
            position++;
        }
    }

    return count;
}

Status replace_text(const char *text, const char *search,
                    const char *replacement, int case_sensitive,
                    int whole_word, char **result,
                    size_t *replacement_count)
{
    size_t match_count;
    size_t text_length;
    size_t search_length;
    size_t replacement_length;
    size_t result_length;
    size_t position;
    size_t output_position;
    char *replacement_text;

    if (result == NULL || replacement_count == NULL || text == NULL ||
        search == NULL || replacement == NULL || search[0] == '\0') {
        return STATUS_ERROR_INPUT;
    }

    *result = NULL;
    *replacement_count = 0U;
    match_count = count_occurrences(text, search, case_sensitive, whole_word);
    if (match_count == 0U) {
        return STATUS_SUCCESS;
    }

    text_length = strlen(text);
    search_length = strlen(search);
    replacement_length = strlen(replacement);
    result_length = text_length;
    if (replacement_length >= search_length) {
        size_t growth = replacement_length - search_length;

        if (growth > 0U && match_count > (SIZE_MAX - result_length) / growth) {
            return STATUS_ERROR_MEMORY;
        }
        result_length += match_count * growth;
    } else {
        result_length -= match_count * (search_length - replacement_length);
    }
    if (result_length == SIZE_MAX) {
        return STATUS_ERROR_MEMORY;
    }

    replacement_text = malloc(result_length + 1U);
    if (replacement_text == NULL) {
        return STATUS_ERROR_MEMORY;
    }

    position = 0U;
    output_position = 0U;
    while (text[position] != '\0') {
        if (is_match(text, search, position, search_length, case_sensitive,
                     whole_word)) {
            if (replacement_length > 0U) {
                memcpy(replacement_text + output_position, replacement,
                       replacement_length);
                output_position += replacement_length;
            }
            position += search_length;
        } else {
            replacement_text[output_position++] = text[position++];
        }
    }
    replacement_text[output_position] = '\0';

    *result = replacement_text;
    *replacement_count = match_count;
    return STATUS_SUCCESS;
}
