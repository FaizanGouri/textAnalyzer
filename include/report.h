#ifndef TEXT_ANALYZER_REPORT_H
#define TEXT_ANALYZER_REPORT_H

#include <stddef.h>

#include "common.h"
#include "text_utils.h"

#define REPORT_DEFAULT_DIRECTORY "reports"
#define REPORT_DEFAULT_MAX_PATH 512U

Status generate_report(const TextBuffer *buffer, const char *source_info,
                       char *generated_path, size_t path_size);
Status generate_report_file(const TextBuffer *buffer, const char *filepath,
                            const char *source_info);

#endif
