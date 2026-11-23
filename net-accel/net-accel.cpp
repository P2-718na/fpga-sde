#include "accelerator.hpp"
#include "rng.hpp"
#include <hls_stream.h>




// Neuron integration ////////////////////////////////////////////////////////////////

static void generate_noise(
    int neuron_count,
    fixed_t sigma_sqrt_dt,
    ap_uint<32> rng_state[GAUSS_UNIFORMS],
    hls::stream<fixed_t>& noise_stream
) {
    NOISE_LOOP:
    for (int n = 0; n < neuron_count; ++n) {
    #pragma HLS PIPELINE II=1
        const fixed_t noise = sigma_sqrt_dt * gaussian_clt(rng_state);
        noise_stream.write(noise);
    }
}

static void integrate_rk_stage(
    int stage,
    hls::stream<State>& state_stream,
    const State y_0[MAX_NEURONS],
    State next_state_intermediate[MAX_NEURONS],
    State state_accumulator[MAX_NEURONS],
    int neuron_count,
    hls::stream<fixed_t>& I_stream,
    fixed_t dt,
    fixed_t dt_half,
    fixed_t dt_third,
    fixed_t dt_sixth,
    fixed_t a,
    fixed_t inv_e
) {
#pragma HLS function_instantiate variable=stage

    NEURON_LOOP: 
    for (int n = 0; n < neuron_count; ++n) {
    #pragma HLS PIPELINE II=1

        const State s_eval = state_stream.read();
        const fixed_t I = I_stream.read();

        const fixed_t u = s_eval.u;
        const fixed_t v = s_eval.v;

        const fixed_t du = (u - u*u*u*(fixed_t)(1.0/3.0) - v + I) * inv_e;
        const fixed_t dv = u + a;

        switch (stage) {
            case 0: {
                State next_s;
                next_s.u = s_eval.u + du * dt_half;
                next_s.v = s_eval.v + dv * dt_half;
                next_state_intermediate[n] = next_s;

                State acc_s;
                acc_s.u = s_eval.u + du * dt_sixth;
                acc_s.v = s_eval.v + dv * dt_sixth;
                state_accumulator[n] = acc_s;
                break;
            }
            case 1: {
                const State s_base = y_0[n];
                State next_s;
                next_s.u = s_base.u + du * dt_half;
                next_s.v = s_base.v + dv * dt_half;
                next_state_intermediate[n] = next_s;

                State acc_s = state_accumulator[n];
                acc_s.u += du * dt_third;
                acc_s.v += dv * dt_third;
                state_accumulator[n] = acc_s;
                break;
            }
            case 2: default: {
                const State s_base = y_0[n];
                State next_s;
                next_s.u = s_base.u + du * dt;
                next_s.v = s_base.v + dv * dt;
                next_state_intermediate[n] = next_s;

                State acc_s = state_accumulator[n];
                acc_s.u += du * dt_third;
                acc_s.v += dv * dt_third;
                state_accumulator[n] = acc_s;
                break;
            }
        }
    }
}

static void integrate_rk_stage_final(
    hls::stream<State>& state_stream,
    State state_accumulator[MAX_NEURONS],
    int neuron_count,
    hls::stream<fixed_t>& I_stream,
    hls::stream<fixed_t>& noise_stream,
    fixed_t dt_sixth,
    fixed_t a,
    fixed_t inv_e
) {
    NEURON_LOOP: 
    for (int n = 0; n < neuron_count; ++n) {
    #pragma HLS PIPELINE II=1

        const State s_eval = state_stream.read();
        const fixed_t I = I_stream.read();
        const fixed_t noise = noise_stream.read();

        const fixed_t u = s_eval.u;
        const fixed_t v = s_eval.v;

        const fixed_t du = (u - u*u*u*(fixed_t)(1.0/3.0) - v + I) * inv_e;
        const fixed_t dv = u + a;

        State acc_s = state_accumulator[n];
        acc_s.u += du * dt_sixth + noise;
        acc_s.v += dv * dt_sixth;
        state_accumulator[n] = acc_s;
    }
}


static void compute_flows(
    const State state_curr[MAX_NEURONS],
    const NeuronDegree out_degrees[MAX_NEURONS],
    const EdgeBits* edge_list,
    int edge_count,
    fixed_t J,
    hls::stream<fixed_t>& I_stream,
    hls::stream<State>& state_stream
) {
    fixed_t incoming_sum = 0;
    int curr_neuron = 0;

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
            State s = state_curr[curr_neuron];
            fixed_t outgoing_term = (fixed_t)out_degrees[curr_neuron] * s.u;
            fixed_t I = J * (incoming_sum - outgoing_term);

            // Produce EXACTLY 1 I and 1 State per neuron in lockstep
            I_stream.write(I);
            state_stream.write(s);

            incoming_sum = 0;
            curr_neuron++;
        }
    }
}


static void rk_stage_intermediate(
    int stage,
    const State y_0[MAX_NEURONS],
    const State curr_state_intermediate[MAX_NEURONS],
    State next_state_intermediate[MAX_NEURONS],
    State state_accumulator[MAX_NEURONS],
    const EdgeBits* edge_list,
    const NeuronDegree out_degrees[MAX_NEURONS],
    int neuron_count,
    int edge_count,
    fixed_t dt,
    fixed_t dt_half,
    fixed_t dt_third,
    fixed_t dt_sixth,
    fixed_t J,
    fixed_t a,
    fixed_t inv_e
) {
// function_instantiate instantiates a separate hardware circuit for each possible value of the variable "stage".
// Avoids vitis having to create a multiplexer for each one of the different input arrays (we are swapping them around in between steps) that are fed to the fn
// this reduces the critical timing path and speeds things up.
#pragma HLS function_instantiate variable=stage
#pragma HLS DATAFLOW

    hls::stream<State> state_stream;
    hls::stream<fixed_t> I_stream;

    #pragma HLS STREAM variable=state_stream depth=16
    #pragma HLS STREAM variable=I_stream depth=16

    compute_flows(
        curr_state_intermediate,
        out_degrees,
        edge_list,
        edge_count,
        J,
        I_stream,
        state_stream
    );

    integrate_rk_stage(
        stage,
        state_stream,
        y_0,
        next_state_intermediate,
        state_accumulator,
        neuron_count,
        I_stream,
        dt,
        dt_half,
        dt_third,
        dt_sixth,
        a,
        inv_e
    );
}

static void rk_stage_final(
    const State curr_state_intermediate[MAX_NEURONS],
    State state_accumulator[MAX_NEURONS],
    const EdgeBits* edge_list,
    const NeuronDegree out_degrees[MAX_NEURONS],
    int neuron_count,
    int edge_count,
    fixed_t dt_sixth,
    fixed_t J,
    fixed_t a,
    fixed_t inv_e,
    fixed_t sigma_sqrt_dt,
    ap_uint<32> rng_state[GAUSS_UNIFORMS]
) {
#pragma HLS DATAFLOW

    hls::stream<State> state_stream;
    hls::stream<fixed_t> I_stream;
    hls::stream<fixed_t> noise_stream;

    #pragma HLS STREAM variable=state_stream depth=16
    #pragma HLS STREAM variable=I_stream depth=16
    #pragma HLS STREAM variable=noise_stream depth=16

    compute_flows(
        curr_state_intermediate,
        out_degrees,
        edge_list,
        edge_count,
        J,
        I_stream,
        state_stream
    );

    generate_noise(
        neuron_count,
        sigma_sqrt_dt,
        rng_state,
        noise_stream
    );

    integrate_rk_stage_final(
        state_stream,
        state_accumulator,
        neuron_count,
        I_stream,
        noise_stream,
        dt_sixth,
        a,
        inv_e
    );
}

static void timestep(
    const State state_curr[MAX_NEURONS],
    State state_accumulator[MAX_NEURONS],
    State rk_buffer_a[MAX_NEURONS],
    State rk_buffer_b[MAX_NEURONS],
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
    const fixed_t dt_half  = dt * (fixed_t)0.5;
    const fixed_t dt_third = dt * (fixed_t)(1.0 / 3.0);
    const fixed_t dt_sixth = dt * (fixed_t)(1.0 / 6.0);

    // Stage 0: y_0 = state_curr, evaluate state_curr, write rk_buffer_a, init state_accumulator
    rk_stage_intermediate(
        0,
        state_curr,          // y_0
        state_curr,          // curr_state_intermediate
        rk_buffer_a,         // next_state_intermediate
        state_accumulator,   // state_accumulator
        edge_list,
        out_degrees,
        neuron_count,
        edge_count,
        dt,
        dt_half,
        dt_third,
        dt_sixth,
        J,
        a,
        inv_e
    );

    // Stage 1: y_0 = state_curr, evaluate rk_buffer_a, write rk_buffer_b, accumulate into state_accumulator
    rk_stage_intermediate(
        1,
        state_curr,          // y_0
        rk_buffer_a,         // curr_state_intermediate
        rk_buffer_b,         // next_state_intermediate
        state_accumulator,   // state_accumulator
        edge_list,
        out_degrees,
        neuron_count,
        edge_count,
        dt,
        dt_half,
        dt_third,
        dt_sixth,
        J,
        a,
        inv_e
    );

    // Stage 2: y_0 = state_curr, evaluate rk_buffer_b, write rk_buffer_a, accumulate into state_accumulator
    rk_stage_intermediate(
        2,
        state_curr,          // y_0
        rk_buffer_b,         // curr_state_intermediate
        rk_buffer_a,         // next_state_intermediate
        state_accumulator,   // state_accumulator
        edge_list,
        out_degrees,
        neuron_count,
        edge_count,
        dt,
        dt_half,
        dt_third,
        dt_sixth,
        J,
        a,
        inv_e
    );

    // Stage 3: evaluate rk_buffer_a, generate noise in parallel, finalize state_accumulator
    rk_stage_final(
        rk_buffer_a,         // curr_state_intermediate
        state_accumulator,   // state_accumulator
        edge_list,
        out_degrees,
        neuron_count,
        edge_count,
        dt_sixth,
        J,
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
    static State rk_buffer_a[MAX_NEURONS];
    static State rk_buffer_b[MAX_NEURONS];
    static NeuronDegree out_degrees_cache[MAX_NEURONS];
    #pragma HLS BIND_STORAGE variable=state_a type=ram_t2p impl=bram
    #pragma HLS BIND_STORAGE variable=state_b type=ram_t2p impl=bram
    #pragma HLS BIND_STORAGE variable=rk_buffer_a type=ram_t2p impl=bram
    #pragma HLS BIND_STORAGE variable=rk_buffer_b type=ram_t2p impl=bram
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
                rk_buffer_a,
                rk_buffer_b,
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
                rk_buffer_a,
                rk_buffer_b,
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