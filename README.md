# Text Analyzer

A high-performance, modular, console-based text analytics and processing dashboard written in C11. Developed as an MCA Final-Year Project, Text Analyzer demonstrates software engineering best practices in C: modular encapsulation, dynamic memory safety, robust file handling, interactive search & replace, statistical modeling, frequency analysis, formatted report generation, and an aesthetically polished terminal user interface with native ANSI color styling.

---

## Table of Contents

- [Project Overview](#project-overview)
- [Project Objectives](#project-objectives)
- [Key Features](#key-features)
- [Technology Stack](#technology-stack)
- [System Requirements](#system-requirements)
- [Project Structure](#project-structure)
- [Build Instructions](#build-instructions)
- [Testing Instructions](#testing-instructions)
- [Execution Instructions](#execution-instructions)
- [Feature & Menu Overview](#feature--menu-overview)
  - [1. Text Input & In-Memory Buffer](#1-text-input--in-memory-buffer)
  - [2. Comprehensive Text Statistics](#2-comprehensive-text-statistics)
  - [3. Frequency Analysis](#3-frequency-analysis)
  - [4. Search & Replace Engine](#4-search--replace-engine)
  - [5. File Handling & Persistence](#5-file-handling--persistence)
  - [6. Analysis Report Generation](#6-analysis-report-generation)
  - [7. Terminal UI & Color System](#7-terminal-ui--color-system)
  - [8. Error Handling & Robustness](#8-error-handling--robustness)
- [Testing & Memory Safety](#testing--memory-safety)
- [Project Limitations](#project-limitations)
- [Future Enhancements](#future-enhancements)
- [Author & Academic Details](#author--academic-details)
- [License](#license)

---

## Project Overview

Text Analyzer provides an in-memory analytics engine capable of ingesting raw text from interactive multiline console input or from external text files. Once loaded into a managed dynamic text buffer (`TextBuffer`), the application extracts detailed character-level, word-level, line-level, sentence-level, and paragraph-level statistics, generates normalized word and character frequency distributions, performs substring/whole-word pattern searches and replacements, and exports comprehensive, timestamped analysis reports to disk.

---

## Project Objectives

1. **Modular Architecture**: Implement strict separation of concerns across dedicated C modules (`analyzer`, `frequency`, `search`, `file_handler`, `report`, `text_utils`, and `ui`).
2. **Memory Safety & Efficiency**: Enforce dynamic buffer management without fixed-length string truncation, checking every allocation for integer overflow and allocation failure.
3. **Robust Input & Error Handling**: Guarantee zero crashes on invalid user inputs, malformed files, empty buffers, extreme edge cases, and unexpected EOF terminations.
4. **Professional Developer Experience**: Provide a developer-grade terminal dashboard featuring clean Unicode/ASCII box styling, live buffer status tracking, color-coded alerts, and non-blocking menu flows.
5. **Zero External Dependencies**: Rely exclusively on standard C11 and the standard C library (`libc`), ensuring complete portability across Windows and POSIX operating systems without requiring libraries like `ncurses`.

---

## Key Features

- **Interactive Multiline Input**: Input arbitrary text directly from the console, finished with a double Enter or single blank line.
- **Dynamic In-Memory Buffer**: Exponential capacity expansion with heap reallocation, automatic null termination, and memory safety guards.
- **Deep Linguistic Statistics**:
  - Total characters, characters excluding whitespace, spaces, and tabs.
  - Letter classification: uppercase letters, lowercase letters, vowels, consonants, and digits.
  - Structural classification: words, lines, sentences, and paragraphs.
  - Lexical metrics: longest word, shortest word, average word length, average sentence length, and estimated reading time (based on 200 words/minute).
- **Dual Frequency Analysis**:
  - Full ASCII character frequency distribution (case-insensitive for letters, excluding whitespace).
  - Dynamic word frequency table with case normalization, top-10 ranking, and deterministic tie handling.
- **Search & Replace Engine**:
  - Substring matching or whole-word boundary matching (`has_whole_word_boundaries`).
  - Case-sensitive or case-insensitive search.
  - Exact match counting with non-overlapping pattern advances.
  - In-place text replacement allocating exact destination sizing with multiplication overflow validation.
- **Reliable File Handling**:
  - Load normal, empty, or multiline text files.
  - Save current buffer to any specified filepath.
  - Failure preservation guarantee: if a file cannot be loaded, existing buffer contents remain untouched.
- **Automated Report Exporting**:
  - Exports complete reports into the `reports/` directory with unique timestamped filenames (`analysis_report_YYYYMMDD_HHMMSS.txt`).
  - Automatic collision resolution with numeric suffixes (`_1.txt`, `_2.txt`).
- **Terminal User Interface**:
  - ANSI-colored dashboard with cyan headers, blue sections, green success notices, yellow warnings, and red error badges.
  - Native Windows 10/11 Virtual Terminal Processing and UTF-8 console enablement.
  - Fallback support: automatically disables ANSI sequences when `NO_COLOR` is present in the environment.

---

## Technology Stack

- **Language**: C11 (`-std=c11`)
- **Compiler**: GCC (tested with GCC 6.3.0+ / MinGW-w64 / Linux GCC)
- **Standard Library**: Standard C Library (`stdio.h`, `stdlib.h`, `string.h`, `ctype.h`, `stdint.h`, `time.h`)
- **Build System**: GNU Make / `mingw32-make`
- **Platform**: Cross-platform (Windows / Linux / macOS)
- **External Dependencies**: **None** (zero third-party dependencies)

---

## System Requirements

- **Operating System**: Windows 7/10/11, Linux (Ubuntu, Debian, Fedora, Arch), or macOS.
- **Compiler**: GCC with C11 support (`gcc`).
- **Build Tool** (Optional): `mingw32-make` or GNU `make`.
- **Terminal**: Modern terminal with ANSI escape code support (Windows Terminal, PowerShell 5.1+, Command Prompt with VT, GNOME Terminal, Bash).

---

## Project Structure

```text
Text Analyzer/
├── include/                     # Public module header files
│   ├── analyzer.h               # Text statistics & lexical metrics
│   ├── common.h                 # Shared Status enum & common definitions
│   ├── file_handler.h           # File I/O operations & persistence
│   ├── frequency.h              # Character & word frequency analysis
│   ├── report.h                 # Formatted analysis report generation
│   ├── search.h                 # Search and replace engine
│   ├── text_utils.h             # Dynamic TextBuffer & string utilities
│   └── ui.h                     # Terminal UI, colors, & menu system
├── src/                         # Module implementations
│   ├── analyzer.c
│   ├── file_handler.c
│   ├── frequency.c
│   ├── main.c                   # Application entry point
│   ├── report.c
│   ├── search.c
│   ├── text_utils.c
│   └── ui.c
├── tests/                       # Unit and regression test suites
│   ├── test_analyzer.c          # Statistics & edge-case tests
│   ├── test_file_handler.c      # File persistence & safety tests
│   ├── test_frequency.c         # Frequency & table resize tests
│   ├── test_report.c            # Report formatting & export tests
│   ├── test_search.c            # Pattern matching & replacement tests
│   └── test_text_utils.c        # Dynamic buffer stress tests
├── data/                        # Sample and test data
│   ├── input.txt
│   └── sample.txt
├── reports/                     # Output directory for exported reports
│   └── .gitkeep
├── docs/                        # Project documentation
│   ├── architecture.md          # Detailed architecture & module data flow
│   ├── project_overview.md      # Problem statement, requirements, & scope
│   ├── test_cases.md            # Comprehensive test matrix & edge-case log
│   └── user_manual.md           # Step-by-step user guide & operation manual
├── build/                       # Compilation output directory
│   └── .gitkeep
├── Makefile                     # Build & test automation script
├── README.md                    # Project documentation
├── .gitignore                   # Git artifact exclusion rules
├── LICENSE                      # MIT Open Source License
└── text analyzer project structure.tex  # Authoritative LaTeX architecture spec
```

---

## Build Instructions

### Method 1: Using GCC directly (Recommended)

From the project root directory, run:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude src/*.c -o build/text_analyzer.exe
```

*Flags explained:*
- `-std=c11`: Enforces the ISO C11 standard.
- `-Wall -Wextra -Wpedantic`: Enables strict compiler warning diagnostics.
- `-Iinclude`: Includes header files from the `include/` directory.
- `src/*.c`: Compiles all project source files.
- `-o build/text_analyzer.exe`: Emits the binary into `build/`.

### Method 2: Using Make (`mingw32-make` or `make`)

```sh
# On Windows (MinGW):
mingw32-make

# On Linux / macOS:
make
```

---

## Testing Instructions

To run the complete automated test suite across all 6 test targets:

```sh
# On Windows (MinGW):
mingw32-make test

# On Linux / macOS:
make test
```

### Manual Individual Test Compilation

If Make is unavailable, test executables can be built and run individually:

```sh
# 1. Text Buffer Stress Tests
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude tests/test_text_utils.c src/text_utils.c -o build/test_text_utils.exe
./build/test_text_utils.exe

# 2. Statistics & Edge Case Tests
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude tests/test_analyzer.c src/analyzer.c src/text_utils.c -o build/test_analyzer.exe
./build/test_analyzer.exe

# 3. Frequency Analysis Tests
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude tests/test_frequency.c src/frequency.c src/text_utils.c -o build/test_frequency.exe
./build/test_frequency.exe

# 4. Search & Replace Tests
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude tests/test_search.c src/search.c src/text_utils.c -o build/test_search.exe
./build/test_search.exe

# 5. File Handler Tests
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude tests/test_file_handler.c src/file_handler.c src/text_utils.c -o build/test_file_handler.exe
./build/test_file_handler.exe

# 6. Report Generation Tests
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude tests/test_report.c src/report.c src/analyzer.c src/frequency.c src/text_utils.c -o build/test_report.exe
./build/test_report.exe
```

---

## Execution Instructions

To launch the interactive application:

```sh
# Direct execution (Windows):
.\build\text_analyzer.exe

# Direct execution (Linux / macOS):
./build/text_analyzer

# Or using Make:
mingw32-make run
```

---

## Feature & Menu Overview

Upon launch, the user is greeted by the dashboard header, live buffer status monitor, and the main menu:

```text
╔══════════════════════════════════════════════════════╗
║                                                      ║
║                 TEXT ANALYZER                        ║
║                                                      ║
║            C11 • TEXT ANALYTICS TOOL                 ║
║                                                      ║
╚══════════════════════════════════════════════════════╝

────────────────────────────────────────────────────
STATUS: ● TEXT LOADED
WORDS: 9      CHARACTERS: 44
────────────────────────────────────────────────────
┌──────────────────── MAIN MENU ────────────────────┐
│                                                   │
│  [1]  Enter Text                                 │
│  [2]  View Current Text                          │
│  [3]  Clear Text                                 │
│  [4]  Analyze Text                               │
│  [5]  Frequency Analysis                         │
│  [6]  Search & Replace                           │
│  [7]  Load Text File                             │
│  [8]  Save Text File                             │
│  [9]  Generate Analysis Report                   │
│                                                   │
│  [0]  Exit                                       │
│                                                   │
└───────────────────────────────────────────────────┘
```

### 1. Text Input & In-Memory Buffer
- **Option [1] Enter Text**: Prompts for arbitrary multiline text. Type or paste content, and press `ENTER` on an empty line to finish. Memory is allocated dynamically; if existing text is present, a prompt asks whether to overwrite or cancel.
- **Option [2] View Current Text**: Displays the full in-memory buffer with exact line breaks.
- **Option [3] Clear Text**: Safely frees allocated buffer memory, resets counts to 0, and updates buffer state to `EMPTY BUFFER`.

### 2. Comprehensive Text Statistics
- **Option [4] Analyze Text**: Performs full statistical analysis and renders three organized cards:
  - **General Statistics**: Total characters, characters excluding whitespace, words, lines, sentences, and paragraphs.
  - **Character Statistics**: Uppercase letters, lowercase letters, digits, spaces, tabs, special characters, vowels, and consonants.
  - **Word Statistics**: Longest word, shortest word, average word length, average sentence length, and estimated reading time.

### 3. Frequency Analysis
- **Option [5] Frequency Analysis**: Opens a dedicated submenu:
  - **[1] Character Frequency**: Displays tabular character occurrences for all non-whitespace characters present in the text, alongside the overall most frequent character.
  - **[2] Word Frequency**: Displays all unique words normalized to lowercase with their exact occurrence counts, the most frequent word, and the top-10 most frequent words.

### 4. Search & Replace Engine
- **Option [6] Search & Replace**: Submenu offering:
  - **[1] Search Word**: Searches for a whole word using strict boundary rules.
  - **[2] Search Phrase**: Substring pattern matching for multi-word phrases.
  - **[3] Count Occurrences**: Fast occurrence counting without full text scan printing.
  - **[4] Replace Word**: Replaces whole-word occurrences with replacement text.
  - **[5] Replace Phrase**: Substring replacement across the entire buffer.
  - *Configuration*: Options to toggle case sensitivity (`y/n`). Displays total replacements performed and automatically updates the active text buffer.

### 5. File Handling & Persistence
- **Option [7] Load Text File**: Loads text from a file into the buffer.
  - *Safety Guarantee*: If the specified file does not exist or fails to open, existing buffer text remains completely intact.
- **Option [8] Save Text File**: Writes the current text buffer to a user-specified path on disk.

### 6. Analysis Report Generation
- **Option [9] Generate Analysis Report**: Compiles a comprehensive analytical summary into a standalone `.txt` report file inside `reports/`:
  - Automatically names files using timestamps: `reports/analysis_report_YYYYMMDD_HHMMSS.txt`.
  - Includes metadata (timestamp, origin source), general counts, character distribution, lexical metrics, top-10 words, and full frequency tables.

### 7. Terminal UI & Color System
- Centralized ANSI color macros defined in `include/ui.h` (`UI_COLOR_CYAN`, `UI_COLOR_BLUE`, `UI_COLOR_GREEN`, `UI_COLOR_YELLOW`, `UI_COLOR_RED`, `UI_COLOR_RESET`).
- Automatic detection of `NO_COLOR` environment variable (respects the standard from https://no-color.org).
- Fully compatible with Windows console host and modern terminal emulators.

### 8. Error Handling & Robustness
- **Invalid Menu Selections**: Non-numeric inputs, negative numbers, out-of-range choices, and empty Enters are handled gracefully without application crashes.
- **Buffer Safety**: Operations on empty buffers (Analyze, Frequency, Search, Report) produce clean `[WARNING]` notices rather than crashing.
- **Stream Termination**: Handles premature `EOF` (`Ctrl+D` / `Ctrl+Z`) cleanly by closing the stream and exiting safely.

---

## Testing & Memory Safety

### Definition of a Word
To maintain mathematical and architectural consistency across the application, the project strictly defines a **word** as:
$$\text{Word} = \text{One or more consecutive alphabetic characters satisfying } \texttt{isalpha(c)} \neq 0$$
Digits, punctuation, whitespace, and symbols act as token delimiters.

### Memory Safety & Audit
- All buffer growth in `text_utils.c` verifies against `SIZE_MAX` integer overflow before calling `realloc()`.
- String replacements in `search.c` calculate exact output buffer sizing with multiplication overflow checks before `malloc()`.
- Dynamic structures (`TextStatistics`, `WordFrequencyTable`) have paired initializer and deallocator functions (`free_statistics()`, `free_word_frequency_table()`).
- All file operations in `file_handler.c` and `report.c` guarantee `fclose()` on every execution path.

### AddressSanitizer Note
AddressSanitizer (`-fsanitize=address`) was tested during verification; however, the host MinGW 32-bit GCC 6.3.0 environment does not bundle `libasan`. Memory safety was verified through strict static code review, arithmetic overflow guards, and comprehensive stress testing (including 100+ buffer cycling tests and large 14,000+ character allocations).

---

## Project Limitations

- **ASCII / Basic Latin Focus**: Word detection and character classifications rely on standard C `<ctype.h>` functions (`isalpha`, `tolower`), optimized for standard ASCII / extended ASCII text rather than multi-byte UTF-8 tokenization.
- **Single Active Buffer**: The application manages one active text buffer in memory at a time.
- **File Encoding**: Best suited for standard UTF-8 or ASCII plain-text files without proprietary binary formatting (e.g., `.docx`, `.pdf`).

---

## Future Enhancements

- Multi-byte UTF-8 Unicode grapheme clustering and multi-language word boundary tokenization.
- Support for multiple concurrent buffer tabs / workspace documents.
- Regular expression (regex) search and replacement.
- Visual charts and frequency bar graphs rendered directly in the terminal using ASCII/Unicode blocks.
- Export formats supporting JSON, CSV, and HTML.

---

## Author & Academic Details

- **Project**: Text Analyzer (Console-based Modular C11 Application)
- **Course**: Master of Computer Applications (MCA)
- **Repository**: [https://github.com/FaizanGouri/textAnalyzer](https://github.com/FaizanGouri/textAnalyzer)
- **Author**: Faizan Gouri

---

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
