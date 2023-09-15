#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "debug.h"

int main(int argc, char **argv)
{

    if (validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);

    printf("global option: %ld\n", global_options);
    if (global_options == HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    // TO BE IMPLEMENTED
    printf("read distance data: %d\n", read_distance_data(stdin));
    return EXIT_FAILURE;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
