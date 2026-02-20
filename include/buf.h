#ifndef BUF_H
#define BUF_H

#include "common.h"

typedef struct {
    char* text;
    sz length;
    sz capacity;
} Line;

typedef struct {
    Line* lines;
    sz count;
    sz capacity;
    sz current_line;
    char last_filename[256];
    bool saved;

    sz* search_results;
    sz search_result_count;
    sz search_result_index;
} Buffer;


Buffer* buffer_create();
void buffer_free(Buffer* buf);
i32 buffer_insert_line(Buffer* buf, sz pos, const char* text);
i32 buffer_append_line(Buffer* buf, const char* text);
i32 buffer_delete_line(Buffer* buf, sz pos);
i32 buffer_delete_range(Buffer* buf, sz start, sz end);
i32 buffer_replace_line(Buffer* buf, sz pos, const char* text);
const char* buffer_get_line(Buffer* buf, sz pos);
void buffer_print_range(Buffer* buf, sz start, sz end, bool compact);
void buffer_print_line(Buffer* buf, sz pos);
sz buffer_line_count(Buffer* buf);
i32 buffer_load_from_file(Buffer* buf, const char* filename);
i32 buffer_save_to_file(Buffer* buf, const char* filename);
i32 buffer_search_pattern(Buffer* buf, const char* pattern);
ssz buffer_search_current(Buffer* buf);
ssz buffer_search_next(Buffer* buf);
ssz buffer_search_prev(Buffer* buf);

#endif
