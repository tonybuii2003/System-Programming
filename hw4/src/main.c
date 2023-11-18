#include <stdlib.h>
#include <stdio.h>
#include "deet.h"
#include "global.h"
#include "other_function.h"
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>
process_info *current_process = NULL;

int main(int argc, char *argv[])
{
    // current_process = malloc(sizeof(process_info));
    log_startup();
    // TO BE IMPLEMENTED
    // Remember: Do not put any functions other than main() in this file.
    struct sigaction action_int, action_child;
    action_int.sa_handler = int_signal_handler;
    sigemptyset(&action_int.sa_mask);
    action_int.sa_flags = 0;
    if (sigaction(SIGINT, &action_int, NULL) == -1)
    {
        exit(EXIT_FAILURE);
    }

    while (!flag_exit)
    {
        char *line = NULL;
        size_t size = MAX_INPUT + 1;
        ssize_t readval;
        log_prompt();
        write(1, "deet> ", strlen("deet> "));
        fflush(stdout);
        readval = getline(&line, &size, stdin);
        if (readval == 1)
        {
            continue;
        }
        if (readval == -1)
        {

            if (errno == EINTR)
                continue;
            else if (feof(stdin))
            {
                // sent SIGKILL to all existing processes that it is managing and it has
                // learned (via SIGCHLD/waitpid()) that those processes have actually terminated.
                // As soon as all extant processes have entered the PSTATE_DEAD state, then deet
                // will itself terminate without undue delay

                break;
            }
            log_error("\n?");
            continue;
        }
        log_input(line);
        line[readval - 1] = '\0';

        // COMPUTE THE ARGS
        char *ptr_input;
        char *input = strtok_r(line, " ", &ptr_input);
        char *prompt = malloc(strlen(ptr_input) + 1);
        if (prompt != NULL)
        {
            strcpy(prompt, ptr_input);
        }
        if (input != NULL)
        {
            if (strcmp(input, "help") == 0)
            {
                options = HELP_OPTION;
            }
            else if (strcmp(input, "quit") == 0)
            {
                if (count_total_arg(ptr_input) > 0)
                {
                    print_error_with_line(line);
                    continue;
                }
                options = QUIT_OPTION;
            }
            else if (strcmp(input, "run") == 0)
            {
                if (count_total_arg(ptr_input) < 1)
                {
                    print_error_with_line(line);
                    continue;
                }
                options = RUN_OPTION;
            }
            else if (strcmp(input, "show") == 0)
            {
                if (count_total_arg(ptr_input) > 1)
                {
                    print_error_with_line(line);
                    continue;
                }
                options = SHOW_OPTION;
            }
            else if (strcmp(input, "stop") == 0)
            {
                if (count_total_arg(ptr_input) != 1)
                {
                    print_error_with_line(line);
                    continue;
                }
                options = STOP_OPTION;
            }
            else if (strcmp(input, "cont") == 0)
            {
                if (count_total_arg(ptr_input) != 1)
                {
                    print_error_with_line(line);
                    continue;
                }
                options = CONT_OPTION;
            }
            else if (strcmp(input, "release") == 0)
            {
                if (count_total_arg(ptr_input) != 1)
                {
                    print_error_with_line(line);
                    continue;
                }
                options = RELEASE_OPTION;
            }
            else if (strcmp(input, "wait") == 0)
            {
                if (count_total_arg(ptr_input) != 1 && count_total_arg(ptr_input) != 2)
                {
                    print_error_with_line(line);
                    continue;
                }
                options = WAIT_OPTION;
            }
            else if (strcmp(input, "kill") == 0)
            {
                if (count_total_arg(ptr_input) != 1)
                {
                    print_error_with_line(line);
                    continue;
                }
                options = KILL_OPTION;
            }
            else if (strcmp(input, "peek") == 0)
            {
                if (count_total_arg(ptr_input) != 2 && count_total_arg(ptr_input) != 3)
                {
                    print_error_with_line(line);
                    continue;
                }
                options = PEEK_OPTION;
            }
            else if (strcmp(input, "poke") == 0)
            {
                if (count_total_arg(ptr_input) != 3)
                {
                    print_error_with_line(line);
                    continue;
                }
                options = POKE_OPTION;
            }
            else if (strcmp(input, "bt") == 0)
            {
                if (count_total_arg(ptr_input) != 1 && count_total_arg(ptr_input) != 2)
                {
                    print_error_with_line(line);
                    continue;
                }
                options = BT_OPTION;
            }
        }
        size_t token_size = count_total_arg(ptr_input) + 1;
        char *args_token[token_size];
        int token_index = 0;
        char *curr_token = strtok_r(NULL, " ", &ptr_input);
        while (curr_token != NULL)
        {
            args_token[token_index++] = curr_token;
            curr_token = strtok_r(NULL, " ", &ptr_input);
        }
        args_token[token_index] = NULL;
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

            break;
        }
        else if (options == RUN_OPTION)
        {
            action_child.sa_handler = child_signal_handler;
            sigemptyset(&action_child.sa_mask);
            action_child.sa_flags = SA_RESTART;
            if (sigaction(SIGCHLD, &action_child, NULL) == -1)
            {
                exit(EXIT_FAILURE);
            }
            pid_t pid = fork();
            if (pid < 0)
            {
                log_error("\n?");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            {
                // child
                // 1) redirect
                dup2(STDERR_FILENO, STDOUT_FILENO);
                // 2) start ptrace, child stop here?
                ptrace(PTRACE_TRACEME, 0, NULL, NULL);
                // 3) execute the command
                execvp(args_token[0], args_token);
            }
            else
            {
                // parent
                log_state_change(pid, PSTATE_NONE, PSTATE_RUNNING, 0);

                current_process = put_process(pid, 'T', "running", prompt);

                // 1) wait for child to stop (when signal is caught)
                while (child_done == 0)
                {
                }
                if (child_done == 1)
                {
                    child_done = 0;
                    update_process(current_process);
                }
            }
        }
        else if (options == KILL_OPTION)
        {
            int kill_index = atoi(args_token[0]);
            current_process = get_process(kill_index);
            if (current_process != NULL)
            {
                if (kill_program(current_process) == -1)
                {
                    print_error_with_line(line);
                    continue;
                }

                while (child_done == 0)
                {
                }
                if (child_done == 1)
                {
                    child_done = 0;
                    update_process(current_process);
                }
            }
        }
        else if (options == SHOW_OPTION)
        {
        }
        else
        {
            // other options
            print_error_with_line(line);
        }
        free(line);
    }
    free_process_list();
    log_shutdown();
    return 0;
}
