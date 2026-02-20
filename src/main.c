#include <stdio.h>
#include <string.h>
#include "buf.h"
#include "common.h"
#include "commands.h"

void print_prompt(Buffer* buf)
{
    if (buf->saved) printf(SAVED " ");
    else printf(UNSAVED "*");
    printf("[cmd]" RESET " ");
}

int main(int argc, char* argv[])
{
    Buffer* buf = buffer_create();

    if (argc > 1) {
        buffer_load_from_file(buf, argv[1]);
    }

    char input[256];

    while (1)
    {
        print_prompt(buf);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = 0;
    
        if (strcmp(input, "q") == 0)
        {
            if (buf->saved == false)
            {
                printf("You have unsaved changes. Do you want to save? [y/n] ");
                char confirmation[3];
                if (fgets(confirmation, sizeof(confirmation), stdin)) {
                    confirmation[strcspn(confirmation, "\n")] = '\0';
                }
                if (confirmation[0] == 'y' || confirmation[0] == 'Y')
                    save(buf, false);
                else if (confirmation[0] == 'n' || confirmation[0] == 'N') break;
                else {printf("?\n"); continue;}
            }
            else break;
        }
        else if (strcmp(input, "clear") == 0) printf("\033[2J\033[H");
        else exec_command(buf, input);
    }
    return 0;
}
