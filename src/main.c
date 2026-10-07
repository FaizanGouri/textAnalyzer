#include "text_utils.h"
#include "ui.h"

int main(void)
{
    TextBuffer buffer;

    text_buffer_init(&buffer);
    (void)ui_run(&buffer);
    text_buffer_free(&buffer);

    return 0;
}
