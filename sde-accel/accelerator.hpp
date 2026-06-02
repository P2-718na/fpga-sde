#ifndef ACCELERATOR_HPP
#define ACCELERATOR_HPP

#include <ap_fixed.h>
#include <ap_int.h>

#define NUM_INPUTS  6
#define NUM_OUTPUTS 4

typedef ap_fixed<32,8> fixed_t;

typedef ap_uint<256> JobPacked;
typedef ap_uint<128> ResultPacked;

void accelerator(
    JobPacked* jobs,
    ResultPacked* results,
    int num_jobs
);

#endif