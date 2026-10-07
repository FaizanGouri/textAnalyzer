#ifndef TEXT_ANALYZER_UI_H
#define TEXT_ANALYZER_UI_H

#include "common.h"
#include "text_utils.h"

void ui_set_color_enabled(int enabled);
int ui_is_color_enabled(void);
Status ui_run(TextBuffer *buffer);

#endif
