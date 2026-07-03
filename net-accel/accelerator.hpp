#ifndef ACCELERATOR_HPP
#define ACCELERATOR_HPP

#include <ap_fixed.h>
#include <ap_int.h>
#include <cstdint>

constexpr int MAX_NEURONS = 24576; //~380Kib

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

//typedef ap_uint<LOG2_2048> NeuronIndex;
// we kinda are stuck with multiples of one byte
typedef ap_uint<32> NeuronIndex;
typedef ap_fixed<32,8> fixed_t;

struct State {
    fixed_t u;
    fixed_t v;
};
typedef ap_uint<64> StateBits;

// Make sure we are correcyly packing to 64 bit words.
static_assert(sizeof(State) == 8, "State must be exactly 64 bits");

// w_nm mean a link from M to N
struct Edge {
    NeuronIndex from;
    bool last;
};
typedef ap_uint<64> EdgeBits;

static_assert(sizeof(NeuronIndex) == 4, "Error");
static_assert(sizeof(bool) == 1, "Bool size is incorrect");
// Some weird padding is going on huh
static_assert(sizeof(Edge) <= sizeof(EdgeBits), "Incorrect packed state size"); 

void net_accel(
    const StateBits* state_in,
    const EdgeBits* edge_list,
    const NeuronIndex* out_degrees,
    StateBits* state_out,
    int neuron_count,
    int edge_count,
    int iteration_count,
    fixed_t dt,
    fixed_t J,
    fixed_t a,
    fixed_t inv_e,
    fixed_t sigma_sqrt_dt,
    ap_uint<32> seed,
    bool reseed
);

#endif