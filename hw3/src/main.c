#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[])
{
    // double *ptr = sf_malloc(PAGE_SZ << 16);
    double *ptr = sf_malloc(sizeof(double));
    //  *ptr = 320320320e-320;
    *ptr = 4;
    printf("%f\n", *ptr);

    sf_free(ptr);

    return EXIT_SUCCESS;
}
