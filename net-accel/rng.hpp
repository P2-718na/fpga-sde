#ifndef RNG_HPP
#define RNG_HPP

#include <ap_int.h>
#include <ap_fixed.h>

#include "accelerator.hpp"

#define GAUSS_UNIFORMS 8
#if GAUSS_UNIFORMS == 4
#define CLT_SCALE (fixed_t)1.73205080757
#elif GAUSS_UNIFORMS == 8
#define CLT_SCALE (fixed_t)1.22474487139
#elif GAUSS_UNIFORMS == 12
#define CLT_SCALE (fixed_t)1.0
#else
#error "Unsupported GAUSS_UNIFORMS"
#endif

// Only used in testbench
int rng_test();


// Gotta use ap_uint so that we can call the .range function
static ap_uint<32> xorshift32(ap_uint<32>& state) {
#pragma HLS INLINE

    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;

    return state;
}

// Uniform generator between 0 and 1. 
static fixed_t uniform01(ap_uint<32>& state) {
#pragma HLS INLINE

    // Here, we want to generate a random uniform between 0 and 1.
    // Since we are using the q8.24 format, one way to do so is just discard the 8 integer bits
    // and just use the upper 24 bits of the generator as the fractional bits.
    // We loose precision, but now the distribution properties (mean and variane) are known and small (same order of 1). 
    ap_uint<32> r = xorshift32(state);
    ap_ufixed<24,0> frac;
    // Range just copies elements bitwise
    frac.range(23,0) = r.range(31,8);

    return (fixed_t)frac;
}

static fixed_t gaussian_clt(ap_uint<32> rng_state[GAUSS_UNIFORMS]) {
#pragma HLS INLINE

    fixed_t sum = 0;

    for (int i = 0; i < GAUSS_UNIFORMS; ++i) {
    #pragma HLS UNROLL
        sum += uniform01(rng_state[i]);
    }

    // TODO maybe properly check fixed number scales if they make sense (we don't wanna waste too much precision)
    return (sum - (fixed_t)(GAUSS_UNIFORMS/2)) * CLT_SCALE;
}



#endif