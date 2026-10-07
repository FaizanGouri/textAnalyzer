# Text Analyzer — User Manual & Operating Guide

Welcome to the **Text Analyzer** User Manual! This guide explains how to build, run, and use every feature of Text Analyzer in simple, step-by-step instructions.

---

## Table of Contents
1. [System Requirements](#1-system-requirements)
2. [Building the Application](#2-building-the-application)
3. [Running the Application](#3-running-the-application)
4. [Understanding the Terminal Dashboard](#4-understanding-the-terminal-dashboard)
5. [Feature Walkthrough](#5-feature-walkthrough)
   - [Step 1: Entering Text](#step-1-entering-text)
   - [Step 2: Viewing Current Text](#step-2-viewing-current-text)
   - [Step 3: Analyzing Text](#step-3-analyzing-text)
   - [Step 4: Character & Word Frequency Analysis](#step-4-character--word-frequency-analysis)
   - [Step 5: Searching for Words and Phrases](#step-5-searching-for-words-and-phrases)
   - [Step 6: Replacing Words and Phrases](#step-6-replacing-words-and-phrases)
   - [Step 7: Saving Text to a File](#step-7-saving-text-to-a-file)
   - [Step 8: Loading Text from a File](#step-8-loading-text-from-a-file)
   - [Step 9: Generating an Analysis Report](#step-9-generating-an-analysis-report)
   - [Step 10: Clearing Text](#step-10-clearing-text)
   - [Step 11: Exiting the Application](#step-11-exiting-the-application)
6. [Error Handling & Edge Cases](#6-error-handling--edge-cases)
7. [Reports Directory Location](#7-reports-directory-location)

---

## 1. System Requirements
- **Operating System**: Windows (10/11 recommended), Linux, or macOS.
- **Compiler**: GCC with C11 support (`gcc`).
- **Terminal**: Any terminal with standard ANSI color support (Windows Terminal, PowerShell, CMD, Bash).
- **Libraries**: No extra packages or third-party libraries needed.

---

## 2. Building the Application

Open your terminal or command prompt in the project root directory and run:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude src/*.c -o build/text_analyzer.exe
```

Alternatively, if you have GNU Make installed:

```sh
mingw32-make    # On Windows
make            # On Linux / macOS
```

The compiled program will be created in the `build/` folder as `text_analyzer.exe` (or `text_analyzer` on Linux/macOS).

---

## 3. Running the Application

Launch the program from your terminal:

```sh
# On Windows:
.\build\text_analyzer.exe

# On Linux / macOS:
./build/text_analyzer
```

---

## 4. Understanding the Terminal Dashboard

When you launch Text Analyzer, you will see a clean, colored dashboard:

```text
╔══════════════════════════════════════════════════════╗
║                                                      ║
║                 TEXT ANALYZER                        ║
║                                                      ║
║            C11 • TEXT ANALYTICS TOOL                 ║
║                                                      ║
╚══════════════════════════════════════════════════════╝

────────────────────────────────────────────────────
STATUS: ● EMPTY BUFFER
WORDS: 0      CHARACTERS: 0
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

Select an option › 
```

### Dashboard Elements:
- **Top Header (Cyan)**: Identifies the tool and version.
- **Status Bar**: 
  - `● EMPTY BUFFER` (Yellow dot) indicates no text is currently loaded.
  - `● TEXT LOADED` (Green dot) indicates an active text document is in memory.
  - Displays dynamic **WORDS** and **CHARACTERS** counters in real time.
- **Menu Frame (Cyan / Blue)**: Lists available operations.
- **Prompt (` › `)**: Awaiting your input.

---

## 5. Feature Walkthrough

### Step 1: Entering Text
1. Type `1` at the menu prompt and press `Enter`.
2. Type or paste your multiline text.
3. To finish inputting text, press `Enter` on an empty line.
4. You will see `[SUCCESS] Text entered and stored in buffer successfully.`
5. Press `Enter` to return to the main menu. The status bar now displays `● TEXT LOADED` with updated word and character counts.

> **Tip**: If you already have text loaded, the application will ask if you wish to overwrite it before proceeding.

### Step 2: Viewing Current Text
1. Type `2` and press `Enter`.
2. The complete in-memory text will be displayed exactly as entered.
3. Press `Enter` to return to the menu.

### Step 3: Analyzing Text
1. Type `4` and press `Enter`.
2. Three structured cards will appear:
   - **General Statistics**: Total characters, characters without spaces, word count, line count, sentence count, and paragraph count.
   - **Character Statistics**: Uppercase letters, lowercase letters, digits, spaces, tabs, special characters, vowels, and consonants.
   - **Word Statistics**: Longest word, shortest word, average word length, average sentence length, and estimated reading time.
3. Press `Enter` to return to the main menu.

### Step 4: Character & Word Frequency Analysis
1. Type `5` and press `Enter` to open the Frequency submenu:
   - **Option [1] Character Frequency**: Shows a sorted table of how many times each character appears in the text, and highlights the single most frequent character.
   - **Option [2] Word Frequency**: Shows all unique words normalized to lowercase, their frequencies, the single most frequent word, and a highlighted **Top 10 Words** leaderboard.
   - **Option [0]**: Return to the main menu.

### Step 5: Searching for Words and Phrases
1. Type `6` and press `Enter` to open Search & Replace.
2. Select:
   - **[1] Search Word**: Searches for exact word boundaries (e.g., searching for "cat" will match "cat", but not "catch" or "bobcat").
   - **[2] Search Phrase**: Substring matching for any arbitrary word or phrase.
   - **[3] Count Occurrences**: Quickly counts total matching instances without full text scan printing.
3. Type your search term when prompted.
4. Choose whether the search should be case-sensitive (`y` or `n`).
5. The result will display the occurrence count.

### Step 6: Replacing Words and Phrases
1. From the Search & Replace submenu, select:
   - **[4] Replace Word**: Replaces whole-word occurrences only.
   - **[5] Replace Phrase**: Replaces substring occurrences anywhere in the text.
2. Enter the target word/phrase to find.
3. Enter the new replacement text (you can leave it blank to delete the word).
4. Select case-sensitivity (`y` or `n`).
5. The application replaces all occurrences across the document, tells you how many replacements were made, and immediately updates the active buffer.

### Step 7: Saving Text to a File
1. Type `8` and press `Enter`.
2. Enter the desired file path (e.g., `data/my_notes.txt` or `output.txt`).
3. The application saves the active text buffer to disk and displays `[SUCCESS] File saved successfully.`

### Step 8: Loading Text from a File
1. Type `7` and press `Enter`.
2. Type the path to an existing text file (e.g., `data/sample.txt`).
3. The file will be loaded into the buffer.
4. **Safety feature**: If the file does not exist, an error notification is shown, and your existing text in memory remains completely untouched.

### Step 9: Generating an Analysis Report
1. Type `9` and press `Enter`.
2. The application compiles an exhaustive statistical and frequency summary and writes it directly to disk.
3. A success notification shows the full file path where the report was saved, such as:
   ```text
   [SUCCESS] Analysis report successfully generated and saved to:
             reports/analysis_report_20261007_235012.txt
   ```

### Step 10: Clearing Text
1. Type `3` and press `Enter`.
2. The application prompts: `Are you sure you want to clear current text? (y/n) [n] › `
3. Type `y` to confirm.
4. The memory is freed, counters reset to 0, and the status bar updates to `● EMPTY BUFFER`.

### Step 11: Exiting the Application
1. Type `0` and press `Enter`.
2. A friendly farewell message is displayed, and the application closes cleanly.

---

## 6. Error Handling & Edge Cases

- **Invalid Menu Options**: If you type an invalid number (like `99`), a negative number (`-1`), or letters (`abc`), Text Analyzer displays `[ERROR] Invalid choice. Please select an option from the menu.` and lets you try again immediately.
- **Empty Buffer Actions**: If you try to analyze, search, or generate a report with no text loaded, a helpful warning `[WARNING] No text available...` is displayed without any crashes.
- **Closing the Terminal / EOF**: If you press `Ctrl+D` (Linux/macOS) or `Ctrl+Z` then `Enter` (Windows) to close input, the program gracefully prints `[INFO] Input stream closed. Exiting.` and terminates cleanly.

---

## 7. Reports Directory Location

All generated reports are stored inside the `reports/` folder in the project root:

```text
Text Analyzer/
└── reports/
    ├── analysis_report_20261007_235012.txt
    └── analysis_report_20261007_235544.txt
```

Each report contains:
- Report generation date & time
- Input source origin
- General & character metrics
- Lexical word statistics & reading time
- Top 10 most frequent words
- Complete frequency tables for characters and words
