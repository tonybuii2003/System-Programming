#ifndef PROCESS_H
#define PROCESS_H
#include <sys/types.h>
#define MAX_LENGTH 2048

typedef struct
{
    int index;
    pid_t pid;
    int is_traced;
    char *current_state;
    char *prompt;
    int is_dead;
    int status;
} process_info;
process_info *put_process(pid_t pid, char is_traced, char *current_state, char *prompt);
void free_process_list();
void print_process_list();
void print_process(process_info *pinfo);
void set_process(process_info *pinfo, char *current_state);
process_info *get_process(int index);
void kill_program(process_info *pinfo);
void free_process(process_info *pinfo);
void update_process();
#endif