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

typedef ap_uint<64> StateBits;

// Writing it this way avoids the ugly pack-unpack functions
union PackedState {
    State state;
    StateBits bits;
};

typedef ap_uint<64> StateBits;

// Make sure we are correcyly packing to 64 bit words.
static_assert(sizeof(State) == 8, "State must be exactly 64 bits");
static_assert(sizeof(PackedState) == 8, "PackedState must be exactly 64 bits");


void accelerator(
    StateBits* state,
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