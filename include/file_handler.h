#ifndef TEXT_ANALYZER_FILE_HANDLER_H
#define TEXT_ANALYZER_FILE_HANDLER_H

#include "text_utils.h"

Status load_text_file(const char *filename, TextBuffer *buffer);
Status save_text_file(const char *filename, const TextBuffer *buffer);

#endif
