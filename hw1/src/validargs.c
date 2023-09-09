#include <stdlib.h>

#include "global.h"
#include "debug.h"
size_t getLen(char *s)
{
    int i = 0;
    size_t length = 0;
    while (*s != 0)
    {
        length += 1;
        s++;
    }
    return length;
}
/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
int validargs(int argc, char **argv)
{
    argv++;
    if (argc > 1 && **argv == '-')
    {
        (*argv)++;
        char selection = **argv;
        switch (selection)
        {
        case 'h':
            global_options = HELP_OPTION;
            return 0;
            break;
        case 'n':
            //  then the program reads distance data from the standard input,
            //  and synthesizes an unrooted tree using the neighbor joining method.
            global_options = NEWICK_OPTION;
            if (argc > 2)
            {
                argv++;
                if (**argv == '-')
                {
                    (*argv)++;
                    if (**argv == 'o')
                    {
                        if (argc > 3)
                        {
                            argv++;
                            if (**argv == '-')
                            {
                                return -1;
                            }
                            else
                            {
                                outlier_name = *argv;
                            }
                        }
                        else
                        {
                            return -1;
                        }
                    }
                }
                else
                {
                    return -1;
                }
            }
            return 0;
            break;
        case 'm':
            global_options = MATRIX_OPTION;
            if (argc > 2)
            {
                argv++;
                if (**argv == '-')
                {
                    (*argv)++;
                    if (**argv == 'o')
                    {
                        global_options = 0;
                        return -1;
                    }
                }
                return -1;
            }
            return 0;
            break;
        default:
            return -1;
        }
    }
    else if (argc <= 1)
        return 0;
    return -1;
}
