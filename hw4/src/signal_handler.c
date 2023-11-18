#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "deet.h"

volatile sig_atomic_t flag_exit = 0;
volatile sig_atomic_t child_done = 0;
void int_signal_handler(int sig)
{
    log_signal(sig);
    if (sig == SIGINT)
    {
        flag_exit = 1;
    }
}

void child_signal_handler(int sig)
{
    log_signal(sig);
    if (sig == SIGCHLD)
    {
        child_done = 1;
    }
}
