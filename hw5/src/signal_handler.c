#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
volatile sig_atomic_t flag_sighup = 0;
void sighup_signal_handler(int sig)
{
    flag_sighup = 1;
}
