#ifndef BUF_H
#define BUF_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char *text;
    size_t length;
    size_t capacity;
} Line;

typedef struct {
    Line *lines;
    size_t count;
    size_t capacity;
    size_t current_line;
    char last_filename[256];
    bool saved;

    size_t *search_results;
    size_t search_result_count;
    size_t search_result_index;
} Buffer;


Buffer* buffer_create();
void buffer_free(Buffer *buf);
int buffer_insert_line(Buffer *buf, size_t pos, const char *text);
int buffer_append_line(Buffer *buf, const char *text);
int buffer_delete_line(Buffer *buf, size_t pos);
int buffer_delete_range(Buffer *buf, size_t start, size_t end);
int buffer_replace_line(Buffer *buf, size_t pos, const char *text);
const char* buffer_get_line(Buffer *buf, size_t pos);
void buffer_print_range(Buffer *buf, size_t start, size_t end, bool compact);
void buffer_print_line(Buffer *buf, size_t pos);
size_t buffer_line_count(Buffer *buf);
int buffer_load_from_file(Buffer *buf, const char *filename);
int buffer_save_to_file(Buffer* buf, const char* filename);
int buffer_search_pattern(Buffer *buf, const char *pattern);
ssize_t buffer_search_current(Buffer *buf);
ssize_t buffer_search_next(Buffer *buf);
ssize_t buffer_search_prev(Buffer *buf);

#endif
