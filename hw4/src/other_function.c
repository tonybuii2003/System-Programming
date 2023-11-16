#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "deet.h"
int count_total_arg(const char *str)
{
    int count = 0;
    int in_arg = 0;

    while (*str)
    {
        if (*str != ' ' && !in_arg)
        {
            in_arg = 1;
            count++;
        }
        else if (*str == ' ')
        {
            in_arg = 0;
        }
        str++;
    }

    return count;
}
void print_error_with_line(char *line)
{
    size_t msg_length = strlen(line) + strlen("\n?") + 1;
    char *msg = malloc(msg_length);
    strcpy(msg, line);
    strcat(msg, "\n?");
    log_error(msg);
    free(msg);
}