#include "analyzer.h"
#include "text_utils.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_READING_SPEED 200.0

static int is_vowel(unsigned char character)
{
    character = (unsigned char)tolower(character);
    return character == 'a' || character == 'e' || character == 'i' ||
           character == 'o' || character == 'u';
}

static Status copy_word(char **destination, const char *start, size_t length)
{
    char *copy;

    copy = malloc(length + 1U);
    if (copy == NULL) {
        return STATUS_ERROR_MEMORY;
    }

    memcpy(copy, start, length);
    copy[length] = '\0';
    free(*destination);
    *destination = copy;
    return STATUS_SUCCESS;
}

static Status record_word(TextStatistics *statistics, const char *start,
                          size_t length)
{
    Status status;

    statistics->words++;

    if (statistics->longest_word == NULL ||
        length > strlen(statistics->longest_word)) {
        status = copy_word(&statistics->longest_word, start, length);
        if (status != STATUS_SUCCESS) {
            return status;
        }
    }

    if (statistics->shortest_word == NULL ||
        length < strlen(statistics->shortest_word)) {
        status = copy_word(&statistics->shortest_word, start, length);
        if (status != STATUS_SUCCESS) {
            return status;
        }
    }

    return STATUS_SUCCESS;
}

void text_statistics_init(TextStatistics *statistics)
{
    if (statistics != NULL) {
        memset(statistics, 0, sizeof(*statistics));
    }
}

void free_statistics(TextStatistics *statistics)
{
    if (statistics == NULL) {
        return;
    }

    free(statistics->longest_word);
    free(statistics->shortest_word);
    text_statistics_init(statistics);
}

Status analyze_text(const char *text, TextStatistics *statistics)
{
    const char *word_start = NULL;
    size_t word_length = 0U;
    size_t total_word_characters = 0U;
    int in_sentence_ending = 0;
    int paragraph_active = 0;
    int line_has_content = 0;
    size_t index;
    unsigned char character;
    Status status;

    if (text == NULL || statistics == NULL) {
        return STATUS_ERROR_INVALID;
    }

    text_statistics_init(statistics);
    if (text[0] == '\0') {
        return STATUS_SUCCESS;
    }

    statistics->lines = 1U;

    for (index = 0U; text[index] != '\0'; index++) {
        character = (unsigned char)text[index];
        statistics->characters++;

        if (!isspace(character)) {
            statistics->characters_no_space++;
        }
        if (character == ' ') {
            statistics->spaces++;
        } else if (character == '\t') {
            statistics->tabs++;
        }

        if (isupper(character)) {
            statistics->uppercase++;
        }
        if (islower(character)) {
            statistics->lowercase++;
        }
        if (isdigit(character)) {
            statistics->digits++;
        }
        if (text_is_word_character(character)) {
            if (is_vowel(character)) {
                statistics->vowels++;
            } else {
                statistics->consonants++;
            }
        } else if (!isdigit(character) && character != ' ' &&
                   character != '\t' && character != '\n' &&
                   character != '\r') {
            statistics->special_chars++;
        }

        if (text_is_word_character(character)) {
            if (word_start == NULL) {
                word_start = text + index;
                word_length = 0U;
            }
            word_length++;
            total_word_characters++;
        } else if (word_start != NULL) {
            status = record_word(statistics, word_start, word_length);
            if (status != STATUS_SUCCESS) {
                free_statistics(statistics);
                return status;
            }
            word_start = NULL;
        }

        if (character == '.' || character == '!' || character == '?') {
            if (!in_sentence_ending) {
                statistics->sentences++;
                in_sentence_ending = 1;
            }
        } else {
            in_sentence_ending = 0;
        }

        if (character == '\n') {
            if (line_has_content) {
                if (!paragraph_active) {
                    statistics->paragraphs++;
                    paragraph_active = 1;
                }
            } else {
                paragraph_active = 0;
            }
            line_has_content = 0;
            if (text[index + 1U] != '\0') {
                statistics->lines++;
            }
        } else if (!isspace(character)) {
            line_has_content = 1;
        }
    }

    if (word_start != NULL) {
        status = record_word(statistics, word_start, word_length);
        if (status != STATUS_SUCCESS) {
            free_statistics(statistics);
            return status;
        }
    }

    if (line_has_content && !paragraph_active) {
        statistics->paragraphs++;
    }

    if (statistics->words > 0U) {
        statistics->average_word_length =
            (double)total_word_characters / (double)statistics->words;
    }
    if (statistics->sentences > 0U) {
        statistics->average_sentence_length =
            (double)statistics->words / (double)statistics->sentences;
    }
    statistics->estimated_reading_time =
        (double)statistics->words / DEFAULT_READING_SPEED;

    return STATUS_SUCCESS;
}
