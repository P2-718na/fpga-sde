#ifndef ACCELERATOR_HPP
#define ACCELERATOR_HPP

#include <ap_fixed.h>
#include <ap_int.h>

// Inputs are u, du, v, dv and I for each neuron (job)
// Outputs are just u, du, v, dv
#define NUM_INPUTS  5
#define NUM_OUTPUTS 4

typedef ap_fixed<32,8> fixed_t;

// HP ports have bandwidth of 64; no need to pad to 256
typedef ap_uint<192> JobPacked;
typedef ap_uint<128> ResultPacked;

void accelerator(
    JobPacked* jobs,
    ResultPacked* results,
    int num_jobs,
    fixed_t dt,
    fixed_t a,
    fixed_t e
);

#endif