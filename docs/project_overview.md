# Text Analyzer — Project Overview

## 1. Introduction
**Text Analyzer** is an advanced, console-based text analytics and inspection tool built entirely in standard C11. Developed as a Master of Computer Applications (MCA) final-year project, the software demonstrates modern, robust, and maintainable software engineering practices within systems programming in C.

The application allows users to ingest text through interactive multiline console input or external files, store it in an automatically managed dynamic text buffer, run analytical and statistical evaluations, perform frequency distributions, execute precision pattern search and replacements, and export detailed reports to disk.

---

## 2. Problem Statement
Processing and analyzing textual data is a foundational task in computing, natural language processing, digital forensics, and content management. While high-level languages provide extensive built-in libraries for string manipulation, building an efficient, leak-free, and modular text processor in C presents core systems engineering challenges:
- Managing dynamic memory safely without fixed-size buffer overflows or heap memory leaks.
- Ensuring atomic file I/O operations where unexpected errors do not corrupt existing application state.
- Accurately parsing lexical tokens (words, sentences, paragraphs) with strict mathematical precision.
- Providing an intuitive, developer-friendly terminal interface without relying on heavy third-party GUI frameworks or terminal libraries like `ncurses`.

Text Analyzer solves these challenges by combining clean modular encapsulation with defensive memory safety and native terminal formatting.

---

## 3. Project Objectives
1. **Modular Architecture**: Separate distinct responsibilities into self-contained modules (`text_utils`, `analyzer`, `frequency`, `search`, `file_handler`, `report`, `ui`).
2. **Defensive Memory Management**: Use dynamic memory allocation with arithmetic overflow protection before every resize operation, guaranteeing clean state management.
3. **Comprehensive Text Statistics**: Compute accurate lexical metrics (characters, words, sentences, paragraphs, reading time, averages).
4. **Frequency Modeling**: Provide normalized word frequency tables and non-whitespace character frequency maps.
5. **Pattern Search & In-Place Replacement**: Support whole-word and substring search/replace with case-sensitivity toggles.
6. **Data Persistence**: Safely load and save text files with failure-preservation semantics.
7. **Automated Report Generation**: Generate formatted, timestamped analysis reports in text format.
8. **Professional Console UX**: Design a clean, colored terminal dashboard utilizing native ANSI escape sequences compatible with both Windows and POSIX systems.

---

## 4. Scope
### In Scope
- Interactive multiline text input terminated by a blank line or EOF.
- In-memory dynamic string management with exponential growth.
- Character classifications (uppercase, lowercase, vowels, consonants, digits, spaces, tabs, special characters).
- Structural parsing (words, lines, sentences, paragraphs).
- Statistical metrics (averages, shortest/longest words, reading time estimation).
- Case-normalized word frequency with top-10 ranking and deterministic tie-breaking.
- Substring and whole-word search with occurrence counting.
- Full buffer text replacement with dynamic resizing.
- Disk operations: loading text files, saving text files, exporting timestamped reports into `reports/`.
- Cross-platform ANSI terminal coloring with `NO_COLOR` environment variable detection.
- Comprehensive unit, integration, and stress test suites.

### Out of Scope
- Multi-buffer concurrent editing (multiple active tabs).
- Binary or proprietary document parsing (e.g., PDF, DOCX).
- Multi-byte complex Unicode/UTF-8 tokenization (standard ASCII / basic Latin tokens are supported).
- Graphical user interface (GUI) or web interface.

---

## 5. Target Users
- **Students & Academics**: Analyzing lexical density, reading levels, and vocabulary metrics in essays and documents.
- **Software Developers & Systems Programmers**: Exploring clean, modular, memory-safe C11 architectural patterns.
- **Writers & Editors**: Measuring word count, character count, sentence length, and word repetition.

---

## 6. Key Features
- **Dynamic TextBuffer**: Expands automatically on demand; no artificial truncation limits.
- **Deep Statistical Analysis**: Instant calculation of 18 distinct metrics.
- **Dual Frequency Tables**: Complete character frequency map and sorted word occurrence tables.
- **Configurable Search & Replace**: Case-sensitive or insensitive; substring or whole-word matching.
- **Safe File Loading**: Preserves previously loaded text if a file open or read error occurs.
- **Formatted Report Exporting**: One-touch generation of formatted `.txt` reports with collision-resistant timestamps.
- **Developer-Grade Dashboard**: Box-drawn frames, live buffer status monitor, colored alert tags (`[SUCCESS]`, `[ERROR]`, `[WARNING]`, `[INFO]`).

---

## 7. Functional Requirements

| Requirement ID | Description |
| :--- | :--- |
| **FR-01: Text Ingestion** | Accept multiline console text input or load text from disk files. |
| **FR-02: State Monitoring** | Maintain live tracking of buffer status (`EMPTY BUFFER` vs `TEXT LOADED`), word count, and character count. |
| **FR-03: Statistical Analysis** | Compute characters, non-space characters, spaces, tabs, uppercase, lowercase, digits, vowels, consonants, special characters, words, lines, sentences, paragraphs, longest/shortest word, average word length, average sentence length, and estimated reading time. |
| **FR-04: Character Frequency** | Calculate frequency of all non-whitespace ASCII characters with case normalization for letters. |
| **FR-05: Word Frequency** | Tokenize alphabetic words, normalize to lowercase, count occurrences, and identify top-10 most frequent words. |
| **FR-06: Text Search** | Search for words or phrases with whole-word and case-sensitivity filters, displaying occurrence counts. |
| **FR-07: Text Replacement** | Replace matching search terms with replacement strings across the entire buffer, dynamically reallocating memory. |
| **FR-08: File I/O** | Save text buffer to a designated path and load text files into memory safely. |
| **FR-09: Report Export** | Write formatted analytical reports to `reports/analysis_report_<timestamp>.txt`. |
| **FR-10: Memory Deallocation** | Explicitly free all allocated heap memory when clearing buffer or terminating the application. |

---

## 8. Non-Functional Requirements
- **Performance**: Near-instantaneous analysis (<50ms for typical documents up to 50KB).
- **Portability**: Standard C11 conforming; builds with GCC on Windows (MinGW) and Linux.
- **Zero Third-Party Dependencies**: Pure standard library (`libc`); no external runtime dependencies.
- **Reliability & Memory Safety**: Arithmetic overflow checks prior to all `realloc` and `malloc` calls; guaranteed `fclose` on all file paths; zero crashes on empty or invalid input.
- **Usability**: Intuitive menu-driven navigation with colored prompts and explicit validation feedback.

---

## 9. Technologies Used
- **Language**: C11 standard (`-std=c11`)
- **Compiler**: GCC (`-Wall -Wextra -Wpedantic`)
- **Build System**: GNU Make / `mingw32-make`
- **Platform APIs**: Standard C library; Windows Console API (`ENABLE_VIRTUAL_TERMINAL_PROCESSING`, `CP_UTF8`) on Windows hosts.

---

## 10. Expected Outcome
A complete, reliable, and production-quality CLI software package accompanied by full test coverage, robust documentation, and an impressive user interface suitable for academic evaluation and real-world utility.

---

## 11. Limitations
1. **Word Definition**: Standardized as consecutive alphabetic characters (`isalpha`). Hyphenated compounds or alphanumeric codes are split into constituent tokens.
2. **Character Set**: Optimized for ASCII and basic Latin characters; multi-byte Unicode/UTF-8 grapheme clustering is not supported.
3. **Single Document Buffer**: Edits apply to a single active document at any given time.

---

## 12. Future Scope
1. Support for UTF-8 multi-byte character processing and internationalization.
2. Multiple buffer management (tabs / split panes).
3. Regular expression (regex) search and pattern matching.
4. Exporting reports to structured formats (JSON, CSV, HTML).
5. Visual ASCII/Unicode bar charts for frequency distributions.
