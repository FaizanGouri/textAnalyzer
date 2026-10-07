#include "file_handler.h"

#include <stdio.h>

#define FILE_READ_CHUNK_SIZE 4096U

Status load_text_file(const char *filename, TextBuffer *buffer)
{
    FILE *file;
    TextBuffer loaded;
    char chunk[FILE_READ_CHUNK_SIZE];
    size_t bytes_read;
    Status status = STATUS_SUCCESS;

    if (filename == NULL || filename[0] == '\0' || buffer == NULL) {
        return STATUS_ERROR_INVALID;
    }

    file = fopen(filename, "rb");
    if (file == NULL) {
        return STATUS_ERROR_FILE;
    }

    text_buffer_init(&loaded);
    while ((bytes_read = fread(chunk, 1U, sizeof(chunk), file)) > 0U) {
        status = text_buffer_append_bytes(&loaded, chunk, bytes_read);
        if (status != STATUS_SUCCESS) {
            break;
        }
    }
    if (status == STATUS_SUCCESS && ferror(file) != 0) {
        status = STATUS_ERROR_FILE;
    }
    if (fclose(file) != 0 && status == STATUS_SUCCESS) {
        status = STATUS_ERROR_FILE;
    }
    if (status != STATUS_SUCCESS) {
        text_buffer_free(&loaded);
        return status;
    }

    text_buffer_clear(buffer);
    *buffer = loaded;
    text_buffer_init(&loaded);
    return STATUS_SUCCESS;
}

Status save_text_file(const char *filename, const TextBuffer *buffer)
{
    FILE *file;
    size_t bytes_written;
    int close_status;

    if (filename == NULL || filename[0] == '\0' || buffer == NULL ||
        (buffer->data == NULL && buffer->length > 0U)) {
        return STATUS_ERROR_INVALID;
    }

    file = fopen(filename, "wb");
    if (file == NULL) {
        return STATUS_ERROR_FILE;
    }

    bytes_written = 0U;
    if (buffer->length > 0U) {
        bytes_written = fwrite(buffer->data, 1U, buffer->length, file);
    }
    close_status = fclose(file);
    if (bytes_written != buffer->length || close_status != 0) {
        return STATUS_ERROR_FILE;
    }

    return STATUS_SUCCESS;
}
