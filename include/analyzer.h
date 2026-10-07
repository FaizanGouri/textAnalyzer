#ifndef TEXT_ANALYZER_ANALYZER_H
#define TEXT_ANALYZER_ANALYZER_H

#include <stddef.h>

#include "common.h"

typedef struct {
    size_t characters;
    size_t characters_no_space;
    size_t words;
    size_t lines;
    size_t sentences;
    size_t paragraphs;
    size_t spaces;
    size_t tabs;
    size_t digits;
    size_t special_chars;
    size_t uppercase;
    size_t lowercase;
    size_t vowels;
    size_t consonants;
    char *longest_word;
    char *shortest_word;
    double average_word_length;
    double average_sentence_length;
    double estimated_reading_time;
} TextStatistics;

void text_statistics_init(TextStatistics *statistics);
void free_statistics(TextStatistics *statistics);
Status analyze_text(const char *text, TextStatistics *statistics);

#endif
