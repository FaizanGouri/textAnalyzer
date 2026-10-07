# Text Analyzer — Architecture Specification

## 1. System Architecture Overview

Text Analyzer is designed following a layered, modular architecture. The application enforces a strict separation of concerns between user interaction, in-memory data management, computational analytics, persistent storage, and report generation.

```text
                        +----------------------------+
                        |         Terminal UI        |
                        |      (src/ui.c, ui.h)      |
                        +--------------+-------------+
                                       |
                                       v
                        +----------------------------+
                        |        TextBuffer          |
                        | (src/text_utils.c, .h)     |
                        +--------------+-------------+
                                       |
         +-----------------------------+-----------------------------+
         |                             |                             |
         v                             v                             v
+------------------+          +------------------+          +------------------+
|     Analyzer     |          |    Frequency     |          |      Search      |
| (analyzer.c, .h) |          | (frequency.c, .h)|          |  (search.c, .h)  |
+--------+---------+          +--------+---------+          +--------+---------+
         |                             |                             |
         +-----------------------------+-----------------------------+
                                       |
                                       v
                        +----------------------------+
                        |        File Handler        |
                        | (file_handler.c, report.c) |
                        +--------------+-------------+
                                       |
                                       v
                         +--------------------------+
                         |      Storage / Disk      |
                         |  (data/ & reports/*.txt) |
                         +--------------------------+
```

---

## 2. Directory Structure & Module Responsibilities

### 2.1 Header Directory (`include/`)
Contains public interface contracts and type definitions:
- [`common.h`](file:///d:/mca/c/Text%20Analyzer/include/common.h): Central `Status` enumeration (`STATUS_SUCCESS`, `STATUS_ERROR_INVALID`, `STATUS_ERROR_MEMORY`, `STATUS_ERROR_FILE`, `STATUS_ERROR_INPUT`, `STATUS_INPUT_EOF`).
- [`text_utils.h`](file:///d:/mca/c/Text%20Analyzer/include/text_utils.h): Definition of `TextBuffer`, capacity allocation, byte-level appending, data replacement, and character classification utilities.
- [`analyzer.h`](file:///d:/mca/c/Text%20Analyzer/include/analyzer.h): Structure definition of `TextStatistics` and prototype for `analyze_text()`.
- [`frequency.h`](file:///d:/mca/c/Text%20Analyzer/include/frequency.h): Character frequency array sizing, `WordFrequencyTable`, and frequency computation prototypes.
- [`search.h`](file:///d:/mca/c/Text%20Analyzer/include/search.h): Prototypes for pattern searching, occurrence counting, and buffer replacement.
- [`file_handler.h`](file:///d:/mca/c/Text%20Analyzer/include/file_handler.h): Interface for loading text files into `TextBuffer` and writing `TextBuffer` to disk.
- [`report.h`](file:///d:/mca/c/Text%20Analyzer/include/report.h): Report generation engine prototypes and path resolution constants.
- [`ui.h`](file:///d:/mca/c/Text%20Analyzer/include/ui.h): Console dashboard lifecycle (`ui_run`), ANSI color definitions, and message display helpers.

### 2.2 Source Directory (`src/`)
Implements the interfaces declared in `include/`:
- `main.c`: Application entry point; initializes console environment and delegates control to `ui_run()`.
- `ui.c`: Interactive menu loop, prompt handling, screen formatting, color output, and input validation.
- `text_utils.c`: In-memory dynamic string management, exponential buffer resizing, memory cleanup.
- `analyzer.c`: Single-pass lexical scanner for character, word, sentence, and paragraph metrics.
- `frequency.c`: Character occurrence tabulation and dynamic word frequency sorting/ranking.
- `search.c`: Substring and whole-word pattern scanning, match counting, and exact-fit replacement allocation.
- `file_handler.c`: Safe file reading with rollback protection and file writing.
- `report.c`: Formats and serializes analytical findings into structured disk reports.

### 2.3 Tests Directory (`tests/`)
Isolated regression and unit test harnesses:
- `test_text_utils.c`: Buffer stress tests, growth validation, memory cycle safety.
- `test_analyzer.c`: Edge-case input parsing (empty text, whitespace only, long words).
- `test_frequency.c`: Word grouping, case normalization, dynamic table reallocation.
- `test_search.c`: Boundary matching, non-overlapping occurrences, empty replacement.
- `test_file_handler.c`: File load/save round trips, missing file handling.
- `test_report.c`: Report generation, timestamped export, error conditions.

### 2.4 Supporting Directories
- `data/`: Contains sample reference text files (`sample.txt`, `input.txt`).
- `reports/`: Target folder for all runtime-generated analysis reports (`reports/.gitkeep` preserved).
- `build/`: Target destination for compiled binaries (`text_analyzer.exe` and test executables).
- `docs/`: Technical and end-user documentation.

---

## 3. Data Design & TextBuffer Strategy

### 3.1 The `TextBuffer` Structure
The core data structure for storing mutable text is `TextBuffer`, defined in [`text_utils.h`](file:///d:/mca/c/Text%20Analyzer/include/text_utils.h):

```c
typedef struct {
    char *data;       /* Pointer to heap-allocated, null-terminated string */
    size_t length;    /* Current string length in bytes (excluding '\0')   */
    size_t capacity;  /* Total allocated capacity in bytes including '\0'  */
} TextBuffer;
```

### 3.2 Dynamic Memory Allocation Strategy
1. **Initial State**: `text_buffer_init()` sets `data = NULL`, `length = 0`, `capacity = 0`.
2. **First Allocation**: The initial allocation assigns a base capacity (e.g., 128 bytes).
3. **Exponential Growth**: When appended data exceeds the remaining capacity, capacity doubles repeatedly (`capacity *= 2`) until it accommodates the required length:
   ```text
   Capacity Progression: 128 -> 256 -> 512 -> 1024 -> 2048 -> ...
   ```
4. **Arithmetic Overflow Guard**: Before every allocation, size computations check for `SIZE_MAX` overflow:
   ```c
   if (length > SIZE_MAX - buffer->length - 1U) {
       return STATUS_ERROR_MEMORY;
   }
   ```
5. **Reallocation Safety**: A temporary pointer is used for `realloc()`:
   ```c
   char *new_data = realloc(buffer->data, new_capacity);
   if (new_data == NULL) {
       return STATUS_ERROR_MEMORY; /* Original buffer->data remains untouched */
   }
   buffer->data = new_data;
   ```
6. **Deallocation**: `text_buffer_clear()` and `text_buffer_free()` release heap memory via `free()`, resetting all members to zero.

---

## 4. Module Interaction & Data Flow

### 4.1 Statistical Analysis Flow
When the user requests text analysis (Menu Option 4):
```text
UI (enter option 4)
  │
  ├─► analyze_text(buffer->data, &statistics)
  │     │
  │     ├─► text_statistics_init(&statistics)
  │     ├─► Single-pass character loop:
  │     │     ├─ Classify character (casing, digits, vowels, consonants, spaces, tabs)
  │     │     ├─ Track sentence terminators ('.', '!', '?')
  │     │     ├─ Track paragraph breaks ('\n' followed by non-space)
  │     │     └─ Tokenize words (consecutive isalpha characters)
  │     │          └─ Record longest & shortest word
  │     └─► Compute averages (avg word length, avg sentence length, reading time)
  │
  └─► UI formats and renders 3 statistics cards in console
        │
        └─► free_statistics(&statistics)
```

### 4.2 Word Definition & Tokenization
The project enforces a single authoritative definition of a word:
$$\text{Word} = \text{Sequence of consecutive characters where } \texttt{text\_is\_word\_character(c)} \neq 0 \ (\texttt{isalpha(c)})$$
Digits, punctuation, and whitespace terminate word tokens. This definition is universally applied across `analyzer.c`, `frequency.c`, `search.c`, and `report.c`.

### 4.3 Search and Replace Flow
When replacing text (Menu Option 6 -> 4 or 5):
```text
UI (Search query, replacement text, options)
  │
  ├─► count_occurrences(text, search, case_sensitive, whole_word)
  │     └─► Scans with non-overlapping position advances
  │
  ├─► If matches found:
  │     ├─ Calculate exact result length (with multiplication overflow checks)
  │     ├─ malloc(result_length + 1)
  │     ├─ Copy unmodified prefixes and replacement strings
  │     └─ Return new result string
  │
  └─► text_buffer_replace_data(buffer, result)
        ├─ Frees old buffer->data
        ├─ Assigns newly allocated replacement buffer
        └─ Updates buffer->length and buffer->capacity
```

### 4.4 File Handling Flow
```text
User selects Load File
  │
  ├─► load_text_file(path, buffer)
  │     ├─ fopen(path, "rb")
  │     ├─ Check file length via fseek / ftell
  │     ├─ Allocate temporary read buffer
  │     ├─ fread into temporary buffer
  │     ├─ fclose(file)
  │     ├─ On success: text_buffer_replace_data(buffer, temp_data)
  │     └─ On error: free(temp_data), existing buffer unchanged (Rollback safety)
  │
  └─► UI notifies success or reports error message
```

### 4.5 Report Generation Flow
```text
User selects Generate Report
  │
  ├─► generate_report(buffer, source_info, generated_path, max_path)
  │     ├─ Ensure 'reports/' directory exists
  │     ├─ Format timestamp: reports/analysis_report_YYYYMMDD_HHMMSS.txt
  │     ├─ Resolve collision (append _1, _2 if file exists)
  │     └─ generate_report_file(buffer, resolved_path, source_info)
  │          ├─ analyze_text()
  │          ├─ calculate_character_frequency()
  │          ├─ build_word_frequency()
  │          ├─ Format sections and tables into report file
  │          ├─ fclose(report_file)
  │          ├─ free_statistics()
  │          └─ free_word_frequency_table()
  │
  └─► UI displays generated report path
```

---

## 5. Error Handling Architecture

The application adopts a defensive error handling model:
1. **Enumerated Status Codes**: All operations that can fail return a `Status` code defined in `common.h`.
2. **Pointer Validation**: Every public function checks pointer parameters for `NULL` before dereferencing.
3. **No Uncaught Panics**: No functions invoke `abort()` or `exit()` on invalid input; errors are propagated back to the UI.
4. **Safe UI Recovery**: Invalid menu inputs, non-numeric values, or oversized inputs do not trigger infinite loops or buffer overflows.
5. **Stream Health**: If `stdin` is closed (EOF detected via `Ctrl+D` or `Ctrl+Z`), the application detects `STATUS_INPUT_EOF` and exits cleanly.
