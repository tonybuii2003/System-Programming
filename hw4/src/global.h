#ifndef GLOBAL_H
#define GLOBAL_H

#include <signal.h>

extern volatile sig_atomic_t flag_exit;

void signal_handler(int sig);

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

#define MAX_INPUT 2048
#endif