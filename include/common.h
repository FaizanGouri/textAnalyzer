#ifndef TEXT_ANALYZER_COMMON_H
#define TEXT_ANALYZER_COMMON_H

/* Shared status values for future modules. */
typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR_FILE,
    STATUS_ERROR_MEMORY,
    STATUS_ERROR_INPUT,
    STATUS_INPUT_EOF,
    STATUS_ERROR_INVALID
} Status;

#endif
