#include "accelerator.hpp"
#include <hls_stream.h>
#include <iostream>

/////////////////////////////////////////////////////////////////////
// RNG

// Gotta use ap_uint so that we can call the .range function
static ap_uint<32> xorshift32(ap_uint<32>& state) {
#pragma HLS INLINE

    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;

    return state;
}

// Uniform generator between 0 and
static fixed_t uniform01(ap_uint<32>& state)
{
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

    for(int i = 0; i < GAUSS_UNIFORMS; ++i) {
    #pragma HLS UNROLL
        sum += uniform01(rng_state[i]);
    }

    // TODO maybe properly check gen scales if they make sense
    return (sum - (fixed_t)(GAUSS_UNIFORMS/2)) * CLT_SCALE;
}



////////////////////////////////////////////////////////////////
// Neuron integration
static void integrate_one(
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
    const State state_cur[MAX_NEURONS],
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

        const fixed_t I = I_stream.read();

        integrate_one(
            state_cur[n],
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

static void compute_outgoing(
    const State state_cur[MAX_NEURONS],
    const NeuronIndex out_degrees[MAX_NEURONS],
    int neuron_count,
    hls::stream<fixed_t>& out_stream
) {
NEURON_LOOP:
    for (int n = 0; n < neuron_count; ++n) {
#pragma HLS PIPELINE II=1

        fixed_t term =
            (fixed_t)out_degrees[n] * state_cur[n].u;

        out_stream.write(term);
    }
}

static void compute_flows(
    const State state_cur[MAX_NEURONS],
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

        EdgeBits bits = edge_list[e];

        uint32_t from    = bits.range(31, 0);
        bool last        = bits[32];

        incoming_sum += state_cur[from].u;

        if (last) {
            fixed_t outgoing_term = out_stream.read();
            fixed_t I =  J * (incoming_sum - outgoing_term);

            I_stream.write(I);

            incoming_sum = 0;
        }
    }
}


static void timestep(
    const State state_cur[MAX_NEURONS],
    State state_next[MAX_NEURONS],
    const EdgeBits* edge_list,
    const NeuronIndex out_degrees[MAX_NEURONS],
    int neuron_count,
    int edge_count,
    fixed_t dt,
    fixed_t J,
    fixed_t a,
    fixed_t inv_e,
    fixed_t sigma_sqrt_dt,
    ap_uint<32> rng_state[GAUSS_UNIFORMS]
) {
#pragma HLS DATAFLOW

    hls::stream<fixed_t> I_stream;
    #pragma HLS STREAM variable=I_stream depth=16
    hls::stream<fixed_t> out_stream;
    #pragma HLS STREAM variable=out_stream depth=16

    compute_outgoing(
        state_cur,
        out_degrees,
        neuron_count,
        out_stream
    );

    compute_flows(
        state_cur,
        edge_list,
        edge_count,
        J,
        out_stream,
        I_stream
    );

    integrate_all(
        state_cur,
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
    const NeuronIndex* out_degrees_in,
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
#pragma HLS INTERFACE m_axi port=edge_list   offset=slave bundle=gmem0 depth=4000// This has to have its own HP port (most critical)
#pragma HLS INTERFACE m_axi port=state_in    offset=slave bundle=gmem1 depth=2000
#pragma HLS INTERFACE m_axi port=state_out   offset=slave bundle=gmem1 depth=2000
#pragma HLS INTERFACE m_axi port=out_degrees_in offset=slave bundle=gmem2 depth=2000 // different port than state in/out because has different width. This way we allow bursting

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
    static NeuronIndex out_degrees_cache[MAX_NEURONS];
#pragma HLS BIND_STORAGE variable=state_a type=ram_2p impl=bram
#pragma HLS BIND_STORAGE variable=state_b type=ram_2p impl=bram
#pragma HLS BIND_STORAGE variable=out_degrees_cache type=ram_1p impl=bram
// basically with this shit in bram, we avoid the overhead of random reads, which would fuck everthing up

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

    LOAD_STATE:
    for (int n = 0; n < neuron_count; ++n) {
#pragma HLS PIPELINE II=1

        StateBits bits = state_in[n];

        state_a[n].u.range(31, 0) =
            bits.range(31, 0);

        state_a[n].v.range(31, 0) =
            bits.range(63, 32);

        out_degrees_cache[n] = out_degrees_in[n];
    }

    // --------------------------------------------------------
    // Timesteps
    // --------------------------------------------------------

TIMESTEP_LOOP:
    for (int t = 0; t < iteration_count; ++t) {

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
        }
        else {
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


    // --------------------------------------------------------
    // Write final state
    // --------------------------------------------------------

STORE_STATE:
    for (int n = 0; n < neuron_count; ++n) {
#pragma HLS PIPELINE II=1

        State s;

        if ((iteration_count & 1) == 0) {
            s = state_a[n];
        }
        else {
            s = state_b[n];
        }

        StateBits bits = 0;

        bits.range(31, 0)  = s.u.range(31, 0);
        bits.range(63, 32) = s.v.range(31, 0);

        state_out[n] = bits;
    }
}