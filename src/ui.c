#include "ui.h"

#include "analyzer.h"
#include "frequency.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MENU_INPUT_SIZE 64U

static void display_menu(void)
{
    puts("========================================");
    puts("             TEXT ANALYZER");
    puts("========================================");
    puts("1. Enter Text");
    puts("2. View Current Text");
    puts("3. Clear Current Text");
    puts("4. Analyze Text");
    puts("5. Frequency Analysis");
    puts("0. Exit");
    fputs("\nEnter choice: ", stdout);
}

static int read_menu_choice(int *choice)
{
    char input[MENU_INPUT_SIZE];
    char *end;
    long value;

    if (choice == NULL || fgets(input, sizeof(input), stdin) == NULL) {
        return 0;
    }

    errno = 0;
    value = strtol(input, &end, 10);
    if (errno != 0 || end == input || value < INT_MIN || value > INT_MAX ||
        (*end != '\n' && *end != '\0')) {
        return -1;
    }

    *choice = (int)value;
    return 1;
}

static void enter_text(TextBuffer *buffer)
{
    Status status;

    puts("Enter your text.");
    puts("Press ENTER on an empty line to finish.");
    status = text_input_multiline(buffer, stdin);

    if (status == STATUS_ERROR_MEMORY) {
        puts("[ERROR] Memory allocation failed.");
    } else if (status == STATUS_ERROR_INPUT) {
        puts("[ERROR] Unable to read text input.");
    } else if (status == STATUS_INPUT_EOF) {
        puts("[Input ended at EOF.]");
    } else if (status != STATUS_SUCCESS) {
        puts("[ERROR] Invalid text input.");
    } else if (buffer->length == 0U) {
        puts("[No text entered.]");
    }
}

static void display_statistics(const TextStatistics *statistics)
{
    const char *longest_word = statistics->longest_word != NULL
                                   ? statistics->longest_word
                                   : "N/A";
    const char *shortest_word = statistics->shortest_word != NULL
                                    ? statistics->shortest_word
                                    : "N/A";

    puts("========================================");
    puts("          TEXT ANALYSIS");
    puts("========================================");
    puts("GENERAL STATISTICS");
    puts("------------------");
    printf("Characters              : %zu\n", statistics->characters);
    printf("Characters Without Space: %zu\n", statistics->characters_no_space);
    printf("Words                   : %zu\n", statistics->words);
    printf("Lines                   : %zu\n", statistics->lines);
    printf("Sentences               : %zu\n", statistics->sentences);
    printf("Paragraphs              : %zu\n\n", statistics->paragraphs);
    puts("CHARACTER ANALYSIS");
    puts("------------------");
    printf("Uppercase               : %zu\n", statistics->uppercase);
    printf("Lowercase               : %zu\n", statistics->lowercase);
    printf("Digits                  : %zu\n", statistics->digits);
    printf("Spaces                  : %zu\n", statistics->spaces);
    printf("Tabs                    : %zu\n", statistics->tabs);
    printf("Special Characters      : %zu\n\n", statistics->special_chars);
    puts("LETTER ANALYSIS");
    puts("---------------");
    printf("Vowels                  : %zu\n", statistics->vowels);
    printf("Consonants              : %zu\n\n", statistics->consonants);
    puts("WORD ANALYSIS");
    puts("-------------");
    printf("Longest Word            : %s\n", longest_word);
    printf("Shortest Word           : %s\n", shortest_word);
    printf("Average Word Length     : %.2f\n\n",
           statistics->average_word_length);
    puts("SENTENCE ANALYSIS");
    puts("-----------------");
    printf("Average Sentence Length : %.2f words\n\n",
           statistics->average_sentence_length);
    puts("READING ESTIMATE");
    puts("----------------");
    printf("Reading Time            : %.2f minutes\n",
           statistics->estimated_reading_time);
    puts("========================================");
}

static void analyze_current_text(const TextBuffer *buffer)
{
    TextStatistics statistics;
    Status status;

    if (buffer == NULL || buffer->data == NULL || buffer->length == 0U) {
        puts("[ERROR] No text available for analysis.");
        return;
    }

    text_statistics_init(&statistics);
    status = analyze_text(buffer->data, &statistics);
    if (status == STATUS_SUCCESS) {
        display_statistics(&statistics);
    } else if (status == STATUS_ERROR_MEMORY) {
        puts("[ERROR] Memory allocation failed during analysis.");
    } else {
        puts("[ERROR] Unable to analyze text.");
    }
    free_statistics(&statistics);
}

static void display_character_label(unsigned char character)
{
    if (isprint(character)) {
        printf("%c", character);
    } else {
        printf("0x%02X", character);
    }
}

static void display_character_frequency(
    const size_t frequency[CHARACTER_FREQUENCY_SIZE])
{
    size_t index;

    puts("========================================");
    puts("       CHARACTER FREQUENCY");
    puts("========================================");
    for (index = 0U; index < CHARACTER_FREQUENCY_SIZE; index++) {
        if (frequency[index] > 0U) {
            display_character_label((unsigned char)index);
            printf(" : %zu\n", frequency[index]);
        }
    }
}

static void display_word_frequency(const WordFrequencyTable *table)
{
    size_t index;

    puts("========================================");
    puts("          WORD FREQUENCY");
    puts("========================================");
    for (index = 0U; index < table->size; index++) {
        printf("%s : %zu\n", table->items[index].word, table->items[index].count);
    }
}

static void display_most_frequent_character(
    const size_t frequency[CHARACTER_FREQUENCY_SIZE])
{
    int character = get_most_frequent_character(frequency);

    if (character < 0) {
        puts("Most frequent character: N/A");
        return;
    }

    fputs("Most frequent character: ", stdout);
    display_character_label((unsigned char)character);
    printf(" (%zu)\n", frequency[character]);
}

static void display_most_frequent_word(const WordFrequencyTable *table)
{
    size_t index = get_most_frequent_word_index(table);

    if (index == SIZE_MAX) {
        puts("Most frequent word: N/A");
        return;
    }

    printf("Most frequent word: %s (%zu)\n", table->items[index].word,
           table->items[index].count);
}

static void display_top_words(const WordFrequencyTable *table)
{
    size_t indices[10];
    size_t count;
    size_t index;

    puts("===============================");
    puts("TOP WORDS");
    puts("===============================");
    count = get_top_word_indices(table, indices, 10U);
    if (count == 0U) {
        puts("N/A");
        return;
    }

    for (index = 0U; index < count; index++) {
        printf("%zu. %s : %zu\n", index + 1U,
               table->items[indices[index]].word,
               table->items[indices[index]].count);
    }
}

static int build_frequency_table(const TextBuffer *buffer,
                                 WordFrequencyTable *table)
{
    Status status;

    word_frequency_table_init(table);
    status = build_word_frequency(buffer->data, table);
    if (status == STATUS_ERROR_MEMORY) {
        puts("[ERROR] Memory allocation failed during frequency analysis.");
        return 0;
    }
    if (status != STATUS_SUCCESS) {
        puts("[ERROR] Unable to analyze word frequency.");
        return 0;
    }
    return 1;
}

static void display_frequency_menu(void)
{
    puts("======= FREQUENCY ANALYSIS =======");
    puts("1. Character Frequency");
    puts("2. Word Frequency");
    puts("3. Most Frequent Character");
    puts("4. Most Frequent Word");
    puts("5. Top 10 Words");
    puts("6. Complete Frequency Analysis");
    puts("0. Back");
    fputs("\nEnter choice: ", stdout);
}

static void frequency_analysis(const TextBuffer *buffer)
{
    size_t characters[CHARACTER_FREQUENCY_SIZE];
    WordFrequencyTable words;
    int choice;
    int read_result;

    if (buffer == NULL || buffer->data == NULL || buffer->length == 0U) {
        puts("[ERROR] No text available for frequency analysis.");
        return;
    }

    for (;;) {
        display_frequency_menu();
        read_result = read_menu_choice(&choice);
        if (read_result == 0 || choice == 0) {
            return;
        }
        if (read_result < 0 || choice < 0 || choice > 6) {
            puts("Invalid choice.");
            continue;
        }

        if (choice == 1 || choice == 3 || choice == 6) {
            calculate_character_frequency(buffer->data, characters);
        }
        if (choice == 1) {
            display_character_frequency(characters);
            continue;
        }
        if (choice == 3) {
            display_most_frequent_character(characters);
            continue;
        }

        if (!build_frequency_table(buffer, &words)) {
            continue;
        }

        if (choice == 2) {
            display_word_frequency(&words);
        } else if (choice == 4) {
            display_most_frequent_word(&words);
        } else if (choice == 5) {
            display_top_words(&words);
        } else {
            display_character_frequency(characters);
            display_word_frequency(&words);
            display_most_frequent_character(characters);
            display_most_frequent_word(&words);
            display_top_words(&words);
        }
        free_word_frequency_table(&words);
    }
}

Status ui_run(TextBuffer *buffer)
{
    int choice;
    int read_result;

    if (buffer == NULL) {
        return STATUS_ERROR_INVALID;
    }

    for (;;) {
        display_menu();
        read_result = read_menu_choice(&choice);
        if (read_result == 0) {
            puts("\nInput ended. Exiting.");
            return STATUS_SUCCESS;
        }
        if (read_result < 0) {
            puts("Invalid choice.");
            continue;
        }

        switch (choice) {
            case 1:
                enter_text(buffer);
                break;
            case 2:
                text_buffer_display(buffer, stdout);
                break;
            case 3:
                text_buffer_clear(buffer);
                puts("Current text cleared successfully.");
                break;
            case 4:
                analyze_current_text(buffer);
                break;
            case 5:
                frequency_analysis(buffer);
                break;
            case 0:
                return STATUS_SUCCESS;
            default:
                puts("Invalid choice.");
                break;
        }
    }
}
