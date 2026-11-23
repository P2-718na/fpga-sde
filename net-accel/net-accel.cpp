#include "accelerator.hpp"
#include "rng.hpp"
#include <hls_stream.h>




// Neuron integration ////////////////////////////////////////////////////////////////

static void generate_noise(
    const ModelParams& params,
    ap_uint<32> rng_state[GAUSS_UNIFORMS],
    hls::stream<fixed_t>& noise_stream
) {
    NOISE_LOOP:
    for (int n = 0; n < params.neuron_count; ++n) {
    #pragma HLS PIPELINE II=1
        const fixed_t noise = params.sigma_sqrt_dt * gaussian_clt(rng_state);
        noise_stream.write(noise);
    }
}

static void integrate_rk_stage(
    int stage,
    hls::stream<State>& state_stream,
    const State y_0[MAX_NEURONS],
    State next_state_intermediate[MAX_NEURONS],
    State state_accumulator[MAX_NEURONS],
    hls::stream<fixed_t>& I_stream,
    const ModelParams& params
) {
#pragma HLS function_instantiate variable=stage

    NEURON_LOOP: 
    for (int n = 0; n < params.neuron_count; ++n) {
    #pragma HLS PIPELINE II=1

        const State s_eval = state_stream.read();
        const fixed_t I = I_stream.read();

        const fixed_t u = s_eval.u;
        const fixed_t v = s_eval.v;

        const fixed_t du = (u - u*u*u*(fixed_t)(1.0/3.0) - v + I) * params.inv_e;
        const fixed_t dv = u + params.a;

        switch (stage) {
            case 0: {
                State next_s;
                next_s.u = s_eval.u + du * params.h_half;
                next_s.v = s_eval.v + dv * params.h_half;
                next_state_intermediate[n] = next_s;

                State acc_s;
                acc_s.u = s_eval.u + du * params.h_sixth;
                acc_s.v = s_eval.v + dv * params.h_sixth;
                state_accumulator[n] = acc_s;
                break;
            }
            case 1: {
                const State s_base = y_0[n];
                State next_s;
                next_s.u = s_base.u + du * params.h_half;
                next_s.v = s_base.v + dv * params.h_half;
                next_state_intermediate[n] = next_s;

                State acc_s = state_accumulator[n];
                acc_s.u += du * params.h_third;
                acc_s.v += dv * params.h_third;
                state_accumulator[n] = acc_s;
                break;
            }
            case 2: default: {
                const State s_base = y_0[n];
                State next_s;
                next_s.u = s_base.u + du * params.h;
                next_s.v = s_base.v + dv * params.h;
                next_state_intermediate[n] = next_s;

                State acc_s = state_accumulator[n];
                acc_s.u += du * params.h_third;
                acc_s.v += dv * params.h_third;
                state_accumulator[n] = acc_s;
                break;
            }
        }
    }
}

static void integrate_rk_stage_final(
    hls::stream<State>& state_stream,
    State state_accumulator[MAX_NEURONS],
    hls::stream<fixed_t>& I_stream,
    hls::stream<fixed_t>& noise_stream,
    const ModelParams& params,
    bool add_noise
) {
    NEURON_LOOP: 
    for (int n = 0; n < params.neuron_count; ++n) {
    #pragma HLS PIPELINE II=1

        const State s_eval = state_stream.read();
        const fixed_t I = I_stream.read();
        const fixed_t noise = noise_stream.read();

        const fixed_t u = s_eval.u;
        const fixed_t v = s_eval.v;

        const fixed_t du = (u - u*u*u*(fixed_t)(1.0/3.0) - v + I) * params.inv_e;
        const fixed_t dv = u + params.a;

        State acc_s = state_accumulator[n];
        
        acc_s.u += du * params.h_sixth + (add_noise ? noise : (fixed_t)0);
        acc_s.v += dv * params.h_sixth;
        state_accumulator[n] = acc_s;
    }
}


static void compute_flows(
    const State state_curr[MAX_NEURONS],
    const NeuronDegree out_degrees[MAX_NEURONS],
    const EdgeBits* edge_list,
    const ModelParams& params,
    hls::stream<fixed_t>& I_stream,
    hls::stream<State>& state_stream
) {
    fixed_t incoming_sum = 0;
    int curr_neuron = 0;

    EDGE_LOOP:
    for (int e = 0; e < params.edge_count; ++e) {
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
            fixed_t I = params.J * (incoming_sum - outgoing_term);

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
    const ModelParams& params
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
        params,
        I_stream,
        state_stream
    );

    integrate_rk_stage(
        stage,
        state_stream,
        y_0,
        next_state_intermediate,
        state_accumulator,
        I_stream,
        params
    );
}

static void rk_stage_final(
    const State curr_state_intermediate[MAX_NEURONS],
    State state_accumulator[MAX_NEURONS],
    const EdgeBits* edge_list,
    const NeuronDegree out_degrees[MAX_NEURONS],
    const ModelParams& params,
    ap_uint<32> rng_state[GAUSS_UNIFORMS],
    bool add_noise
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
        params,
        I_stream,
        state_stream
    );

    generate_noise(
        params,
        rng_state,
        noise_stream
    );

    integrate_rk_stage_final(
        state_stream,
        state_accumulator,
        I_stream,
        noise_stream,
        params,
        add_noise
    );
}

// =================================================================================================================
// Zero-Copy RK4 Buffer Liveness & Routing Table:
// Stage | state_curr            | rk_buffer_a           | rk_buffer_b                       | rk_buffer_c (output)
// ------+-----------------------+-----------------------+-----------------------------------+----------------------
//   0   | Read y_0              | Write y_0 + h/2*k1    | Init  y_0 + h/6*k1                | (idle)
//   1   | Read y_0 (base)       | Read  y_0 + h/2*k1    | Acc   + h/3*k2                    | Write y_0 + h/2*k2
//   2   | Read y_0 (base)       | Write y_0 + h*k3      | Acc   + h/3*k3                    | Read  y_0 + h/2*k2
//   3   | (idle)                | Read  y_0 + h*k3      | Final + h/6*k4 + noise = y_half   | (idle)
//   4   | (idle)                | Write y_half + h/2*k1 | Read  y_half (base & eval)        | Init  y_half + h/6*k1
//   5   | Write y_half + h/2*k2 | Read  y_half + h/2*k1 | Read  y_half (base)               | Acc   + h/3*k2
//   6   | Read  y_half + h/2*k2 | Write y_half + h*k3   | Read  y_half (base)               | Acc   + h/3*k3
//   7   | (idle)                | Read  y_half + h*k3   | Read  y_half (base)               | Final + h/6*k4 = y_out
// =================================================================================================================
static void timestep(
    State state_curr[MAX_NEURONS],
    State rk_buffer_c[MAX_NEURONS],
    State rk_buffer_a[MAX_NEURONS],
    State rk_buffer_b[MAX_NEURONS],
    const EdgeBits* edge_list,
    const NeuronDegree out_degrees[MAX_NEURONS],
    const ModelParams& params,
    ap_uint<32> rng_state[GAUSS_UNIFORMS]
) {
    // -------------------------------------------------------------------------
    // Step 1: First RK4 half-step (step size h), with noise added at stage 3.
    // Base is state_curr; accumulated directly into rk_buffer_b (holds y_half).
    // -------------------------------------------------------------------------

    // Stage 0: y_0 = state_curr, eval state_curr, write next to rk_buffer_a, init rk_buffer_b
    rk_stage_intermediate(
        0,
        state_curr,          // y_0
        state_curr,          // curr_state_intermediate
        rk_buffer_a,         // next_state_intermediate
        rk_buffer_b,         // state_accumulator (holding y_half)
        edge_list,
        out_degrees,
        params
    );

    // Stage 1: y_0 = state_curr, eval rk_buffer_a, write next to rk_buffer_c, acc to rk_buffer_b
    rk_stage_intermediate(
        1,
        state_curr,          // y_0
        rk_buffer_a,         // curr_state_intermediate
        rk_buffer_c,         // next_state_intermediate
        rk_buffer_b,         // state_accumulator (holding y_half)
        edge_list,
        out_degrees,
        params
    );

    // Stage 2: y_0 = state_curr, eval rk_buffer_c, write next to rk_buffer_a, acc to rk_buffer_b
    rk_stage_intermediate(
        2,
        state_curr,          // y_0
        rk_buffer_c,         // curr_state_intermediate
        rk_buffer_a,         // next_state_intermediate
        rk_buffer_b,         // state_accumulator (holding y_half)
        edge_list,
        out_degrees,
        params
    );

    // Stage 3: eval rk_buffer_a, add noise, finalize rk_buffer_b -> holds y_half
    rk_stage_final(
        rk_buffer_a,         // curr_state_intermediate
        rk_buffer_b,         // state_accumulator (holding y_half)
        edge_list,
        out_degrees,
        params,
        rng_state,
        true                 // add_noise = true
    );

    // -------------------------------------------------------------------------
    // Step 2: Second RK4 half-step (step size h), without noise.
    // Base is rk_buffer_b (y_half); accumulated directly into rk_buffer_c (final).
    // state_curr is dead after Step 1, so it is reused as intermediate scratchpad.
    // -------------------------------------------------------------------------

    // Stage 4 (0): y_0 = rk_buffer_b, eval rk_buffer_b, write next to rk_buffer_a, init rk_buffer_c
    rk_stage_intermediate(
        0,
        rk_buffer_b,         // y_0 (y_half)
        rk_buffer_b,         // curr_state_intermediate (y_half)
        rk_buffer_a,         // next_state_intermediate
        rk_buffer_c,         // state_accumulator (final output)
        edge_list,
        out_degrees,
        params
    );

    // Stage 5 (1): y_0 = rk_buffer_b, eval rk_buffer_a, write next to state_curr, acc to rk_buffer_c
    rk_stage_intermediate(
        1,
        rk_buffer_b,         // y_0 (y_half)
        rk_buffer_a,         // curr_state_intermediate
        state_curr,          // next_state_intermediate (scratch)
        rk_buffer_c,         // state_accumulator (final output)
        edge_list,
        out_degrees,
        params
    );

    // Stage 6 (2): y_0 = rk_buffer_b, eval state_curr, write next to rk_buffer_a, acc to rk_buffer_c
    rk_stage_intermediate(
        2,
        rk_buffer_b,         // y_0 (y_half)
        state_curr,          // curr_state_intermediate
        rk_buffer_a,         // next_state_intermediate
        rk_buffer_c,         // state_accumulator (final output)
        edge_list,
        out_degrees,
        params
    );

    // Stage 7 (3): eval rk_buffer_a, finalize rk_buffer_c, without noise
    rk_stage_final(
        rk_buffer_a,         // curr_state_intermediate
        rk_buffer_c,         // state_accumulator (final output)
        edge_list,
        out_degrees,
        params,
        rng_state,
        false                // add_noise = false
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

    const fixed_t h = dt * (fixed_t)0.5;

    ModelParams params;
    params.neuron_count  = neuron_count;
    params.edge_count    = edge_count;
    params.h             = h;
    params.h_half        = h * (fixed_t)0.5;
    params.h_third       = h * (fixed_t)(1.0 / 3.0);
    params.h_sixth       = h * (fixed_t)(1.0 / 6.0);
    params.J             = J;
    params.a             = a;
    params.inv_e         = inv_e;
    params.sigma_sqrt_dt = sigma_sqrt_dt;

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
                params,
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
                params,
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