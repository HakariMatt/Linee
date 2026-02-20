#include <stdio.h>
#include <string.h>
#include "buf.h"
#include "common.h"

#define BUFFER_INITIAL_CAPACITY 128
#define LINE_INITIAL_CAPACITY 256

Buffer* buffer_create() {
    Buffer *buf = malloc(sizeof(Buffer));
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

void buffer_free(Buffer *buf) {
    if (!buf) return;
    
    for (size_t i = 0; i < buf->count; i++) {
        free(buf->lines[i].text);
    }
    free(buf->search_results);
    free(buf->lines);
    free(buf);
}

int buffer_insert_line(Buffer *buf, size_t pos, const char *text) {
    if (!buf) return -1;
    
    if (pos > buf->count) {
        while (buf->capacity <= pos) {
            size_t new_capacity = buf->capacity * 2;
            Line *new_lines = realloc(buf->lines, sizeof(Line) * new_capacity);
            if (!new_lines) return -1;
            buf->lines = new_lines;
            buf->capacity = new_capacity;
        }
        
        for (size_t i = buf->count; i < pos; i++) {
            buf->lines[i].text = malloc(LINE_INITIAL_CAPACITY);
            if (!buf->lines[i].text) return -1;
            buf->lines[i].text[0] = '\0';
            buf->lines[i].length = 0;
            buf->lines[i].capacity = LINE_INITIAL_CAPACITY;
        }
        
        buf->count = pos;
    }
    
    if (buf->count >= buf->capacity) {
        size_t new_capacity = buf->capacity * 2;
        Line *new_lines = realloc(buf->lines, sizeof(Line) * new_capacity);
        if (!new_lines) return -1;
        buf->lines = new_lines;
        buf->capacity = new_capacity;
    }
    
    for (size_t i = buf->count; i > pos; i--) {
        buf->lines[i] = buf->lines[i-1];
    }
    
    size_t text_len = strlen(text);
    size_t capacity = (text_len < LINE_INITIAL_CAPACITY) ? 
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

int buffer_append_line(Buffer *buf, const char *text) {
    return buffer_insert_line(buf, buf->count, text);
}

int buffer_delete_line(Buffer *buf, size_t pos) {
    if (pos >= buf->count || !buf) return -1;
    
    free(buf->lines[pos].text);
    
    for (size_t i = pos; i < buf->count - 1; i++) {
        buf->lines[i] = buf->lines[i+1];
    }
    
    buf->count--;
    buf->saved = false;
    return 0;
}

int buffer_delete_range(Buffer *buf, size_t start, size_t end) {
    if (!buf || start > end || end >= buf->count) return -1;

    for (size_t i = start; i <= end; i++) {
        free(buf->lines[i].text);
    }

    size_t shift_count = end - start + 1;
    for (size_t i = end + 1; i < buf->count; i++) {
        buf->lines[i - shift_count] = buf->lines[i];
    }

    buf->count -= shift_count;
    buf->saved = false;
    return 0;
}

int buffer_replace_line(Buffer *buf, size_t pos, const char *text) {
    if (pos >= buf->count || !buf) return -1;
    
    size_t text_len = strlen(text);
    Line *line = &buf->lines[pos];
    
    if (text_len >= line->capacity) {
        size_t new_capacity = line->capacity * 2;
        if (new_capacity <= text_len) new_capacity = text_len + 1;
        
        char *new_text = realloc(line->text, new_capacity);
        if (!new_text) return -1;
        
        line->text = new_text;
        line->capacity = new_capacity;
    }
    
    strncpy(line->text, text, text_len + 1);
    line->length = text_len;
    buf->saved = false;
    return 0;
}

const char* buffer_get_line(Buffer *buf, size_t pos) {
    if (pos >= buf->count || !buf) return NULL;
    return buf->lines[pos].text;
}

void buffer_print_range(Buffer *buf, size_t start, size_t end, bool compact) {
    if (!buf || start > end || end >= buf->count) return;
    
    for (size_t i = start; i <= end; i++) {
        if (compact && buf->lines[i].text[0] == '\0')
            continue;
        printf("%5zu │ %s\n", i+1, buf->lines[i].text);
    }
}

void buffer_print_line(Buffer *buf, size_t pos)
{
    if (!buf || pos >= buf->count || pos < 0) return;
    printf("%5zu │ %s\n", pos+1, buf->lines[pos].text);
}

size_t buffer_line_count(Buffer *buf) {
    return buf ? buf->count : 0;
}

int buffer_load_from_file(Buffer *buf, const char *filename) {
    if (!buf || !filename) return -1;

    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    while (buf->count > 0) {
        buffer_delete_line(buf, 0);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

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

int buffer_save_to_file(Buffer* buf, const char* filename)
{
    if (!buf || !filename) return -1;

    FILE* file = fopen(filename, "w");
    if (!file) return -1;

    for (size_t i = 0; i < buf->count; i++)
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

int buffer_search_pattern(Buffer *buf, const char *pattern) {
    if (!buf || !pattern) return -1;

    free(buf->search_results);
    buf->search_results = NULL;
    buf->search_result_count = 0;
    buf->search_result_index = 0;

    size_t *results = malloc(sizeof(size_t) * buf->count);
    if (!results) return -1;

    size_t count = 0;
    for (size_t i = 0; i < buf->count; i++) {
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

    return (int)count;
}

ssize_t buffer_search_current(Buffer *buf) {
    if (!buf || buf->search_result_count == 0) return -1;
    return (ssize_t)buf->search_results[buf->search_result_index];
}

ssize_t buffer_search_next(Buffer *buf) {
    if (!buf || buf->search_result_count == 0) return -1;
    buf->search_result_index = (buf->search_result_index + 1) % buf->search_result_count;
    return (ssize_t)buf->search_results[buf->search_result_index];
}

ssize_t buffer_search_prev(Buffer *buf) {
    if (!buf || buf->search_result_count == 0) return -1;
    if (buf->search_result_index == 0)
        buf->search_result_index = buf->search_result_count - 1;
    else
        buf->search_result_index--;
    return (ssize_t)buf->search_results[buf->search_result_index];
}