#ifndef COMMANDS_H
#define COMMANDS_H
#include "buf.h"

typedef struct {
    char cmd;
    i32 arg1;
    i32 arg2;
    i32 arg3;
} Command;

Command parse_command(const char* line);
void exec_command(Buffer* buf, const char* cmd);
void print_help();
i32 save(Buffer* buf, bool as_new);

#endif