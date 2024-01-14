#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "csapp.h"

extern volatile sig_atomic_t flag_sighup;
void sighup_signal_handler(int sig);

#endif