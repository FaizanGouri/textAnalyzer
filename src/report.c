#include "report.h"

#include "analyzer.h"
#include "frequency.h"
#include "text_utils.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int file_exists(const char *path)
{
    FILE *file;

    if (path == NULL) {
        return 0;
    }

    file = fopen(path, "rb");
    if (file != NULL) {
        fclose(file);
        return 1;
    }

    return 0;
}

static Status make_unique_report_path(char *destination, size_t destination_size)
{
    time_t raw_time;
    struct tm *time_info;
    char timestamp[32];
    size_t counter = 0U;
    int written;

    if (destination == NULL || destination_size == 0U) {
        return STATUS_ERROR_INVALID;
    }

    raw_time = time(NULL);
    time_info = localtime(&raw_time);
    if (time_info == NULL) {
        return STATUS_ERROR_INVALID;
    }

    if (strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", time_info) == 0U) {
        return STATUS_ERROR_INVALID;
    }

    written = snprintf(destination, destination_size, "%s/analysis_report_%s.txt",
                       REPORT_DEFAULT_DIRECTORY, timestamp);
    if (written < 0 || (size_t)written >= destination_size) {
        return STATUS_ERROR_INVALID;
    }

    while (file_exists(destination)) {
        counter++;
        written = snprintf(destination, destination_size,
                           "%s/analysis_report_%s_%lu.txt",
                           REPORT_DEFAULT_DIRECTORY, timestamp,
                           (unsigned long)counter);
        if (written < 0 || (size_t)written >= destination_size) {
            return STATUS_ERROR_INVALID;
        }
    }

    return STATUS_SUCCESS;
}

static void write_character_label(FILE *file, unsigned char character)
{
    if (isprint(character)) {
        fprintf(file, "%c", character);
    } else {
        fprintf(file, "0x%02X", character);
    }
}

static Status write_report_content(FILE *file, const TextBuffer *buffer,
                                   const char *source_info)
{
    TextStatistics statistics;
    WordFrequencyTable word_table;
    size_t char_frequency[CHARACTER_FREQUENCY_SIZE];
    time_t raw_time;
    struct tm *time_info;
    char time_str[64];
    const char *text_data;
    const char *longest_word;
    const char *shortest_word;
    const char *source_str;
    size_t top_indices[10];
    size_t top_count;
    size_t index;
    int best_char;
    size_t best_word_idx;
    int has_chars;
    Status status;

    text_data = (buffer != NULL && buffer->data != NULL) ? buffer->data : "";

    text_statistics_init(&statistics);
    status = analyze_text(text_data, &statistics);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    calculate_character_frequency(text_data, char_frequency);

    word_frequency_table_init(&word_table);
    status = build_word_frequency(text_data, &word_table);
    if (status != STATUS_SUCCESS) {
        free_statistics(&statistics);
        return status;
    }

    raw_time = time(NULL);
    time_info = localtime(&raw_time);
    if (time_info == NULL ||
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info) == 0U) {
        strncpy(time_str, "N/A", sizeof(time_str) - 1U);
        time_str[sizeof(time_str) - 1U] = '\0';
    }

    source_str = (source_info != NULL && source_info[0] != '\0')
                     ? source_info
                     : "N/A";

    longest_word = (statistics.longest_word != NULL)
                       ? statistics.longest_word
                       : "N/A";
    shortest_word = (statistics.shortest_word != NULL)
                        ? statistics.shortest_word
                        : "N/A";

    fputs("==================================================\n", file);
    fputs("              TEXT ANALYSIS REPORT\n", file);
    fputs("==================================================\n\n", file);

    fputs("REPORT INFORMATION\n", file);
    fputs("------------------\n", file);
    fprintf(file, "Date/Time              : %s\n", time_str);
    fprintf(file, "Source                 : %s\n\n", source_str);

    fputs("1. GENERAL STATISTICS\n", file);
    fputs("---------------------\n", file);
    fprintf(file, "Total Characters       : %zu\n", statistics.characters);
    fprintf(file, "Characters Excl Spaces : %zu\n", statistics.characters_no_space);
    fprintf(file, "Words                  : %zu\n", statistics.words);
    fprintf(file, "Lines                  : %zu\n", statistics.lines);
    fprintf(file, "Sentences              : %zu\n", statistics.sentences);
    fprintf(file, "Paragraphs             : %zu\n\n", statistics.paragraphs);

    fputs("2. CHARACTER STATISTICS\n", file);
    fputs("-----------------------\n", file);
    fprintf(file, "Uppercase Letters      : %zu\n", statistics.uppercase);
    fprintf(file, "Lowercase Letters      : %zu\n", statistics.lowercase);
    fprintf(file, "Digits                 : %zu\n", statistics.digits);
    fprintf(file, "Spaces                 : %zu\n", statistics.spaces);
    fprintf(file, "Tabs                   : %zu\n", statistics.tabs);
    fprintf(file, "Special Characters     : %zu\n", statistics.special_chars);
    fprintf(file, "Vowels                 : %zu\n", statistics.vowels);
    fprintf(file, "Consonants             : %zu\n\n", statistics.consonants);

    fputs("3. WORD STATISTICS\n", file);
    fputs("------------------\n", file);
    fprintf(file, "Longest Word           : %s\n", longest_word);
    fprintf(file, "Shortest Word          : %s\n", shortest_word);
    fprintf(file, "Average Word Length    : %.2f\n", statistics.average_word_length);
    fprintf(file, "Average Sentence Length: %.2f words\n", statistics.average_sentence_length);
    fprintf(file, "Estimated Reading Time : %.2f minutes\n\n", statistics.estimated_reading_time);

    fputs("4. CHARACTER FREQUENCY\n", file);
    fputs("----------------------\n", file);
    best_char = get_most_frequent_character(char_frequency);
    if (best_char < 0) {
        fputs("Most Frequent Character: N/A\n\n", file);
    } else {
        fputs("Most Frequent Character: ", file);
        write_character_label(file, (unsigned char)best_char);
        fprintf(file, " (%zu)\n\n", char_frequency[best_char]);
    }

    fputs("Character Frequency Table:\n", file);
    has_chars = 0;
    for (index = 0U; index < CHARACTER_FREQUENCY_SIZE; index++) {
        if (char_frequency[index] > 0U) {
            has_chars = 1;
            fputs("  ", file);
            write_character_label(file, (unsigned char)index);
            fprintf(file, " : %zu\n", char_frequency[index]);
        }
    }
    if (!has_chars) {
        fputs("  (None)\n", file);
    }
    fputs("\n", file);

    fputs("5. WORD FREQUENCY\n", file);
    fputs("-----------------\n", file);
    best_word_idx = get_most_frequent_word_index(&word_table);
    if (best_word_idx == SIZE_MAX) {
        fputs("Most Frequent Word     : N/A\n\n", file);
    } else {
        fprintf(file, "Most Frequent Word     : %s (%zu)\n\n",
                word_table.items[best_word_idx].word,
                word_table.items[best_word_idx].count);
    }

    fputs("Top 10 Words:\n", file);
    top_count = get_top_word_indices(&word_table, top_indices, 10U);
    if (top_count == 0U) {
        fputs("  N/A\n", file);
    } else {
        for (index = 0U; index < top_count; index++) {
            fprintf(file, "  %zu. %s : %zu\n", index + 1U,
                    word_table.items[top_indices[index]].word,
                    word_table.items[top_indices[index]].count);
        }
    }
    fputs("\n", file);

    fputs("Word Frequency Table:\n", file);
    if (word_table.size == 0U) {
        fputs("  (None)\n", file);
    } else {
        for (index = 0U; index < word_table.size; index++) {
            fprintf(file, "  %s : %zu\n",
                    word_table.items[index].word,
                    word_table.items[index].count);
        }
    }
    fputs("\n", file);

    fputs("==================================================\n", file);

    free_word_frequency_table(&word_table);
    free_statistics(&statistics);

    if (ferror(file) != 0) {
        return STATUS_ERROR_FILE;
    }

    return STATUS_SUCCESS;
}

Status generate_report_file(const TextBuffer *buffer, const char *filepath,
                            const char *source_info)
{
    FILE *file;
    Status status;

    if (buffer == NULL || filepath == NULL || filepath[0] == '\0') {
        return STATUS_ERROR_INVALID;
    }

    file = fopen(filepath, "w");
    if (file == NULL) {
        return STATUS_ERROR_FILE;
    }

    status = write_report_content(file, buffer, source_info);
    if (fclose(file) != 0 && status == STATUS_SUCCESS) {
        return STATUS_ERROR_FILE;
    }

    return status;
}

Status generate_report(const TextBuffer *buffer, const char *source_info,
                       char *generated_path, size_t path_size)
{
    char target_path[REPORT_DEFAULT_MAX_PATH];
    Status status;

    if (buffer == NULL) {
        return STATUS_ERROR_INVALID;
    }

    status = make_unique_report_path(target_path, sizeof(target_path));
    if (status != STATUS_SUCCESS) {
        return status;
    }

    status = generate_report_file(buffer, target_path, source_info);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    if (generated_path != NULL && path_size > 0U) {
        strncpy(generated_path, target_path, path_size - 1U);
        generated_path[path_size - 1U] = '\0';
    }

    return STATUS_SUCCESS;
}
