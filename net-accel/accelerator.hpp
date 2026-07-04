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

//typedef ap_uint<LOG2_2048> NeuronIndex; -- impossible to use
// we are kinda stuck with multiples of one byte
typedef ap_uint<32> NeuronIndex;
typedef NeuronIndex NeuronDegree; // Just to make notation clearer. The the degree of a neuron is by definition leq the max neuron id
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
    // unfortunately, everything has to be padded to 64bits.
    // We could use this unused 3 bytes of padding to store edge weights, maybe (?)
};
typedef ap_uint<64> EdgeBits;
// And check we are not doing some stupid stuff
static_assert(sizeof(NeuronIndex) == 4, "Error");
static_assert(sizeof(bool) == 1, "Bool size is incorrect");
static_assert(sizeof(Edge) == sizeof(EdgeBits), "Incorrect packed state size"); 

void net_accel(
    // Large memory accesses
    const StateBits* state_in,      // Input state provided by PS. Will be copied once from DDR to BRAM
    const EdgeBits* edge_list,      // Static edge list, sorted by destination node (for w_nm, sorted by n)
                                    // EVERY NEURON MUST HAVE AT LEAST ONE EDGE IN THIS LIST! (todo check: we can prolly force this by connecting every neuron to himself)
    const NeuronDegree* out_degrees, // static out-degrees, again sorted by n. Also stored in BRAM to avoid switching read locations to/from edge_list
    StateBits* state_out,           // result of computation (copied back to DRAM from BRAM)

    // Integration parameters
    // (Keep them ints for simplicity)
    int neuron_count, // must not exceed MAX_NEURONS
    int edge_count,
    int iteration_count,

    // Model parameters
    fixed_t dt,
    fixed_t J,
    fixed_t a,
    fixed_t inv_e,
    fixed_t sigma_sqrt_dt,

    // RNG options
    ap_uint<32> seed,
    bool reseed
);

#endif