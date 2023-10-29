#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[])
{
    double *ptr = sf_malloc(16384 - 48 - (sizeof(sf_header) + sizeof(sf_footer)));
    // double *ptr = sf_malloc(sizeof(double));
    //  *ptr = 320320320e-320;
    *ptr = 4;
    printf("%f\n", *ptr);

    sf_free(ptr);

    return EXIT_SUCCESS;
}
