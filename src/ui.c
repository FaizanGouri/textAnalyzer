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

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <unistd.h>
#endif

#define MENU_INPUT_SIZE 64U
#define SEARCH_INPUT_SIZE 512U

/* ANSI Color Theme */
#define UI_COLOR_RESET        "\033[0m"
#define UI_COLOR_BOLD         "\033[1m"
#define UI_COLOR_DIM          "\033[2m"
#define UI_COLOR_RED          "\033[31m"
#define UI_COLOR_GREEN        "\033[32m"
#define UI_COLOR_YELLOW       "\033[33m"
#define UI_COLOR_BLUE         "\033[34m"
#define UI_COLOR_MAGENTA      "\033[35m"
#define UI_COLOR_CYAN         "\033[36m"
#define UI_COLOR_WHITE        "\033[37m"
#define UI_COLOR_BRIGHT_CYAN  "\033[96m"
#define UI_COLOR_BRIGHT_BLUE  "\033[94m"
#define UI_COLOR_BRIGHT_GREEN "\033[92m"

static int color_enabled = 1;
static char current_source_info[SEARCH_INPUT_SIZE] = "Manual Input";

void ui_set_color_enabled(int enabled)
{
    color_enabled = enabled ? 1 : 0;
}

int ui_is_color_enabled(void)
{
    return color_enabled;
}

static const char *c_val(const char *code)
{
    return color_enabled ? code : "";
}

static int is_interactive_terminal(void)
{
#ifdef _WIN32
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (hIn != INVALID_HANDLE_VALUE && GetConsoleMode(hIn, &mode)) {
        return 1;
    }
    return 0;
#else
    return isatty(0);
#endif
}

static void ui_init_terminal(void)
{
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleOutputCP(CP_UTF8);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
    }
#endif
    if (getenv("NO_COLOR") != NULL) {
        color_enabled = 0;
    }
}

static void ui_pause(void)
{
    if (is_interactive_terminal()) {
        char buffer[64];
        printf("\n%sPress Enter to continue › %s",
               c_val(UI_COLOR_DIM), c_val(UI_COLOR_RESET));
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            /* Input ended */
        }
    }
}

/* Status and Notification Helpers */
static void ui_msg_success(const char *text)
{
    printf("%s[SUCCESS]%s %s\n", c_val(UI_COLOR_GREEN), c_val(UI_COLOR_RESET), text);
}

static void ui_msg_error(const char *text)
{
    printf("%s[ERROR]%s %s\n", c_val(UI_COLOR_RED), c_val(UI_COLOR_RESET), text);
}

static void ui_msg_warning(const char *text)
{
    printf("%s[WARNING]%s %s\n", c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_RESET), text);
}

static void ui_msg_info(const char *text)
{
    printf("%s[INFO]%s %s\n", c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET), text);
}

static void print_section_header(const char *title)
{
    printf("\n%s════════════════ %s%s%s ════════════════%s\n\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_BOLD), title,
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
}

static void display_banner(void)
{
    printf("%s╔══════════════════════════════════════════════════════╗%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s║%s                                                      %s║%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s║%s                 %s%sTEXT ANALYZER%s                        %s║%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BOLD), c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s║%s                                                      %s║%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s║%s            %sC11 • TEXT ANALYTICS TOOL%s                 %s║%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s║%s                                                      %s║%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s╚══════════════════════════════════════════════════════╝%s\n\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
}

static size_t quick_count_words(const char *text)
{
    size_t count = 0U;
    int in_word = 0;
    size_t index;

    if (text == NULL) {
        return 0U;
    }

    for (index = 0U; text[index] != '\0'; index++) {
        if (text_is_word_character((unsigned char)text[index])) {
            if (!in_word) {
                in_word = 1;
                count++;
            }
        } else {
            in_word = 0;
        }
    }

    return count;
}

static void display_status_bar(const TextBuffer *buffer)
{
    printf("%s────────────────────────────────────────────────────%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    if (buffer == NULL || buffer->data == NULL || buffer->length == 0U) {
        printf("STATUS: %s● EMPTY BUFFER%s\n",
               c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_RESET));
        printf("WORDS: 0      CHARACTERS: 0\n");
    } else {
        size_t words = quick_count_words(buffer->data);
        printf("STATUS: %s● TEXT LOADED%s\n",
               c_val(UI_COLOR_GREEN), c_val(UI_COLOR_RESET));
        printf("WORDS: %-6zu CHARACTERS: %zu\n", words, buffer->length);
    }
    printf("%s────────────────────────────────────────────────────%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
}

static void display_main_menu(const TextBuffer *buffer)
{
    display_status_bar(buffer);
    printf("%s┌──────────────────── MAIN MENU ────────────────────┐%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s                                                   %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s1%s]%s  Enter Text                                 %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s2%s]%s  View Current Text                          %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s3%s]%s  Clear Text                                 %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s4%s]%s  Analyze Text                               %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s5%s]%s  Frequency Analysis                         %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s6%s]%s  Search & Replace                           %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s7%s]%s  Load Text File                             %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s8%s]%s  Save Text File                             %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s9%s]%s  Generate Analysis Report                   %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s                                                   %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s0%s]%s  Exit                                       %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s│%s                                                   %s│%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s└───────────────────────────────────────────────────┘%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("\n%sSelect an option › %s", c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
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

    printf("%s%s%s", c_val(UI_COLOR_CYAN), prompt, c_val(UI_COLOR_RESET));
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
    ui_msg_error("Input is too long.");
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
        if ((input[0] == 'y' || input[0] == 'Y') && input[1] == '\0') {
            *answer = 1;
            return 1;
        }
        if ((input[0] == 'n' || input[0] == 'N') && input[1] == '\0') {
            *answer = 0;
            return 1;
        }
        ui_msg_error("Please enter 'y' for yes or 'n' for no.");
    }
}

static void enter_text(TextBuffer *buffer)
{
    Status status;

    print_section_header("TEXT INPUT");
    printf("%sEnter text below. Press ENTER on an empty line to finish:%s\n\n",
           c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_RESET));
    status = text_input_multiline(buffer, stdin);

    if (status == STATUS_ERROR_MEMORY) {
        ui_msg_error("Memory allocation failed.");
    } else if (status == STATUS_ERROR_INPUT) {
        ui_msg_error("Unable to read text input.");
    } else if (status == STATUS_INPUT_EOF) {
        ui_msg_info("Input stream closed at EOF.");
    } else if (status != STATUS_SUCCESS) {
        ui_msg_error("Invalid text input.");
    } else if (buffer->length == 0U) {
        ui_msg_warning("No text was entered.");
    } else {
        strncpy(current_source_info, "Manual Input", sizeof(current_source_info));
        current_source_info[sizeof(current_source_info) - 1U] = '\0';
        ui_msg_success("Text entered and stored in buffer successfully.");
    }
    ui_pause();
}

static void display_current_text(const TextBuffer *buffer)
{
    print_section_header("CURRENT TEXT");

    if (buffer == NULL || buffer->data == NULL || buffer->length == 0U) {
        ui_msg_warning("No text is currently loaded.");
        ui_pause();
        return;
    }

    printf("%s╔════════════════════ CURRENT TEXT ═══════════════════╗%s\n\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    fputs(buffer->data, stdout);
    if (buffer->data[buffer->length - 1U] != '\n') {
        putchar('\n');
    }
    printf("\n%s╚═════════════════════════════════════════════════════╝%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    ui_pause();
}

static void load_text_file_ui(TextBuffer *buffer)
{
    char filename[SEARCH_INPUT_SIZE];
    int read_result;
    Status status;

    print_section_header("FILE LOAD");

    read_result = read_text_line("File path › ", filename, sizeof(filename));
    if (read_result <= 0) {
        return;
    }
    if (filename[0] == '\0') {
        ui_msg_error("File path cannot be empty.");
        ui_pause();
        return;
    }

    ui_msg_info("Loading file...");
    status = load_text_file(filename, buffer);
    if (status == STATUS_SUCCESS) {
        strncpy(current_source_info, filename, sizeof(current_source_info));
        current_source_info[sizeof(current_source_info) - 1U] = '\0';
        if (buffer->length == 0U) {
            ui_msg_success("File loaded successfully. Note: the file is empty.");
        } else {
            ui_msg_success("File loaded successfully.");
        }
    } else if (status == STATUS_ERROR_MEMORY) {
        ui_msg_error("Memory allocation failed. Current text was not changed.");
    } else if (status == STATUS_ERROR_INVALID) {
        ui_msg_error("Invalid file path. Current text was not changed.");
    } else {
        ui_msg_error("File could not be loaded. Current text was not changed.");
    }
    ui_pause();
}

static void save_text_file_ui(const TextBuffer *buffer)
{
    char filename[SEARCH_INPUT_SIZE];
    int read_result;
    Status status;

    print_section_header("FILE SAVE");

    read_result = read_text_line("File path › ", filename, sizeof(filename));
    if (read_result <= 0) {
        return;
    }
    if (filename[0] == '\0') {
        ui_msg_error("File path cannot be empty.");
        ui_pause();
        return;
    }

    ui_msg_info("Saving file...");
    status = save_text_file(filename, buffer);
    if (status == STATUS_SUCCESS) {
        ui_msg_success("File saved successfully.");
    } else if (status == STATUS_ERROR_INVALID) {
        ui_msg_error("Invalid file path or text buffer.");
    } else {
        ui_msg_error("File could not be saved.");
    }
    ui_pause();
}

static void generate_report_ui(const TextBuffer *buffer)
{
    char report_path[REPORT_DEFAULT_MAX_PATH];
    const char *source;
    Status status;

    print_section_header("REPORT GENERATION");

    if (buffer == NULL || buffer->data == NULL || buffer->length == 0U) {
        ui_msg_warning("No text available to generate a report.");
        ui_pause();
        return;
    }

    ui_msg_info("Preparing analysis...");
    ui_msg_info("Writing report to disk...");

    source = current_source_info[0] != '\0' ? current_source_info : "Manual Input";
    status = generate_report(buffer, source, report_path, sizeof(report_path));
    if (status == STATUS_SUCCESS) {
        ui_msg_success("Report generated successfully.");
        printf("%sPath:%s %s%s%s\n",
               c_val(UI_COLOR_BOLD), c_val(UI_COLOR_RESET),
               c_val(UI_COLOR_CYAN), report_path, c_val(UI_COLOR_RESET));
    } else if (status == STATUS_ERROR_MEMORY) {
        ui_msg_error("Memory allocation failed during report generation.");
    } else if (status == STATUS_ERROR_FILE) {
        ui_msg_error("Unable to write report file. Please verify 'reports' directory exists.");
    } else {
        ui_msg_error("Unable to generate analysis report.");
    }
    ui_pause();
}

static void display_statistics_dashboard(const TextStatistics *statistics)
{
    const char *longest_word = statistics->longest_word != NULL
                                   ? statistics->longest_word
                                   : "N/A";
    const char *shortest_word = statistics->shortest_word != NULL
                                    ? statistics->shortest_word
                                    : "N/A";

    printf("\n%s╔════════════════ GENERAL STATISTICS ════════════════╗%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("  Total Characters       : %s%zu%s\n",
           c_val(UI_COLOR_BOLD), statistics->characters, c_val(UI_COLOR_RESET));
    printf("  Characters No Space    : %zu\n", statistics->characters_no_space);
    printf("  Words                  : %s%zu%s\n",
           c_val(UI_COLOR_BOLD), statistics->words, c_val(UI_COLOR_RESET));
    printf("  Lines                  : %zu\n", statistics->lines);
    printf("  Sentences              : %zu\n", statistics->sentences);
    printf("  Paragraphs             : %zu\n", statistics->paragraphs);
    printf("%s╚════════════════════════════════════════════════════╝%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));

    printf("\n%s╔══════════════ CHARACTER STATISTICS ════════════════╗%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("  Uppercase              : %zu\n", statistics->uppercase);
    printf("  Lowercase              : %zu\n", statistics->lowercase);
    printf("  Digits                 : %zu\n", statistics->digits);
    printf("  Spaces                 : %zu\n", statistics->spaces);
    printf("  Tabs                   : %zu\n", statistics->tabs);
    printf("  Special Characters     : %zu\n", statistics->special_chars);
    printf("  Vowels                 : %zu\n", statistics->vowels);
    printf("  Consonants             : %zu\n", statistics->consonants);
    printf("%s╚════════════════════════════════════════════════════╝%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));

    printf("\n%s╔════════════════ WORD & READING ═══════════════════╗%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("  Longest Word           : %s%s%s\n",
           c_val(UI_COLOR_BOLD), longest_word, c_val(UI_COLOR_RESET));
    printf("  Shortest Word          : %s%s%s\n",
           c_val(UI_COLOR_BOLD), shortest_word, c_val(UI_COLOR_RESET));
    printf("  Average Word Length    : %.2f\n", statistics->average_word_length);
    printf("  Average Sentence Length: %.2f words\n", statistics->average_sentence_length);
    printf("  Estimated Reading Time : %s%.2f minutes%s\n",
           c_val(UI_COLOR_BOLD), statistics->estimated_reading_time, c_val(UI_COLOR_RESET));
    printf("%s╚════════════════════════════════════════════════════╝%s\n\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
}

static void analyze_current_text(const TextBuffer *buffer)
{
    TextStatistics statistics;
    Status status;

    print_section_header("TEXT ANALYSIS");

    if (buffer == NULL || buffer->data == NULL || buffer->length == 0U) {
        ui_msg_warning("No text available for analysis.");
        ui_pause();
        return;
    }

    text_statistics_init(&statistics);
    status = analyze_text(buffer->data, &statistics);
    if (status == STATUS_SUCCESS) {
        display_statistics_dashboard(&statistics);
    } else if (status == STATUS_ERROR_MEMORY) {
        ui_msg_error("Memory allocation failed during analysis.");
    } else {
        ui_msg_error("Unable to analyze text.");
    }

    free_statistics(&statistics);
    ui_pause();
}

static void display_character_frequency(
    const size_t frequency[CHARACTER_FREQUENCY_SIZE])
{
    size_t index;

    print_section_header("CHARACTER FREQUENCY");
    printf("%s────────────────────────────────%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s%-16s %s%s\n",
           c_val(UI_COLOR_BOLD), "Character", "Count", c_val(UI_COLOR_RESET));
    printf("%s────────────────────────────────%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    for (index = 0U; index < CHARACTER_FREQUENCY_SIZE; index++) {
        if (frequency[index] > 0U) {
            char label[16];
            if (isprint((unsigned char)index)) {
                snprintf(label, sizeof(label), "'%c'", (char)index);
            } else {
                snprintf(label, sizeof(label), "0x%02X", (unsigned int)index);
            }
            printf("%-16s %zu\n", label, frequency[index]);
        }
    }
    printf("%s────────────────────────────────%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
}

static void display_word_frequency(const WordFrequencyTable *table)
{
    size_t index;

    print_section_header("WORD FREQUENCY");
    printf("%s──────────────────────────────────────%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s%-24s %s%s\n",
           c_val(UI_COLOR_BOLD), "Word", "Count", c_val(UI_COLOR_RESET));
    printf("%s──────────────────────────────────────%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    for (index = 0U; index < table->size; index++) {
        printf("%-24s %zu\n", table->items[index].word, table->items[index].count);
    }
    printf("%s──────────────────────────────────────%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
}

static void display_most_frequent_character(
    const size_t frequency[CHARACTER_FREQUENCY_SIZE])
{
    int character = get_most_frequent_character(frequency);

    if (character < 0) {
        ui_msg_warning("Most frequent character: N/A");
        return;
    }

    if (isprint((unsigned char)character)) {
        printf("\n%sMost Frequent Character :%s %s'%c'%s (%zu occurrences)\n\n",
               c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
               c_val(UI_COLOR_BOLD), (char)character, c_val(UI_COLOR_RESET),
               frequency[character]);
    } else {
        printf("\n%sMost Frequent Character :%s %s0x%02X%s (%zu occurrences)\n\n",
               c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
               c_val(UI_COLOR_BOLD), (unsigned int)character, c_val(UI_COLOR_RESET),
               frequency[character]);
    }
}

static void display_most_frequent_word(const WordFrequencyTable *table)
{
    size_t index = get_most_frequent_word_index(table);

    if (index == SIZE_MAX) {
        ui_msg_warning("Most frequent word: N/A");
        return;
    }

    printf("\n%sMost Frequent Word      :%s %s\"%s\"%s (%zu occurrences)\n\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BOLD), table->items[index].word, c_val(UI_COLOR_RESET),
           table->items[index].count);
}

static void display_top_words(const WordFrequencyTable *table)
{
    size_t indices[10];
    size_t count;
    size_t index;

    print_section_header("TOP 10 WORDS");
    count = get_top_word_indices(table, indices, 10U);
    if (count == 0U) {
        ui_msg_warning("No words available.");
        return;
    }

    printf("%s──────────────────────────────────────────%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    printf("%s%-6s %-24s %s%s\n",
           c_val(UI_COLOR_BOLD), "Rank", "Word", "Count", c_val(UI_COLOR_RESET));
    printf("%s──────────────────────────────────────────%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    for (index = 0U; index < count; index++) {
        char rank_buf[8];
        snprintf(rank_buf, sizeof(rank_buf), "%lu.", (unsigned long)(index + 1U));
        printf("%-6s %-24s %zu\n",
               rank_buf,
               table->items[indices[index]].word,
               table->items[indices[index]].count);
    }
    printf("%s──────────────────────────────────────────%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
}

static int build_frequency_table(const TextBuffer *buffer,
                                 WordFrequencyTable *table)
{
    Status status;

    word_frequency_table_init(table);
    status = build_word_frequency(buffer->data, table);
    if (status == STATUS_ERROR_MEMORY) {
        ui_msg_error("Memory allocation failed during frequency analysis.");
        return 0;
    }
    if (status != STATUS_SUCCESS) {
        ui_msg_error("Unable to perform frequency analysis.");
        return 0;
    }

    return 1;
}

static void display_frequency_menu(void)
{
    printf("\n%s┌──────────────── FREQUENCY ANALYSIS ───────────────┐%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s                                                   %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s1%s]%s  Character Frequency                         %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s2%s]%s  Word Frequency                              %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s3%s]%s  Most Frequent Character                     %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s4%s]%s  Most Frequent Word                          %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s5%s]%s  Top 10 Words                                %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s6%s]%s  Complete Frequency Analysis                 %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s                                                   %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s0%s]%s  Back to Main Menu                           %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s                                                   %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s└───────────────────────────────────────────────────┘%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("\n%sSelect an option › %s", c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
}

static void frequency_analysis(const TextBuffer *buffer)
{
    size_t character_frequency[CHARACTER_FREQUENCY_SIZE];
    WordFrequencyTable word_table;
    int choice;
    int read_result;

    if (buffer == NULL || buffer->data == NULL || buffer->length == 0U) {
        ui_msg_warning("No text available for frequency analysis.");
        ui_pause();
        return;
    }

    calculate_character_frequency(buffer->data, character_frequency);
    if (!build_frequency_table(buffer, &word_table)) {
        ui_pause();
        return;
    }

    for (;;) {
        display_frequency_menu();
        read_result = read_menu_choice(&choice);
        if (read_result == 0) {
            break;
        }
        if (read_result < 0) {
            ui_msg_error("Invalid choice. Please select an option from the menu.");
            continue;
        }

        switch (choice) {
            case 0:
                free_word_frequency_table(&word_table);
                return;
            case 1:
                display_character_frequency(character_frequency);
                ui_pause();
                break;
            case 2:
                display_word_frequency(&word_table);
                ui_pause();
                break;
            case 3:
                display_most_frequent_character(character_frequency);
                ui_pause();
                break;
            case 4:
                display_most_frequent_word(&word_table);
                ui_pause();
                break;
            case 5:
                display_top_words(&word_table);
                ui_pause();
                break;
            case 6:
                display_character_frequency(character_frequency);
                display_most_frequent_character(character_frequency);
                display_word_frequency(&word_table);
                display_most_frequent_word(&word_table);
                display_top_words(&word_table);
                ui_pause();
                break;
            default:
                ui_msg_error("Invalid choice. Please select an option from the menu.");
                break;
        }
    }

    free_word_frequency_table(&word_table);
}

static void display_search_menu(void)
{
    printf("\n%s┌──────────────── SEARCH & REPLACE ─────────────────┐%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s                                                   %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s1%s]%s  Search Word                                 %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s2%s]%s  Search Phrase                               %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s3%s]%s  Count Occurrences                           %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s4%s]%s  Replace Word                                %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s5%s]%s  Replace Phrase                              %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s                                                   %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s  %s[%s0%s]%s  Back to Main Menu                           %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_YELLOW), c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s│%s                                                   %s│%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET),
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("%s└───────────────────────────────────────────────────┘%s\n",
           c_val(UI_COLOR_BLUE), c_val(UI_COLOR_RESET));
    printf("\n%sSelect an option › %s", c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
}

static void search_current_text(const TextBuffer *buffer, int whole_word)
{
    char pattern[SEARCH_INPUT_SIZE];
    int case_sensitive;
    int ask_result;
    size_t occurrences;

    print_section_header(whole_word ? "SEARCH WORD" : "SEARCH TEXT");

    if (read_text_line("Search text › ", pattern, sizeof(pattern)) <= 0) {
        return;
    }
    if (pattern[0] == '\0') {
        ui_msg_error("Search pattern cannot be empty.");
        ui_pause();
        return;
    }

    ask_result = ask_yes_no("Case-sensitive? (y/n) [n] › ", 0, &case_sensitive);
    if (ask_result <= 0) {
        return;
    }

    occurrences = count_occurrences(buffer->data, pattern, case_sensitive, whole_word);
    printf("%s──────────────────────────────────────────%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    if (occurrences > 0U) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Pattern found! Total occurrences: %lu",
                 (unsigned long)occurrences);
        ui_msg_success(msg);
    } else {
        ui_msg_warning("Pattern not found in the current text.");
    }
    printf("%s──────────────────────────────────────────%s\n",
           c_val(UI_COLOR_CYAN), c_val(UI_COLOR_RESET));
    ui_pause();
}

static void replace_current_text(TextBuffer *buffer, int whole_word)
{
    char search_pattern[SEARCH_INPUT_SIZE];
    char replacement[SEARCH_INPUT_SIZE];
    int case_sensitive;
    int confirmed;
    int ask_result;
    size_t occurrences;
    size_t replacement_count = 0U;
    char *result_text = NULL;
    Status status;

    print_section_header(whole_word ? "REPLACE WORD" : "REPLACE PHRASE");

    if (read_text_line("Search text › ", search_pattern, sizeof(search_pattern)) <= 0) {
        return;
    }
    if (search_pattern[0] == '\0') {
        ui_msg_error("Search pattern cannot be empty.");
        ui_pause();
        return;
    }

    if (read_text_line("Replacement › ", replacement, sizeof(replacement)) <= 0) {
        return;
    }

    ask_result = ask_yes_no("Case-sensitive? (y/n) [n] › ", 0, &case_sensitive);
    if (ask_result <= 0) {
        return;
    }

    occurrences = count_occurrences(buffer->data, search_pattern, case_sensitive, whole_word);
    if (occurrences == 0U) {
        ui_msg_warning("No matching occurrences found. Text was not modified.");
        ui_pause();
        return;
    }

    {
        char count_msg[128];
        snprintf(count_msg, sizeof(count_msg),
                 "Original text contains %lu matching occurrence(s).",
                 (unsigned long)occurrences);
        ui_msg_info(count_msg);
    }

    ask_result = ask_yes_no("Confirm replacement? (y/n) [y] › ", 1, &confirmed);
    if (ask_result <= 0 || !confirmed) {
        ui_msg_info("Replacement cancelled. Text was not modified.");
        ui_pause();
        return;
    }

    status = replace_text(buffer->data, search_pattern, replacement,
                          case_sensitive, whole_word, &result_text,
                          &replacement_count);
    if (status == STATUS_SUCCESS) {
        char succ_msg[128];
        text_buffer_replace_data(buffer, result_text);
        snprintf(succ_msg, sizeof(succ_msg),
                 "%lu occurrence(s) replaced successfully.",
                 (unsigned long)replacement_count);
        ui_msg_success(succ_msg);
    } else if (status == STATUS_ERROR_MEMORY) {
        ui_msg_error("Memory allocation failed during text replacement.");
    } else {
        ui_msg_error("Unable to replace text.");
    }
    ui_pause();
}

static void search_and_replace(TextBuffer *buffer)
{
    int choice;
    int read_result;

    if (buffer == NULL || buffer->data == NULL || buffer->length == 0U) {
        ui_msg_warning("No text available for search or replace.");
        ui_pause();
        return;
    }

    for (;;) {
        display_search_menu();
        read_result = read_menu_choice(&choice);
        if (read_result == 0) {
            break;
        }
        if (read_result < 0) {
            ui_msg_error("Invalid choice. Please select an option from the menu.");
            continue;
        }

        switch (choice) {
            case 0:
                return;
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
                ui_msg_error("Invalid choice. Please select an option from the menu.");
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

    ui_init_terminal();
    display_banner();

    for (;;) {
        display_main_menu(buffer);
        read_result = read_menu_choice(&choice);
        if (read_result == 0) {
            printf("\n");
            ui_msg_info("Input stream closed. Exiting.");
            return STATUS_SUCCESS;
        }
        if (read_result < 0) {
            ui_msg_error("Invalid choice. Please select an option from the menu.");
            continue;
        }

        switch (choice) {
            case 1:
                enter_text(buffer);
                break;
            case 2:
                display_current_text(buffer);
                break;
            case 3:
                text_buffer_clear(buffer);
                current_source_info[0] = '\0';
                ui_msg_success("Current text cleared successfully.");
                ui_pause();
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
                printf("\n");
                ui_msg_info("Thank you for using Text Analyzer. Goodbye!");
                return STATUS_SUCCESS;
            default:
                ui_msg_error("Invalid choice. Please select an option from the menu.");
                break;
        }
    }
}
