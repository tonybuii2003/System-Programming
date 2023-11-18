#ifndef GLOBAL_H
#define GLOBAL_H

#include <signal.h>

extern volatile sig_atomic_t flag_exit;
extern volatile sig_atomic_t child_done;

void int_signal_handler(int sig);
void child_signal_handler(int sig);
int count_total_arg(char *str);
void print_error_with_line(char *line);
long options;

#define HELP_OPTION 1
#define QUIT_OPTION 2
#define RUN_OPTION 3
#define SHOW_OPTION 4
#define STOP_OPTION 5
#define CONT_OPTION 6
#define RELEASE_OPTION 7
#define WAIT_OPTION 8
#define KILL_OPTION 9
#define PEEK_OPTION 10
#define POKE_OPTION 11
#define BT_OPTION 12
#define OTHER_OPTION 13

#define MAX_INPUT 2048
#endif