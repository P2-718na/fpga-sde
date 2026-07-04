#include "accelerator.hpp"
#include <hls_stream.h>
#include <iostream>

// FIXME AFER CLEANU IT UCKING DOESNT WORK
// ONLY WORKS FOR NEURONS UP TO 16

// RNG //////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////



// Neuron integration ////////////////////////////////////////////////////////////////
// Duplicate the internal state_curr buffer into two separate queues
// so we avoid the deadlock due to concurrent reads. FOR SOME REASON I HAVE TO WRITE THIS MYSELF (NOT) THANKS COMPILER
static void read_states(
    const State state_curr[MAX_NEURONS],
    int neuron_count,
    hls::stream<State>& integrate_state_stream,
    hls::stream<fixed_t>& outgoing_u_stream
) {
    READ_LOOP:
    for (int n = 0; n < neuron_count; ++n) {
    #pragma HLS PIPELINE II=1   

        State s = state_curr[n];

        integrate_state_stream.write(s);
        outgoing_u_stream.write(s.u);
    }
}

static void integrate_one( // Euler-maruyama integration step
    const State& old_s,
    State& new_s,
    fixed_t I,
    fixed_t dt,
    fixed_t a,
    fixed_t inv_e,
    fixed_t sigma_sqrt_dt,
    ap_uint<32> rng_state[GAUSS_UNIFORMS]
) {
#pragma HLS INLINE

    const fixed_t u = old_s.u;
    const fixed_t v = old_s.v;

    const fixed_t du = (u - u*u*u*(fixed_t)(1.0/3.0) - v + I) * inv_e;
    const fixed_t dv = u + a;
    // Qui non sto dividendo per epsilon per salvare risorse.
    // La temperatura lato python deve contenere il fattore giusto.
    const fixed_t noise = sigma_sqrt_dt * gaussian_clt(rng_state);

    new_s.u = u + du * dt + noise;
    new_s.v = v + dv * dt;
}


static void integrate_all(
    hls::stream<State>& state_stream,
    State state_next[MAX_NEURONS],
    int neuron_count,
    hls::stream<fixed_t>& I_stream,
    fixed_t dt,
    fixed_t a,
    fixed_t inv_e,
    fixed_t sigma_sqrt_dt,
    ap_uint<32> rng_state[GAUSS_UNIFORMS]
) {
    NEURON_LOOP: 
    for (int n = 0; n < neuron_count; ++n) {
    #pragma HLS PIPELINE II=1

        State old_s = state_stream.read();
        fixed_t I = I_stream.read();

        integrate_one(
            old_s,
            state_next[n],
            I,
            dt,
            a,
            inv_e,
            sigma_sqrt_dt,
            rng_state
        );
    }
}

// Computes outgoing term for flows. 
static void compute_outgoing(
    hls::stream<fixed_t>& u_stream,
    const NeuronIndex out_degrees[MAX_NEURONS],
    int neuron_count,
    hls::stream<fixed_t>& out_stream
) {
    OUT_LOOP:
    for (int n = 0; n < neuron_count; ++n) {
    #pragma HLS PIPELINE II=1

        fixed_t u = u_stream.read();
        fixed_t term = (fixed_t)out_degrees[n] * u;
        out_stream.write(term);
    }
}

static void compute_flows(
    const State state_curr[MAX_NEURONS],
    const EdgeBits* edge_list,
    int edge_count,
    fixed_t J,
    hls::stream<fixed_t>& out_stream,
    hls::stream<fixed_t>& I_stream
) {
    fixed_t incoming_sum = 0;

    EDGE_LOOP:
    for (int e = 0; e < edge_count; ++e) {
    #pragma HLS PIPELINE II=1

        // Parse packed edge format
        EdgeBits bits = edge_list[e];
        uint32_t from = bits.range(31, 0);
        bool last     = bits[32];

        incoming_sum += state_curr[from].u;

        // EVERY Neuron MUST have a last edge. We force this by making every neuron connected to itself.
        // The interaction terms of it with itself cancel out in this branch...
        if (last) {
            fixed_t outgoing_term = out_stream.read();
            fixed_t I =  J * (incoming_sum - outgoing_term);

            // Produce EXACTLY 1 I per neuron
            I_stream.write(I);

            incoming_sum = 0;
        }
    }
}


static void timestep(
    const State state_curr[MAX_NEURONS],
    State state_next[MAX_NEURONS],
    const EdgeBits* edge_list,
    const NeuronDegree out_degrees[MAX_NEURONS],
    int neuron_count,
    int edge_count,
    fixed_t dt,
    fixed_t J,
    fixed_t a,
    fixed_t inv_e,
    fixed_t sigma_sqrt_dt,
    ap_uint<32> rng_state[GAUSS_UNIFORMS]
) {
// We want to _stream_
// Basically, the next operation will start alongside the first, as soon as the first bytes of useful data come out.
// Depth of 16 allows for a small buffer in the operations, in case some loops trip way later/sooner than the others.
// Dataflow pragma is HERE, and not in top level, because we wanna make sure that the BRAM pingpong is not DATAFLOW'd.
// Also to allow for easier swapping syntax.
#pragma HLS DATAFLOW

    hls::stream<State> state_stream;
    hls::stream<fixed_t> outgoing_u_stream;
    hls::stream<fixed_t> out_stream;
    hls::stream<fixed_t> I_stream;

    #pragma HLS STREAM variable=state_stream depth=16
    #pragma HLS STREAM variable=outgoing_u_stream depth=16
    #pragma HLS STREAM variable=out_stream depth=16
    #pragma HLS STREAM variable=I_stream depth=16

    // This is necessary to fanout the value of state_curr[n] otherwise it deadlocks
    // (thanks compiler for not inferring this)
    read_states(
        state_curr,
        neuron_count,
        state_stream,
        outgoing_u_stream
    );

    compute_outgoing(
        outgoing_u_stream,
        out_degrees,
        neuron_count,
        out_stream
    );

    compute_flows(
        state_curr,
        edge_list,
        edge_count,
        J,
        out_stream,
        I_stream
    );

    integrate_all(
        state_stream,
        state_next,
        neuron_count,
        I_stream,
        dt,
        a,
        inv_e,
        sigma_sqrt_dt,
        rng_state
    );
}

void net_accel(
    const StateBits* state_in,
    const EdgeBits* edge_list,
    const NeuronDegree* out_degrees_in,
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
) {
// HP Port connections
// depth pragmas are useful only in RTL simulation
#pragma HLS INTERFACE m_axi port=edge_list        offset=slave bundle=gmem0 depth=4000// This has to have its own HP port (most critical)
#pragma HLS INTERFACE m_axi port=state_in         offset=slave bundle=gmem1 depth=2000
#pragma HLS INTERFACE m_axi port=state_out        offset=slave bundle=gmem1 depth=2000
#pragma HLS INTERFACE m_axi port=out_degrees_in   offset=slave bundle=gmem2 depth=2000 // different port than state in/out because has different width. This way we allow bursting

// Accelerator parameters
#pragma HLS INTERFACE s_axilite port=state_in
#pragma HLS INTERFACE s_axilite port=edge_list
#pragma HLS INTERFACE s_axilite port=out_degrees_in
#pragma HLS INTERFACE s_axilite port=state_out
#pragma HLS INTERFACE s_axilite port=neuron_count
#pragma HLS INTERFACE s_axilite port=edge_count
#pragma HLS INTERFACE s_axilite port=iteration_count
#pragma HLS INTERFACE s_axilite port=return

// Model parameters
#pragma HLS INTERFACE s_axilite port=dt
#pragma HLS INTERFACE s_axilite port=J
#pragma HLS INTERFACE s_axilite port=a
#pragma HLS INTERFACE s_axilite port=inv_e
#pragma HLS INTERFACE s_axilite port=sigma_sqrt_dt
#pragma HLS INTERFACE s_axilite port=seed
#pragma HLS INTERFACE s_axilite port=reseed


    static State state_a[MAX_NEURONS];
    static State state_b[MAX_NEURONS];
    static NeuronDegree out_degrees_cache[MAX_NEURONS];
    #pragma HLS BIND_STORAGE variable=state_a type=ram_2p impl=bram
    #pragma HLS BIND_STORAGE variable=state_b type=ram_2p impl=bram
    #pragma HLS BIND_STORAGE variable=out_degrees_cache type=ram_1p impl=bram
    // basically with these things in BRAM, we avoid
    // 1) random reads for state, which would make DRAM bus really slow
    // 2) We leave the DRAM bus free to fetch the (very long) sequential edge data, without intermittent halts to eftch state and/or degree

    static ap_uint<32> rng_state[GAUSS_UNIFORMS] = {
        0x12345678,
        0x9E3779B9,
        0x243F6A88,
        0xB7E15162,
        0xDEADBEEF,
        0xCAFEBABE,
        0x8BADF00D,
        0xFEEDFACE
    };
    #pragma HLS ARRAY_PARTITION variable=rng_state complete

    RESEED:
    if(reseed) {
        rng_state[0] = seed;
        rng_state[1] = seed ^ 0x9E3779B9;
        rng_state[2] = seed ^ 0x243F6A88;
        rng_state[3] = seed ^ 0xB7E15162;
        rng_state[4] = seed ^ 0xDEADBEEF;
        rng_state[5] = seed ^ 0xCAFEBABE;
        rng_state[6] = seed ^ 0x8BADF00D;
        rng_state[7] = seed ^ 0xFEEDFACE;
    }

    LOAD_BRAM:
    for (int n = 0; n < neuron_count; ++n) { // We don't wanna use != operator in for in hls
    #pragma HLS PIPELINE II=1

        StateBits bits = state_in[n];

        state_a[n].u.range(31, 0) = bits.range(31, 0);
        state_a[n].v.range(31, 0) = bits.range(63, 32);
        out_degrees_cache[n] = out_degrees_in[n];
    }

    TIMESTEP_LOOP:
    for (int t = 0; t < iteration_count; ++t) {

        // We want to swap BRAM arrays every timestep (pingpong buffering)
        if ((t & 1) == 0) {
            timestep(
                state_a,
                state_b,
                edge_list,
                out_degrees_cache,
                neuron_count,
                edge_count,
                dt,
                J,
                a,
                inv_e,
                sigma_sqrt_dt,
                rng_state
            );
        } else {
            timestep(
                state_b,
                state_a,
                edge_list,
                out_degrees_cache,
                neuron_count,
                edge_count,
                dt,
                J,
                a,
                inv_e,
                sigma_sqrt_dt,
                rng_state
            );
        }
    }

    // Write back final state to DRAM
    STORE_STATE:
    for (int n = 0; n < neuron_count; ++n) {
    #pragma HLS PIPELINE II=1

        const State s = ((iteration_count & 1) == 0) ? state_a[n] : state_b[n];

        StateBits bits = 0;
        bits.range(31, 0)  = s.u.range(31, 0);
        bits.range(63, 32) = s.v.range(31, 0);

        state_out[n] = bits;
    }
}