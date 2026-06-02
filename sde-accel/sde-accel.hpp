#ifndef ACCELERATOR_HPP
#define ACCELERATOR_HPP

#include <ap_fixed.h>

#define NUM_INPUTS   6
#define NUM_OUTPUTS  4

typedef ap_fixed<32, 8> fixed_t;

void accelerator(
    fixed_t jobs[][NUM_INPUTS],
    fixed_t results[][NUM_OUTPUTS],
    int num_jobs
);

#endif