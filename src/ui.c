#include "ui.h"

#include "analyzer.h"
#include "file_handler.h"
#include "frequency.h"
#include "report.h"
#include "search.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MENU_INPUT_SIZE 64U
#define SEARCH_INPUT_SIZE 512U

static char current_source_info[SEARCH_INPUT_SIZE] = "Manual Input";

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
    puts("6. Search & Replace");
    puts("7. Load Text File");
    puts("8. Save Text File");
    puts("9. Generate Analysis Report");
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

static int read_text_line(const char *prompt, char *text, size_t capacity)
{
    char *newline;
    int character;

    if (prompt == NULL || text == NULL || capacity < 2U) {
        return -1;
    }

    fputs(prompt, stdout);
    if (fgets(text, (int)capacity, stdin) == NULL) {
        return 0;
    }

    newline = strchr(text, '\n');
    if (newline != NULL) {
        *newline = '\0';
        if (newline > text && newline[-1] == '\r') {
            newline[-1] = '\0';
        }
        return 1;
    }

    while ((character = fgetc(stdin)) != '\n' && character != EOF) {
    }
    puts("[ERROR] Input is too long.");
    return -1;
}

static int ask_yes_no(const char *prompt, int default_value, int *answer)
{
    char input[MENU_INPUT_SIZE];
    int read_result;

    if (answer == NULL) {
        return 0;
    }

    for (;;) {
        read_result = read_text_line(prompt, input, sizeof(input));
        if (read_result <= 0) {
            return read_result;
        }
        if (input[0] == '\0') {
            *answer = default_value;
            return 1;
        }
        if (input[1] == '\0' && (input[0] == 'y' || input[0] == 'Y')) {
            *answer = 1;
            return 1;
        }
        if (input[1] == '\0' && (input[0] == 'n' || input[0] == 'N')) {
            *answer = 0;
            return 1;
        }
        puts("Please enter y or n.");
    }
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
    } else {
        strncpy(current_source_info, "Manual Input", sizeof(current_source_info));
        current_source_info[sizeof(current_source_info) - 1U] = '\0';
    }
}

static void load_text_file_ui(TextBuffer *buffer)
{
    char filename[SEARCH_INPUT_SIZE];
    int read_result;
    Status status;

    read_result = read_text_line("File path: ", filename, sizeof(filename));
    if (read_result <= 0) {
        return;
    }
    if (filename[0] == '\0') {
        puts("[ERROR] File path cannot be empty.");
        return;
    }

    status = load_text_file(filename, buffer);
    if (status == STATUS_SUCCESS) {
        strncpy(current_source_info, filename, sizeof(current_source_info));
        current_source_info[sizeof(current_source_info) - 1U] = '\0';
        if (buffer->length == 0U) {
            puts("Text file loaded successfully. The file is empty.");
        } else {
            puts("Text file loaded successfully.");
        }
    } else if (status == STATUS_ERROR_MEMORY) {
        puts("[ERROR] Memory allocation failed. Current text was not changed.");
    } else if (status == STATUS_ERROR_INVALID) {
        puts("[ERROR] Invalid file path. Current text was not changed.");
    } else {
        puts("[ERROR] Unable to load file. Current text was not changed.");
    }
}

static void save_text_file_ui(const TextBuffer *buffer)
{
    char filename[SEARCH_INPUT_SIZE];
    int read_result;
    Status status;

    read_result = read_text_line("File path: ", filename, sizeof(filename));
    if (read_result <= 0) {
        return;
    }
    if (filename[0] == '\0') {
        puts("[ERROR] File path cannot be empty.");
        return;
    }

    status = save_text_file(filename, buffer);
    if (status == STATUS_SUCCESS) {
        puts("Text file saved successfully.");
    } else if (status == STATUS_ERROR_INVALID) {
        puts("[ERROR] Invalid file path or text buffer.");
    } else {
        puts("[ERROR] Unable to save file.");
    }
}

static void generate_report_ui(const TextBuffer *buffer)
{
    char report_path[REPORT_DEFAULT_MAX_PATH];
    const char *source;
    Status status;

    if (buffer == NULL || buffer->data == NULL || buffer->length == 0U) {
        puts("[ERROR] No text available to generate a report.");
        return;
    }

    source = current_source_info[0] != '\0' ? current_source_info : "Manual Input";
    status = generate_report(buffer, source, report_path, sizeof(report_path));
    if (status == STATUS_SUCCESS) {
        printf("Analysis report generated successfully: %s\n", report_path);
    } else if (status == STATUS_ERROR_MEMORY) {
        puts("[ERROR] Memory allocation failed during report generation.");
    } else if (status == STATUS_ERROR_FILE) {
        puts("[ERROR] Unable to create report file. Please verify 'reports' directory exists.");
    } else {
        puts("[ERROR] Unable to generate analysis report.");
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

static void display_search_result(const char *pattern, int case_sensitive,
                                  int whole_word, size_t occurrences)
{
    puts("========================================");
    puts("             SEARCH RESULT");
    puts("========================================");
    printf("Search: %s\n", pattern);
    printf("Mode: %s\n", case_sensitive ? "Case Sensitive" : "Case Insensitive");
    printf("Type: %s\n", whole_word ? "Whole Word" : "Substring/Phrase");
    if (occurrences == 0U) {
        puts("No occurrences found.");
    } else {
        printf("Occurrences found: %zu\n", occurrences);
    }
    puts("========================================");
}

static void search_current_text(const TextBuffer *buffer, int word_default)
{
    char pattern[SEARCH_INPUT_SIZE];
    int case_sensitive;
    int whole_word;
    int read_result;
    size_t occurrences;

    if (buffer == NULL || buffer->data == NULL || buffer->length == 0U) {
        puts("[ERROR] No text available for search.");
        return;
    }

    read_result = read_text_line("Search text: ", pattern, sizeof(pattern));
    if (read_result == 0) {
        return;
    }
    if (read_result < 0 || pattern[0] == '\0') {
        puts("[ERROR] Search pattern cannot be empty.");
        return;
    }
    if (ask_yes_no("Case-sensitive? (y/n): ", 0, &case_sensitive) <= 0 ||
        ask_yes_no(word_default ? "Whole-word matching? (y/n) [y]: "
                                : "Whole-word matching? (y/n) [n]: ",
                   word_default, &whole_word) <= 0) {
        return;
    }

    occurrences = count_occurrences(buffer->data, pattern, case_sensitive,
                                    whole_word);
    display_search_result(pattern, case_sensitive, whole_word, occurrences);
}

static void replace_current_text(TextBuffer *buffer, int word_default)
{
    char pattern[SEARCH_INPUT_SIZE];
    char replacement[SEARCH_INPUT_SIZE];
    char *result;
    int case_sensitive;
    int whole_word;
    int confirmed;
    int read_result;
    size_t matches;
    size_t replacements;
    Status status;

    if (buffer == NULL || buffer->data == NULL || buffer->length == 0U) {
        puts("[ERROR] No text available for search.");
        return;
    }

    read_result = read_text_line("Search text: ", pattern, sizeof(pattern));
    if (read_result == 0) {
        return;
    }
    if (read_result < 0 || pattern[0] == '\0') {
        puts("[ERROR] Search pattern cannot be empty.");
        return;
    }
    if (read_text_line("Replacement text: ", replacement,
                       sizeof(replacement)) <= 0) {
        return;
    }
    if (ask_yes_no("Case-sensitive? (y/n): ", 0, &case_sensitive) <= 0 ||
        ask_yes_no(word_default ? "Whole-word matching? (y/n) [y]: "
                                : "Whole-word matching? (y/n) [n]: ",
                   word_default, &whole_word) <= 0) {
        return;
    }

    matches = count_occurrences(buffer->data, pattern, case_sensitive,
                                whole_word);
    if (matches == 0U) {
        puts("No matching occurrences found.");
        puts("Text was not changed.");
        return;
    }
    printf("Original text contains %zu matching occurrence(s).\n", matches);
    if (replacement[0] == '\0') {
        puts("Empty replacement will remove matching text.");
    }
    if (ask_yes_no("Replace these occurrences? (y/n): ", 0, &confirmed) <= 0 ||
        !confirmed) {
        puts("Replacement cancelled.");
        return;
    }

    result = NULL;
    replacements = 0U;
    status = replace_text(buffer->data, pattern, replacement, case_sensitive,
                          whole_word, &result, &replacements);
    if (status == STATUS_ERROR_MEMORY) {
        puts("[ERROR] Memory allocation failed. Text was not changed.");
        return;
    }
    if (status != STATUS_SUCCESS || result == NULL) {
        puts("[ERROR] Replacement failed. Text was not changed.");
        return;
    }

    text_buffer_replace_data(buffer, result);
    printf("%zu occurrence(s) replaced successfully.\n", replacements);
}

static void display_search_menu(void)
{
    puts("======= SEARCH & REPLACE =======");
    puts("1. Search Word");
    puts("2. Search Phrase");
    puts("3. Count Occurrences");
    puts("4. Replace Word");
    puts("5. Replace Phrase");
    puts("0. Back");
    fputs("\nEnter choice: ", stdout);
}

static void search_and_replace(TextBuffer *buffer)
{
    int choice;
    int read_result;

    for (;;) {
        display_search_menu();
        read_result = read_menu_choice(&choice);
        if (read_result == 0 || choice == 0) {
            return;
        }
        if (read_result < 0 || choice < 0 || choice > 5) {
            puts("Invalid choice.");
            continue;
        }

        switch (choice) {
            case 1:
                search_current_text(buffer, 1);
                break;
            case 2:
            case 3:
                search_current_text(buffer, 0);
                break;
            case 4:
                replace_current_text(buffer, 1);
                break;
            case 5:
                replace_current_text(buffer, 0);
                break;
            default:
                break;
        }
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
                current_source_info[0] = '\0';
                puts("Current text cleared successfully.");
                break;
            case 4:
                analyze_current_text(buffer);
                break;
            case 5:
                frequency_analysis(buffer);
                break;
            case 6:
                search_and_replace(buffer);
                break;
            case 7:
                load_text_file_ui(buffer);
                break;
            case 8:
                save_text_file_ui(buffer);
                break;
            case 9:
                generate_report_ui(buffer);
                break;
            case 0:
                return STATUS_SUCCESS;
            default:
                puts("Invalid choice.");
                break;
        }
    }
}
