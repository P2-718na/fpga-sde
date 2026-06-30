#ifndef ACCELERATOR_HPP
#define ACCELERATOR_HPP

#include <ap_fixed.h>
#include <ap_int.h>

typedef ap_fixed<32,8> fixed_t;

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

struct State {
    fixed_t u;
    fixed_t v;
};

// Make sure we are correcyly packing to 64 bit words.
// Writing it this way avoids the ugly pack-unpack functions
static_assert(sizeof(State) == 8);

void accelerator(
    State* state,
    fixed_t* I,
    int num_jobs,
    fixed_t dt,
    fixed_t a,
    fixed_t inv_e,
    fixed_t sigma_sqrt_dt,
    ap_uint<32> seed,
    bool reseed
);

#endif