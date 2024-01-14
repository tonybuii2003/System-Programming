#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[])
{
    void *x = sf_malloc(sizeof(double) * 8);
    sf_show_heap();
    void *y = sf_realloc(x, sizeof(int));

    // sf_free(x);
    sf_free(y);
    return EXIT_SUCCESS;
}
