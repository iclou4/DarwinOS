#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef __UINT64_TYPE__ stub_uint64_t;

int ccrng_uniform(struct ccrng_state *rng, uint64_t bound, uint64_t *rand)
{
        (void)rng;
        (void)bound;
        if ( rand != NULL )
                *rand = 0;
        return 0;
}

