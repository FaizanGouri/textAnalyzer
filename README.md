# Text Analyzer

A modular, console-based C11 application for analyzing text. It is an MCA
final-year project designed for a clean, maintainable, module-based codebase.

## Technology

- C11 and the standard C library
- GCC
- GNU Make (when available)

## Current status

Phase 8 is complete: the application features a modern, creative colored terminal
dashboard with structured ASCII/box styling, live buffer status monitoring, aligned
frequency tables, interactive search & replace, file management, report exporting,
and comprehensive error handling.

## Build

```sh
make
```

## Run

```sh
make run
```

## User Interface & Terminal Experience

- **Visual Dashboard:** Clean box borders, startup banner, and structured submenus.
- **Live Status Area:** Displays current buffer status (`● TEXT LOADED` / `● EMPTY BUFFER`), word count, and character count.
- **Consistent Prompts:** Unified prompt style (` › `) across all menu selections and inputs.
- **Standardized Notifications:** Color-coded status tags (`[SUCCESS]`, `[ERROR]`, `[WARNING]`, `[INFO]`).
- **ANSI Color Theme:**
  - **Cyan:** Main headers, titles, and primary UI borders.
  - **Blue:** Section separators, secondary information, and submenu frames.
  - **Green:** Success notifications and loaded status.
  - **Yellow:** Warnings, menu option numbers, and attention notices.
  - **Red:** Error alerts and failed operations.
- **Cross-Platform Compatibility:**
  - Native Windows Virtual Terminal Processing and UTF-8 console code page enablement.
  - Zero external library dependencies (no ncurses).
  - Automatic detection: disables colors when `NO_COLOR` environment variable is set.

## Project layout

- `include/` — public module headers
- `src/` — C source files
- `tests/` — module tests
- `data/` — sample and input text files
- `reports/` — generated analysis reports
- `docs/` — project documentation
- `build/` — generated executable and test binaries

The root LaTeX architecture specification is the authoritative design document.

## File workflow

Use menu option 7 to load a text file and option 8 to save the current text.
Failed loads leave the current text unchanged. File-open, read, write, close,
and memory-allocation failures are reported to the user.

## Report Generation

Use menu option 9 to generate a structured analysis report from the current
text buffer.

- **Location:** Reports are stored in the `reports/` directory.
- **Filename Format:** `analysis_report_YYYYMMDD_HHMMSS.txt` (with collision-avoidance suffixes such as `_1.txt` when necessary).
- **Report Contents:**
  - **Metadata:** Generation date/time and input source origin.
  - **General Statistics:** Total characters, characters excluding whitespace, words, lines, sentences, and paragraphs.
  - **Character Statistics:** Uppercase letters, lowercase letters, digits, spaces, tabs, special characters, vowels, and consonants.
  - **Word Statistics:** Longest word, shortest word, average word length, average sentence length, and estimated reading time.
  - **Character Frequency:** Most frequent character and complete non-whitespace frequency table.
  - **Word Frequency:** Most frequent word, Top 10 words ranked by occurrence, and full normalized word frequency table.
