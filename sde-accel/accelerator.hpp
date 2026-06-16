#ifndef ACCELERATOR_HPP
#define ACCELERATOR_HPP

#include <ap_fixed.h>
#include <ap_int.h>

// FIXME All of this defines should probably be typedefs.
// They still allow for unroll

// Inputs are u, v and I for each neuron (job)
// Outputs are just u, v.
// Since FH is first-order, we don't need the derivatives...
#define NUM_INPUTS  3
#define NUM_OUTPUTS 2

typedef ap_fixed<32,8> fixed_t;

// How many LCG to use for gaussian generator
#define GAUSS_UNIFORMS 8

#if GAUSS_UNIFORMS == 4
#define CLT_SCALE (fixed_t)1.73205080757;
#elif GAUSS_UNIFORMS == 8
#define CLT_SCALE (fixed_t)1.22474487139;
#elif GAUSS_UNIFORMS == 12
#define CLT_SCALE (fixed_t)1.0;
#else
    #error "Unsupported GAUSS_UNIFORMS"
#endif

// HP ports have bandwidth of 64; no need to pad to 256
typedef ap_uint<128> JobPacked;
typedef ap_uint<64> ResultPacked;

void accelerator(
    JobPacked* jobs,
    ResultPacked* results,
    int num_jobs,
    fixed_t dt,
    fixed_t a,
    fixed_t inv_e,
    fixed_t sigma_sqrt_dt,
    ap_uint<32> seed,
    bool reseed
);

#endif