# Text Analyzer — Test Cases & Quality Assurance Specification

## 1. Overview & Verification Summary
This document provides the formal test case matrix and test execution results for the **Text Analyzer** application. All test cases were executed and verified during Phase 9 Quality & Memory Safety testing and Phase 10 final verification.

### 1.1 Test Suite Summary
- **Total Test Suites**: 6 automated test binaries + 1 manual interactive smoke test.
- **Automated Test Targets**:
  1. `test_text_utils` (Dynamic buffer, byte operations, memory lifecycle)
  2. `test_analyzer` (General, character, and lexical statistics edge cases)
  3. `test_frequency` (Character frequency and dynamic word frequency tables)
  4. `test_search` (Pattern searching and text replacement engine)
  5. `test_file_handler` (Disk persistence and failure recovery)
  6. `test_report` (Report generation and file serialization)
- **Overall Status**: **100% PASS (0 Failures, 0 Regressions)**

### 1.2 AddressSanitizer (ASan) Status
> **AddressSanitizer Notice**:
> AddressSanitizer was attempted, but the installed MinGW GCC environment does not provide `libasan`, so ASan execution could not be completed.
> 
> Memory safety was verified using static code inspection, integer overflow guards before memory operations, explicit pointer validation, and automated stress testing suites (including 100+ buffer reallocation and cycling iterations).

### 1.3 Core Word Definition Finding
During testing, the canonical project definition of a word was reaffirmed:
$$\text{Word} = \text{A sequence of consecutive alphabetic characters satisfying } \texttt{isalpha(c)} \neq 0$$
Tokens containing numbers (e.g., `w1`, `v2.0`) are parsed such that alphabetic segments form words while digits and symbols act as word boundaries.

---

## 2. Test Case Matrix

### 2.1 TextBuffer Stress & Memory Safety Tests (`tests/test_text_utils.c`)

| Test ID | Category | Description | Input / Condition | Expected Result | Actual Result | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **TB-01** | TextBuffer | Initial buffer state | Call `text_buffer_init(&b)` | `data == NULL`, `length == 0`, `capacity == 0` | Verified | **PASS** |
| **TB-02** | TextBuffer | Small text append | Append `"Hello World"` | Length = 11, null-terminated, data matches | Verified | **PASS** |
| **TB-03** | TextBuffer | Buffer growth stress | 100 appends of 149-char blocks (14,970 chars total) | Dynamic capacity expansion without truncation or leaks | 14,970 chars intact | **PASS** |
| **TB-04** | TextBuffer | Replace large with small | Replace 10,000-char buffer with `"small"` | Previous buffer freed, length = 5, capacity = 6 | Verified | **PASS** |
| **TB-05** | TextBuffer | Replace small with large | Replace `"small"` with 5,000-char buffer | Reallocated correctly, length = 5,000 | Verified | **PASS** |
| **TB-06** | TextBuffer | Cycling stress test | 100 cycles of append followed by `clear()` | Clean heap allocation/deallocation on each iteration | Zero memory corruption | **PASS** |
| **TB-07** | TextBuffer | Null safety check | Append `NULL` pointer to buffer | Returns `STATUS_ERROR_INVALID` | Handled cleanly | **PASS** |
| **TB-08** | TextBuffer | Character helper checks | Test `text_is_word_character` and `text_to_lowercase` | Letters return 1, digits/symbols return 0 | Verified | **PASS** |

---

### 2.2 Text Statistics Edge Cases (`tests/test_analyzer.c`)

| Test ID | Category | Description | Input / Condition | Expected Result | Actual Result | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **ST-01** | Statistics | Basic statistics | `"Hello world"` | Chars: 11, Words: 2, Lines: 1, Sentences: 0, Paragraphs: 1 | As expected | **PASS** |
| **ST-02** | Statistics | Empty text | `""` | All counts = 0, avg word/sentence len = 0.0, reading time = 0.0 | As expected | **PASS** |
| **ST-03** | Statistics | Only spaces | `"     "` (5 spaces) | Spaces = 5, Words = 0, no division-by-zero | Handled safely | **PASS** |
| **ST-04** | Statistics | Only tabs | `"\t\t\t"` (3 tabs) | Tabs = 3, Words = 0 | As expected | **PASS** |
| **ST-05** | Statistics | Only newlines | `"\n\n\n"` (3 newlines) | Lines = 3, Paragraphs = 0, Words = 0 | As expected | **PASS** |
| **ST-06** | Statistics | Only digits | `"12345 67890"` | Digits = 10, Words = 0 | As expected | **PASS** |
| **ST-07** | Statistics | Only punctuation | `"!?,.:;"` | Special chars = 6, Words = 0 | As expected | **PASS** |
| **ST-08** | Statistics | Uppercase only | `"HELLO WORLD"` | Uppercase = 10, Lowercase = 0, Words = 2 | As expected | **PASS** |
| **ST-09** | Statistics | Lowercase only | `"hello world"` | Lowercase = 10, Uppercase = 0, Words = 2 | As expected | **PASS** |
| **ST-10** | Statistics | Sentences without spaces | `"Sentence one.Sentence two!Sentence three?"` | Sentences = 3, Words = 6 | As expected | **PASS** |
| **ST-11** | Statistics | Repeated punctuation | `"Hello!!! ... ???"` | Sentences = 3, Words = 1 | As expected | **PASS** |
| **ST-12** | Statistics | Extreme word length | Continuous 599-character word | Longest word length = 599, correctly allocated and freed | As expected | **PASS** |

---

### 2.3 Frequency Analysis Edge Cases (`tests/test_frequency.c`)

| Test ID | Category | Description | Input / Condition | Expected Result | Actual Result | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **FQ-01** | Frequency | Normal word frequency | `"apple apple banana"` | `apple` count = 2, `banana` count = 1 | As expected | **PASS** |
| **FQ-02** | Frequency | Case normalization | `"Apple APPLE apple"` | Normalized `apple` count = 3 | As expected | **PASS** |
| **FQ-03** | Frequency | Character frequency | `"aaabb!"` | `'a'` = 3, `'b'` = 2, `'!'` = 1, most frequent = `'a'` | As expected | **PASS** |
| **FQ-04** | Frequency | Empty text frequency | `""` | `get_most_frequent_character` returns `-1`, word table empty | Handled safely | **PASS** |
| **FQ-05** | Frequency | Whitespace only | `"   \t\n  \r\n "` | Returns most frequent char = `-1` | Handled safely | **PASS** |
| **FQ-06** | Frequency | Top-10 ranking | 11 unique words | Exactly 10 words selected in descending order | As expected | **PASS** |
| **FQ-07** | Frequency | Deterministic tie breaking | `"beta alpha beta alpha gamma"` | Ties broken deterministically by first encounter | As expected | **PASS** |
| **FQ-08** | Frequency | Dynamic table resize | 60 unique words (`"aa"`, `"ab"`, ... `"ch"`) | Table grows beyond initial capacity 16 to 60 elements | 60 words counted | **PASS** |
| **FQ-09** | Frequency | Digits excluded from words | `"12345 67890 999"` | Word table size = 0 | As expected | **PASS** |
| **FQ-10** | Frequency | Boundary tokens | `"[start] middle. (end)"` | 3 words extracted: `start`, `middle`, `end` | As expected | **PASS** |

---

### 2.4 Search & Replace Engine (`tests/test_search.c`)

| Test ID | Category | Description | Input / Condition | Expected Result | Actual Result | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **SR-01** | Search | Whole-word matching | `"cat catch bobcat"`, query `"cat"` (whole word) | Exactly 1 occurrence | As expected | **PASS** |
| **SR-02** | Search | Substring matching | `"cat catch bobcat"`, query `"cat"` (substring) | Exactly 3 occurrences | As expected | **PASS** |
| **SR-03** | Search | Case-insensitive search | `"Computer computer COMPUTER"`, query `"computer"` | Exactly 3 occurrences | As expected | **PASS** |
| **SR-04** | Search | Non-overlapping pattern | `"aaaa"`, query `"aa"` | Exactly 2 occurrences (not 3) | As expected | **PASS** |
| **SR-05** | Search | Pattern longer than text | `"short"`, query `"verylongpattern"` | 0 occurrences, returns safely | As expected | **PASS** |
| **SR-06** | Replace | Match entire text | `"exact"`, search `"exact"`, replace `"replaced"` | Replaces complete string with `"replaced"` | As expected | **PASS** |
| **SR-07** | Replace | Deletion (replace with `""`)| `"one, two, three"`, search `", "`, replace `""` | Result is `"onetwothree"` | As expected | **PASS** |
| **SR-08** | Replace | Multiple consecutive matches | `"ababab"`, search `"ab"`, replace `"z"` | Result is `"zzz"` | As expected | **PASS** |
| **SR-09** | Replace | Boundary replacements | `"alpha beta alpha"`, replace `"alpha"` with `"omega"` | Result is `"omega beta omega"` | As expected | **PASS** |
| **SR-10** | Replace | Special characters | `"var = val"`, replace `"="` with `"<===>"` | Result is `"var <===> val"` | As expected | **PASS** |
| **SR-11** | Replace | Invalid empty search term | Search pattern `""` | Returns `STATUS_ERROR_INPUT` | Handled cleanly | **PASS** |

---

### 2.5 File Handling Tests (`tests/test_file_handler.c`)

| Test ID | Category | Description | Input / Condition | Expected Result | Actual Result | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **FH-01** | FileHandler | Normal multiline load | Multiline `.txt` file | All lines loaded into buffer intact | Verified | **PASS** |
| **FH-02** | FileHandler | Empty file load | Zero-byte `.txt` file | Buffer length = 0, status success | Verified | **PASS** |
| **FH-03** | FileHandler | Large file load | 10,000-byte test file | Buffer length = 10,000, status success | Verified | **PASS** |
| **FH-04** | FileHandler | Missing file recovery | Load `no_such_file.txt` with existing text | Returns `STATUS_ERROR_FILE`, previous text preserved | Unchanged | **PASS** |
| **FH-05** | FileHandler | Save and round trip | Save multiline text, reload from disk | Loaded text identical to saved text | Exact match | **PASS** |
| **FH-06** | FileHandler | Save empty text | Save zero-length buffer to file | Created zero-byte file on disk | Verified | **PASS** |

---

### 2.6 Report Generation Tests (`tests/test_report.c`)

| Test ID | Category | Description | Input / Condition | Expected Result | Actual Result | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **RP-01** | Report | Normal report export | Text with words, sentences, punctuation | Creates formatted report containing all 6 sections | Verified | **PASS** |
| **RP-02** | Report | Empty text report | Buffer with 0 characters | All stats rendered as `0` or `N/A`, tables display `(None)` | Verified | **PASS** |
| **RP-03** | Report | Top-10 ranking in report | Text with 12 distinct words | Top-10 list sorted descending by frequency | Verified | **PASS** |
| **RP-04** | Report | Timestamped filename | `generate_report()` | Generates `reports/analysis_report_YYYYMMDD_HHMMSS.txt` | File created | **PASS** |
| **RP-05** | Report | Invalid output path | Directory path that does not exist | Returns `STATUS_ERROR_FILE` | Handled cleanly | **PASS** |
| **RP-06** | Report | Null parameter handling | Call with `NULL` buffer or filepath | Returns `STATUS_ERROR_INVALID` | Handled cleanly | **PASS** |

---

### 2.7 UI & Input Robustness Tests

| Test ID | Category | Description | Input / Condition | Expected Result | Actual Result | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **UI-01** | Interface | Invalid menu choices | Inputs: `-1`, `10`, `999` | Displays `[ERROR] Invalid choice`, stays in menu loop | Clean prompt | **PASS** |
| **UI-02** | Interface | Non-numeric menu input | Inputs: `"abc"`, `"!@#"` | Displays `[ERROR] Invalid choice`, stays in menu loop | Clean prompt | **PASS** |
| **UI-03** | Interface | Empty Enter on menu | Input: `""` (Enter key) | Re-prompts without errors or loop lockup | Clean prompt | **PASS** |
| **UI-04** | Interface | Actions on empty buffer | Options 2, 4, 5, 6, 8, 9 on empty buffer | Displays `[WARNING] No text available...` | Clean warning | **PASS** |
| **UI-05** | Interface | Search pattern not found | Search for nonexistent string | Displays `[WARNING] Pattern not found...` | Clean warning | **PASS** |
| **UI-06** | Interface | Load nonexistent file | Enter invalid file path | Displays `[ERROR] File could not be loaded...` | Text preserved | **PASS** |
| **UI-07** | Interface | Input stream EOF | Premature stream closure (`EOF`) | Displays `[INFO] Input stream closed. Exiting.` | Clean exit | **PASS** |
