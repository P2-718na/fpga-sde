#ifndef ACCELERATOR_HPP
#define ACCELERATOR_HPP

#include <ap_fixed.h>
#include <ap_int.h>
#include <cstdint>

constexpr int MAX_NEURONS = 12288;

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

struct ModelParams {
    int neuron_count;
    int edge_count;
    fixed_t J;
    fixed_t a;
    fixed_t sigma_sqrt_dt;
};

constexpr int NUM_BANKS = 2;

enum Bank : bool {
    BANK_A = 0,
    BANK_B = 1
};

// One row of the 4-stage RK4 Butcher tableau + rk_u ping-pong bank routing
struct StageConfig {
    Bank    eval_u_bank; // rk_u[] bank read for incoming edges
    Bank    next_u_bank; // rk_u[] bank written for next stage
    bool    is_init;     // Stage 0: evaluate at y_0 and init accumulator from y_0
    bool    is_final;    // Stage 3: finalize half-step & mirror acc.u -> rk_u[BANK_A]
    fixed_t a_u;         // Next-stage step weight / epsilon
    fixed_t a_v;         // Next-stage step weight
    fixed_t b_u;         // Quadrature weight / epsilon
    fixed_t b_v;         // Quadrature weight
};

// On-chip BRAM workspace organized for strict 1R1W II=1 access
struct WorkBuffers {
    State        state[NUM_BANKS][MAX_NEURONS]; // Persistent half-step ping-pong banks (A, B)
    fixed_t      rk_u[NUM_BANKS][MAX_NEURONS];  // Intermediate u banks for random edge reads (A, B)
    State        rk_eval[MAX_NEURONS];          // Sequential intermediate (u, v) at curr_neuron
    NeuronDegree out_deg[MAX_NEURONS];
};

void net_accel(
    // Large memory accesses
    const StateBits* state_in,       // Input state provided by PS. Will be copied once from DDR to BRAM
    const EdgeBits* edge_list,       // Static edge list, sorted by destination node (for w_nm, sorted by n)
                                     // EVERY NEURON MUST HAVE AT LEAST ONE EDGE IN THIS LIST!
    const NeuronDegree* out_degrees, // static out-degrees, again sorted by n. Also stored in BRAM to avoid switching read locations to/from edge_list
    StateBits* state_out,            // result of computation (copied back to DRAM from BRAM)

    // Integration parameters
    // (Keep them ints for simplicity)
    int neuron_count, // must not exceed MAX_NEURONS. (TODO CHECK, possibly add error handling flag)
    int edge_count,
    int iteration_count,

    // Model parameters
    fixed_t dt,
    fixed_t J,
    fixed_t a,
    fixed_t dt_over_e,
    fixed_t sigma_sqrt_dt,

    // RNG options
    ap_uint<32> seed,
    bool reseed
);

#endif
