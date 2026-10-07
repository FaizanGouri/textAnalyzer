# Text Analyzer

A modular, console-based C11 application for analyzing text. It is an MCA
final-year project designed for a clean, maintainable, module-based codebase.

## Technology

- C11 and the standard C library
- GCC
- GNU Make (when available)

## Current status

Phase 4 is complete: the application supports multiline text input, dynamic
text storage, complete statistical analysis, and character/word frequency
analysis. Search, file operations, and generated reports are not yet
implemented.

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

- Search and replacement
- Search and replacement
- Text-file loading and saving
- Analysis reports
