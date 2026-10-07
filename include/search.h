#ifndef TEXT_ANALYZER_SEARCH_H
#define TEXT_ANALYZER_SEARCH_H

#include <stddef.h>

#include "common.h"

int search_text(const char *text, const char *pattern, int case_sensitive,
                int whole_word);
size_t count_occurrences(const char *text, const char *pattern,
                         int case_sensitive, int whole_word);
Status replace_text(const char *text, const char *search,
                    const char *replacement, int case_sensitive,
                    int whole_word, char **result,
                    size_t *replacement_count);

#endif
