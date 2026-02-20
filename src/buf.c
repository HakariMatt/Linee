#include "buf.h"
#include "common.h"

#define BUFFER_INITIAL_CAPACITY 128
#define LINE_INITIAL_CAPACITY 256

Buffer* buffer_create() {
    Buffer* buf = malloc(sizeof(Buffer));
    if (!buf) return NULL;
    
    buf->lines = malloc(sizeof(Line) * BUFFER_INITIAL_CAPACITY);
    if (!buf->lines) {
        free(buf);
        return NULL;
    }
    
    buf->count = 0;
    buf->capacity = BUFFER_INITIAL_CAPACITY;
    buf->current_line = 0;

    buf->search_results = NULL;
    buf->search_result_count = 0;
    buf->search_result_index = 0;

    buf->saved = true;
    return buf;
}

void buffer_free(Buffer* buf) {
    if (!buf) return;
    
    for (sz i = 0; i < buf->count; i++) {
        free(buf->lines[i].text);
    }
    free(buf->search_results);
    free(buf->lines);
    free(buf);
}

i32 buffer_insert_line(Buffer* buf, sz pos, const char* text) {
    if (!buf) return -1;
    
    if (pos > buf->count) {
        while (buf->capacity <= pos) {
            sz new_capacity = buf->capacity * 2;
            Line* new_lines = realloc(buf->lines, sizeof(Line) * new_capacity);
            if (!new_lines) return -1;
            buf->lines = new_lines;
            buf->capacity = new_capacity;
        }
        
        for (sz i = buf->count; i < pos; i++) {
            buf->lines[i].text = malloc(LINE_INITIAL_CAPACITY);
            if (!buf->lines[i].text) return -1;
            buf->lines[i].text[0] = '\0';
            buf->lines[i].length = 0;
            buf->lines[i].capacity = LINE_INITIAL_CAPACITY;
        }
        
        buf->count = pos;
    }
    
    if (buf->count >= buf->capacity) {
        sz new_capacity = buf->capacity * 2;
        Line* new_lines = realloc(buf->lines, sizeof(Line) * new_capacity);
        if (!new_lines) return -1;
        buf->lines = new_lines;
        buf->capacity = new_capacity;
    }
    
    for (sz i = buf->count; i > pos; i--) {
        buf->lines[i] = buf->lines[i-1];
    }
    
    sz text_len = strlen(text);
    sz capacity = (text_len < LINE_INITIAL_CAPACITY) ? 
        LINE_INITIAL_CAPACITY : text_len + 1;
    
    buf->lines[pos].text = malloc(capacity);
    if (!buf->lines[pos].text) return -1;
    
    strncpy(buf->lines[pos].text, text, text_len + 1);
    buf->lines[pos].length = text_len;
    buf->lines[pos].capacity = capacity;
    
    buf->count++;
    buf->saved = false;
    return 0;
}

i32 buffer_append_line(Buffer* buf, const char* text) {
    return buffer_insert_line(buf, buf->count, text);
}

i32 buffer_delete_line(Buffer* buf, sz pos) {
    if (pos >= buf->count || !buf) return -1;
    
    free(buf->lines[pos].text);
    
    for (sz i = pos; i < buf->count - 1; i++) {
        buf->lines[i] = buf->lines[i+1];
    }
    
    buf->count--;
    buf->saved = false;
    return 0;
}

i32 buffer_delete_range(Buffer* buf, sz start, sz end) {
    if (!buf || start > end || end >= buf->count) return -1;

    for (sz i = start; i <= end; i++) {
        free(buf->lines[i].text);
    }

    sz shift_count = end - start + 1;
    for (sz i = end + 1; i < buf->count; i++) {
        buf->lines[i - shift_count] = buf->lines[i];
    }

    buf->count -= shift_count;
    buf->saved = false;
    return 0;
}

i32 buffer_replace_line(Buffer* buf, sz pos, const char* text) {
    if (pos >= buf->count || !buf) return -1;
    
    sz text_len = strlen(text);
    Line* line = &buf->lines[pos];
    
    if (text_len >= line->capacity) {
        sz new_capacity = line->capacity * 2;
        if (new_capacity <= text_len) new_capacity = text_len + 1;
        
        char* new_text = realloc(line->text, new_capacity);
        if (!new_text) return -1;
        
        line->text = new_text;
        line->capacity = new_capacity;
    }
    
    strncpy(line->text, text, text_len + 1);
    line->length = text_len;
    buf->saved = false;
    return 0;
}

const char* buffer_get_line(Buffer* buf, sz pos) {
    if (pos >= buf->count || !buf) return NULL;
    return buf->lines[pos].text;
}

void buffer_print_range(Buffer* buf, sz start, sz end, bool compact) {
    if (!buf || start > end || end >= buf->count) return;
    
    for (sz i = start; i <= end; i++) {
        if (compact && buf->lines[i].text[0] == '\0')
            continue;
        printf("%5zu │ %s\n", i+1, buf->lines[i].text);
    }
}

void buffer_print_line(Buffer* buf, sz pos)
{
    if (!buf || pos >= buf->count || pos < 0) return;
    printf("%5zu │ %s\n", pos+1, buf->lines[pos].text);
}

sz buffer_line_count(Buffer* buf) {
    return buf ? buf->count : 0;
}

i32 buffer_load_from_file(Buffer* buf, const char* filename) {
    if (!buf || !filename) return -1;

    FILE* file = fopen(filename, "r");
    if (!file) return -1;

    while (buf->count > 0) {
        buffer_delete_line(buf, 0);
    }

    char* line = NULL;
    sz len = 0;
    ssz read;

    while ((read = getline(&line, &len, file)) != -1) {
        if (read > 0 && (line[read - 1] == '\n' || line[read - 1] == '\r')) {
            line[--read] = '\0';
        }
        buffer_append_line(buf, line);
    }

    free(line);
    fclose(file);
    buf->saved = true;
    return 0;
}

i32 buffer_save_to_file(Buffer* buf, const char* filename)
{
    if (!buf || !filename) return -1;

    FILE* file = fopen(filename, "w");
    if (!file) return -1;

    for (sz i = 0; i < buf->count; i++)
    {
        const char* line = buffer_get_line(buf, i);
        if (line) {
            fprintf(file, "%s\n", line);
        }
    }
    fclose(file);
    buf->saved = true;
    return 0;
}

i32 buffer_search_pattern(Buffer* buf, const char* pattern) {
    if (!buf || !pattern) return -1;

    free(buf->search_results);
    buf->search_results = NULL;
    buf->search_result_count = 0;
    buf->search_result_index = 0;

    sz* results = malloc(sizeof(sz)*  buf->count);
    if (!results) return -1;

    sz count = 0;
    for (sz i = 0; i < buf->count; i++) {
        if (strstr(buf->lines[i].text, pattern)) {
            results[count++] = i;
        }
    }

    if (count > 0) {
        buf->search_results = results;
        buf->search_result_count = count;
        buf->search_result_index = 0;
    } else {
        free(results);
    }

    return (i32)count;
}

ssz buffer_search_current(Buffer* buf) {
    if (!buf || buf->search_result_count == 0) return -1;
    return (ssz)buf->search_results[buf->search_result_index];
}

ssz buffer_search_next(Buffer* buf) {
    if (!buf || buf->search_result_count == 0) return -1;
    buf->search_result_index = (buf->search_result_index + 1) % buf->search_result_count;
    return (ssz)buf->search_results[buf->search_result_index];
}

ssz buffer_search_prev(Buffer* buf) {
    if (!buf || buf->search_result_count == 0) return -1;
    if (buf->search_result_index == 0)
        buf->search_result_index = buf->search_result_count - 1;
    else
        buf->search_result_index--;
    return (ssz)buf->search_results[buf->search_result_index];
}