# Text Analyzer

A modular, console-based C11 application for analyzing text. It is an MCA
final-year project designed for a clean, maintainable, module-based codebase.

## Technology

- C11 and the standard C library
- GCC
- GNU Make (when available)

## Current status

Phase 6 is complete: the application supports multiline text input, dynamic
text storage, statistical and frequency analysis, search and replacement, and
loading/saving plain-text files. Generated reports are not yet implemented.

## Build

```sh
make
```

## Run

```sh
make run
```

## Project layout

- `include/` — public module headers
- `src/` — C source files
- `tests/` — module tests
- `data/` — sample and input text files
- `reports/` — generated reports
- `docs/` — project documentation
- `build/` — generated executable

The root LaTeX architecture specification is the authoritative design document.

## Planned major features

- Word and phrase search
- Occurrence counting with case-sensitive or case-insensitive matching
- Whole-word and substring matching
- Confirmed text replacement
- Load complete `.txt` files into the current text buffer
- Save current text to a `.txt` file, including empty text

## File workflow

Use menu option 7 to load a text file and option 8 to save the current text.
Failed loads leave the current text unchanged. File-open, read, write, close,
and memory-allocation failures are reported to the user.
- Search and replacement
- Text-file loading and saving
- Analysis reports
