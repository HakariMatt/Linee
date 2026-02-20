#ifndef COMMANDS_H
#define COMMANDS_H
#include "buf.h"

typedef struct {
    char cmd;
    int arg1;
    int arg2;
    int arg3;
} Command;

Command parse_command(const char* line);
void exec_command(Buffer* buf, const char* cmd);
void print_help();
int save(Buffer* buf, bool as_new);

#endif