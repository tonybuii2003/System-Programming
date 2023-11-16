#include <stdlib.h>
#include <stdio.h>
#include "deet.h"
#include "global.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    log_startup();
    // TO BE IMPLEMENTED
    // Remember: Do not put any functions other than main() in this file.
    struct sigaction action;
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGINT, &action, NULL) == -1)
    {
        exit(EXIT_FAILURE);
    }

    while (!flag_exit)
    {
        log_prompt();
        char *line = NULL;
        size_t size = MAX_INPUT + 1;
        ssize_t readval;

        write(1, "deet> ", strlen("deet> "));
        fflush(stdout);
        readval = getline(&line, &size, stdin);
        if (readval == -1)
        {
            log_error("\n?\n");
            if (errno == EINTR)
                continue;
            else if (feof(stdin))
            {
                // sent SIGKILL to all existing processes that it is managing and it has
                // learned (via SIGCHLD/waitpid()) that those processes have actually terminated.
                // As soon as all extant processes have entered the PSTATE_DEAD state, then deet
                // will itself terminate without undue delay

                log_shutdown();
                break;
            }
        }
        log_input(line);
        line[readval - 1] = '\0';

        // COMPUTE THE ARGS
        char *ptr_input;
        char *input = strtok_r(line, " ", &ptr_input);
        if (input != NULL)
        {
            if (strcmp(input, "help") == 0)
            {
                options = HELP_OPTION;
            }
            else if (strcmp(input, "quit") == 0)
            {
                options = QUIT_OPTION;
            }
        }

        // RUN
        if (options == HELP_OPTION)
        {
            // PRINT MENU
            const char *print_help =
                "Available commands:\n"
                "help -- Print this help message\n"
                "quit (<=0 args) -- Quit the program\n"
                "show (<=1 args) -- Show process info\n"
                "run (>=1 args) -- Start a process\n"
                "stop (1 args) -- Stop a running process\n"
                "cont (1 args) -- Continue a stopped process\n"
                "release (1 args) -- Stop tracing a process, allowing it to continue normally\n"
                "wait (1-2 args) -- Wait for a process to enter a specified state\n"
                "kill (1 args) -- Forcibly terminate a process\n"
                "peek (2-3 args) -- Read from the address space of a traced process\n"
                "poke (3 args) -- Write to the address space of a traced process\n"
                "bt (1-2 args) -- Show a stack trace for a traced process\n";
            write(1, print_help, strlen(print_help));
        }
        else if (options == QUIT_OPTION)
        {
            // sent SIGKILL to all existing processes that it is managing and it has
            // learned (via SIGCHLD/waitpid()) that those processes have actually terminated.
            // As soon as all extant processes have entered the PSTATE_DEAD state, then deet
            // will itself terminate without undue delay

            log_shutdown();
            break;
        }
        else
        {
            // other options
            size_t msg_length = strlen(line) + strlen("\n?") + 1;
            char *msg = malloc(msg_length);
            strcpy(msg, line);
            strcat(msg, "\n?");
            log_error(msg);
            free(msg);
        }
        free(line);
    }
    return 0;
}
