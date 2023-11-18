#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include "deet.h"
#include "other_function.h"
static int process_index = 0;
static process_info process_list[MAX_LENGTH];
int count_total_arg(char *str)
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
process_info *get_process(int index)
{
    return &process_list[index];
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
void set_process(process_info *pinfo, char *current_state)
{
    pinfo->current_state = current_state;
    print_process(pinfo);
}
process_info *put_process(pid_t pid, char is_traced, char *current_state, char *prompt)
{
    // create new  process
    process_info *pinfo = malloc(sizeof(process_info));
    pinfo->pid = pid;
    pinfo->is_init = 1;
    pinfo->is_traced = is_traced;
    pinfo->current_state = malloc(strlen(current_state) + 1);
    if (pinfo->current_state != NULL)
    {
        strcpy(pinfo->current_state, current_state);
    }
    pinfo->prompt = malloc(strlen(prompt) + 1);
    if (pinfo->prompt != NULL)
    {
        strcpy(pinfo->prompt, prompt);
    }
    process_list[process_index] = *pinfo;
    pinfo->index = process_index;
    print_process(pinfo);
    process_index++;
    return pinfo;
}
void update_process(process_info *pinfo)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
    {
        if (WIFSTOPPED(status))
        {
            log_state_change(pid, PSTATE_RUNNING, PSTATE_STOPPED, WSTOPSIG(status));
            pinfo->status = WSTOPSIG(status);
            printf("stop status: %d\n", pinfo->status);
        }
        else if (WIFEXITED(status))
        {
            log_state_change(pid, PSTATE_KILLED, PSTATE_DEAD, WEXITSTATUS(status));
            pinfo->status = WEXITSTATUS(status);
            printf("kill status: %d\n", pinfo->status);
            // Update current_process for an exited child
        }
        else if (WIFSIGNALED(status))
        {
            log_state_change(pid, PSTATE_KILLED, PSTATE_DEAD, WTERMSIG(status));
            pinfo->status = WTERMSIG(status);
            printf("dead status: %d\n", pinfo->status);
            // Update current_process for a signaled child
        }
    }
}
void free_process(process_info *pinfo)
{
    if (pinfo != NULL)
    {
        free(pinfo->current_state);
        free(pinfo->prompt);
    }
}
void free_process_list()
{
    for (int i = 0; i < process_index; i++)
    {
        process_info *pinfo = &process_list[i];
        free_process(pinfo);
    }
}
void print_process(process_info *pinfo)
{
    printf("status: %d\n", pinfo->status);
    if (pinfo->status == 0x9)
    {
        printf("%d\t%d\t%c\t%s\t0x%x\t%s\n", pinfo->index, pinfo->pid, pinfo->is_traced, pinfo->current_state, pinfo->status, pinfo->prompt);
    }
    else
    {
        printf("%d\t%d\t%c\t%s\t\t%s\n", pinfo->index, pinfo->pid, pinfo->is_traced, pinfo->current_state, pinfo->prompt);
    }
}
void print_process_list()
{
    for (int i = 0; i < process_index; i++)
    {
        process_info *pinfo = &process_list[i];
        print_process(pinfo);
    }
}
int kill_program(process_info *pinfo)
{
    if (pinfo->is_init == 0)
    {
        return -1;
    }
    log_state_change(pinfo->pid, PSTATE_RUNNING, PSTATE_KILLED, 0);
    kill(pinfo->pid, SIGKILL);
    return 0;
}