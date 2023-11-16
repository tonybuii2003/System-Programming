#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "deet.h"

// BEGIN SIGNAL HANDLER
volatile sig_atomic_t flag_exit = 0;

void signal_handler(int sig)
{
    log_signal(sig);
    if (sig == SIGINT)
    {
        flag_exit = 1;
    }
}
// END SIGNAL HANDLER
