#include "commands.h"
#include "common.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

void print_help(void)
{
    printf("Usage: linee [<filename>]\n");
    printf("\nCommands list:\n");
    printf("	h               - prints this documentation\n");
    printf("	i [#]           - insert line at the current location or after #-th line\n");
    printf("	e  #            - edit (retype) #-th line\n");
    printf("	p [#,#]         - prints a range of lines (# to #), skips empty lines\n");
    printf("	P [#,#]         - same as 'p' but prints empty lines\n");
    printf("	r #[,#]         - removes #-th line [range of lines from # to #]\n");
    printf("	o               - opens file specified in the following prompt\n");
    printf("	w               - writes to a current file\n");
    printf("	W               - writes to a new file specified in the following prompt\n");
    printf("	f               - searches pattern, specified in the prompt, through the text\n");
    printf("	n               - prints next search result in the search queue\n");
    printf("	b               - prints previous search result in the search queue\n");
    printf("	q               - quit program\n");
    printf("	clear           - clears the screen\n");
    printf("Everything inside [] is optional.\n");
}

int save_new(Buffer* buf)
{
    char filename[256];
    printf("save as > ");
    if (fgets(filename, sizeof(filename), stdin)) {
        filename[strcspn(filename, "\n")] = '\0';
    }
    if (buffer_save_to_file(buf, filename) == 0) {
        strncpy(buf->last_filename, filename, sizeof(buf->last_filename));
        buf->last_filename[sizeof(buf->last_filename)-1] = '\0';
        return 0;
    } else {
        return -1;
    }
}

int save(Buffer* buf, bool as_new)
{
    if (buf->last_filename[0] == '\0' || as_new) {
        if (save_new(buf) == 0) return 0;
        else return -1;
    } else {
        if (buffer_save_to_file(buf, buf->last_filename) == 0) {
            return 0;
        } else {
            return -1;
        }
    }
}

Command parse_command(const char* line) {
    Command result = {'\0', 0, 0, 0};
    char buffer[100];
    strncpy(buffer, line, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0'; // Ensure null termination

    char* ptr = buffer;

    if (*ptr == '\0' || *ptr == '\n') return result;
    result.cmd = *(ptr++);
    while (isspace(*ptr)) ptr++;

    if (*ptr == '\0') return result;

    if (strchr(ptr, ',') && strchr(ptr, ' ')) {
        sscanf(ptr, "%d,%d %d", &result.arg1, &result.arg2, &result.arg3);
    } else if (strchr(ptr, ',')) {
        sscanf(ptr, "%d,%d", &result.arg1, &result.arg2);
    } else if (*ptr != '\0') {
        sscanf(ptr, "%d", &result.arg1);
    }

    return result;
}

void input_mode(Buffer* buf)
{
    char input[1024];

    size_t line_num = buf->current_line;

    while (1)
    {
        printf(DIM "%5zu │ " RESET, line_num+1);
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break; // EOF
        
        if (strcmp(input, ".\n") == 0)
            break;
        
        input[strcspn(input, "\n")] = '\0';
        buffer_insert_line(buf, line_num++, input);
    }
    buf->current_line = line_num;
}

void input_edit_line(Buffer* buf)
{
    char input[1024];

    size_t line_num = buf->current_line;

    printf(DIM "%5zu │ " RESET, line_num+1);
    fflush(stdout);

    if (!fgets(input, sizeof(input), stdin))
        return; // EOF

    if (strcmp(input, ".\n") == 0)
        return;

    input[strcspn(input, "\n")] = '\0';
    buffer_replace_line(buf, line_num, input);

    buf->current_line = line_num + 1;
    for (uint16_t i = 0; i < 1024; i++) input[i] = '\0';

    input_mode(buf);
}

void exec_command(Buffer* buf, const char* cmd)
{
    Command command = parse_command(cmd);

    switch (command.cmd)
    {
    case 'h': {
        print_help();
        break;
    }

    case 'i': {
        if (command.arg1 != 0)
            buf->current_line = command.arg1-1;
        input_mode(buf);
        break;
    }
    
    case 'e': {
        if (command.arg1 == 0) {
            printf("?\n");
            break;
        }
        buf->current_line = command.arg1-1;
        input_edit_line(buf);
        break;
    }
    
    case 'p': {
        if (command.arg1 == 0 || command.arg2 == 0)
            buffer_print_range(buf, 0, buf->count-1, true);
        else
            buffer_print_range(buf, command.arg1, command.arg2, true);
        break;
    }
    
    case 'P': {
        if (command.arg1 == 0 || command.arg2 == 0)
            buffer_print_range(buf, 0, buf->count-1, false);
        else
            buffer_print_range(buf, command.arg1, command.arg2, false);
        break;
    }

    case 'r': {
        if (command.arg1 == 0 && command.arg2 == 0)
            printf("?\n");
        else if (command.arg1 != 0 && command.arg2 == 0)
            buffer_delete_line(buf, command.arg1-1);
        else if (command.arg1 != 0 && command.arg2 != 0)
        {
            buffer_delete_range(buf, command.arg1-1, command.arg2-1);
        } else {
            printf("?\n");
        }
        break;
    }

    case 'o': {
        char filename[256];
        printf("filename > ");
        if (fgets(filename, sizeof(filename), stdin)) {
            filename[strcspn(filename, "\n")] = '\0';
        }
        if (buffer_load_from_file(buf, filename) == 0) {
            strncpy(buf->last_filename, filename, sizeof(buf->last_filename));
            buf->last_filename[sizeof(buf->last_filename)-1] = '\0';
            printf("Loaded '%s'\n", buf->last_filename);
        } else {
            fprintf(stderr, "Failed to open '%s'\n", filename);
        }
        break;
    }

    case 'W': {
        if (save(buf, true) == 0) printf("Saved to '%s'\n", buf->last_filename);
        else fprintf(stderr, "Error saving '%s'\n", buf->last_filename);
        break;
    }

    case 'w': {
        if (save(buf, false) == 0) printf("Saved to '%s'\n", buf->last_filename);
        else fprintf(stderr, "Error saving '%s'\n", buf->last_filename);
        break;
    }

    case 'f': {
        char pattern[256];
        printf("> ");
        if (fgets(pattern, sizeof(pattern), stdin)) {
            pattern[strcspn(pattern, "\n")] = '\0';
        }
        int res = buffer_search_pattern(buf, pattern);
        if (res == -1) printf("uh... idk\n");
        else {
            printf("Found %d matches\n", res);
            buffer_print_line(buf, buffer_search_current(buf));
        }
        break;
    }

    case 'n': {
        buffer_print_line(buf, buffer_search_next(buf));
        break;
    }

    case 'b': {
        buffer_print_line(buf, buffer_search_prev(buf));
        break;
    }
    
    default: {
        printf("?\n");
        break;
    }
    }
}