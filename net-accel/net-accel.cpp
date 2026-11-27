#include "accelerator.hpp"
#include "rng.hpp"

#ifndef __SYNTHESIS__
#include <cassert>
#include <cstdlib>
#include <iostream>


// TESTING ///////////////////////////////////////////////////////////////////////////////////
static void check_fixed_range(const char* loc, const char* name, double val) {
    if (val < -128.0 || val >= 128.0) {
        std::cerr << "OVERFLOW in " << loc << " (" << name << "): " << val
                  << " outside [-128, 128)\n";
        assert(false && "fixed_t overflow");
        std::abort();
    }
}
#endif
// TESTING ///////////////////////////////////////////////////////////////////////////////////


static fixed_t compute_du(
    fixed_t u,
    fixed_t v,
    fixed_t I
) {
#pragma HLS INLINE

    // Additional overflow check.
#ifndef __SYNTHESIS__
    const double ud   = static_cast<double>(u);
    const double vd   = static_cast<double>(v);
    const double Id   = static_cast<double>(I);
    const double u2d  = ud * ud;
    const double u3d  = (u2d * ud) / 3.0;
    const double rhsd = ud - u3d - vd + Id;
    check_fixed_range("compute_du", "u^2", u2d);
    check_fixed_range("compute_du", "u^3/3", u3d);
    check_fixed_range("compute_du", "du", rhsd);
#endif

    // Split u^3 / 3 computation to save DSPs; inv_e is factored into dt_over_e
    // to prevent intermediate Q8.24 overflow when epsilon is small.
    static const fixed_t INV_3 = 1.0 / 3.0;
    const fixed_t u2   = u * u;
    const fixed_t u_3  = u * INV_3;
    const fixed_t u3_3 = u2 * u_3;
    return u - u3_3 - v + I;
}

static void run_rk_stage(
    WorkBuffers& mem,
    const EdgeBits* edge_list,
    const StageConfig& cfg,
    Bank base_bank,
    Bank acc_bank,
    bool add_noise,
    const ModelParams& params,
    ap_uint<32> rng_state[GAUSS_UNIFORMS]
) {
// Force vitis to disable duplication
// So same hardware is reused across stages
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    double incoming_sum_d = 0.0;
#endif

    fixed_t incoming_sum = 0;
    int curr_neuron = 0;

    EDGE_LOOP:
    for (int e = 0; e < params.edge_count; ++e) {
    #pragma HLS PIPELINE II=1
    #pragma HLS DEPENDENCE variable=mem.state inter false
    #pragma HLS DEPENDENCE variable=mem.rk_u inter false
    #pragma HLS DEPENDENCE variable=mem.rk_eval inter false

        // Parse packed edge format
        EdgeBits bits = edge_list[e];
        uint32_t from = bits.range(31, 0);
        bool last     = bits[32];

        const fixed_t u_from = mem.rk_u[cfg.eval_u_bank][from];

#ifndef __SYNTHESIS__
        incoming_sum_d += static_cast<double>(u_from);
        check_fixed_range("run_rk_stage", "incoming_sum", incoming_sum_d);
#endif

        incoming_sum += u_from;

        // EVERY Neuron MUST have a last edge. We canforce this by making every neuron connected to itself.
        // The interaction terms of it with itself cancel out in this branch...
        if (last) {
            // Force a read to the two separate banks at once.
            // If we were to access the bram using directly the base_bank or acc_bank function parameters,
            // Vitis could assume that they both refer to the same bank, and that two different circuits are reading from the same bank at the same time (not possible in the same clock cycle).
            // In that case, vitis prefers setting II=2 and allowing itself to only read from the real brams instead of automatically forcing a read on all brams and fanout the value.
            // By adding the intermediate vars, we are explicitly forcing it to read from both and do the thing.
            // This wastes a bit of power.
            const State s0     = mem.state[BANK_A][curr_neuron];
            const State s1     = mem.state[BANK_B][curr_neuron];
            const State s_base = (base_bank == BANK_A) ? s0 : s1;
            const State s_acc  = (acc_bank  == BANK_A) ? s0 : s1;
            const State s_eval = cfg.is_init ? s_base : mem.rk_eval[curr_neuron];

#ifndef __SYNTHESIS__
            const double deg_d      = static_cast<double>(static_cast<uint32_t>(mem.out_deg[curr_neuron]));
            const double outgoing_d = deg_d * static_cast<double>(s_eval.u);
            const double diff_d     = incoming_sum_d - outgoing_d;
            check_fixed_range("run_rk_stage", "out_degree", deg_d);
            check_fixed_range("run_rk_stage", "outgoing_term", outgoing_d);
            check_fixed_range("run_rk_stage", "incoming_sum - outgoing_term", diff_d);
            check_fixed_range("run_rk_stage", "I", static_cast<double>(params.J) * diff_d);
#endif
        
            const fixed_t outgoing_term = (fixed_t)mem.out_deg[curr_neuron] * s_eval.u;
            const fixed_t I  = params.J * (incoming_sum - outgoing_term);
            const fixed_t du = compute_du(s_eval.u, s_eval.v, I);
            const fixed_t dv = s_eval.u + params.a;

            fixed_t noise = 0;
            if (add_noise) {
                noise = params.sigma_sqrt_dt * gaussian_clt(rng_state);
            }

            State next_s;
            next_s.u = s_base.u + du * cfg.a_u;
            next_s.v = s_base.v + dv * cfg.a_v;

            // decide wether to start accumulating from y_0 or from the latest accumulator
            const State acc_base = cfg.is_init ? s_base : s_acc;
            State acc_next;
            acc_next.u = acc_base.u + du * cfg.b_u + noise;
            acc_next.v = acc_base.v + dv * cfg.b_v;

            mem.state[acc_bank][curr_neuron]       = acc_next;
            // On stage 3 (is_final), mirror acc_next.u into rk_u[0] ready for stage 0 of the next half-step.
            mem.rk_u[cfg.next_u_bank][curr_neuron] = cfg.is_final ? acc_next.u : next_s.u;
            if (!cfg.is_final) {
                mem.rk_eval[curr_neuron] = next_s;
            }

            incoming_sum = 0;
            curr_neuron++;

#ifndef __SYNTHESIS__
            incoming_sum_d = 0.0;
#endif
        }
    }
}

static void integrate(
    WorkBuffers& mem,
    const EdgeBits* edge_list,
    const ModelParams& params,
    int iteration_count,
    fixed_t dt,
    fixed_t dt_over_e,
    ap_uint<32> rng_state[GAUSS_UNIFORMS]
) {
#pragma HLS INLINE

    // Half-step (h = dt / 2) Butcher tableau weights precomputed once per call
    // We kinda have to do this fixed_t inverse conversion cast multply thingy otherwise vitis will slap a DSP onto that
    const fixed_t h         = dt        * (fixed_t)0.5;
    const fixed_t h_over_2  = dt        * (fixed_t)0.25;
    const fixed_t h_over_3  = dt        * (fixed_t)(1.0 / 6.0);
    const fixed_t h_over_6  = dt        * (fixed_t)(1.0 / 12.0);
    const fixed_t h_over_e  = dt_over_e * (fixed_t)0.5;
    const fixed_t h_over_2e = dt_over_e * (fixed_t)0.25;
    const fixed_t h_over_3e = dt_over_e * (fixed_t)(1.0 / 6.0);
    const fixed_t h_over_6e = dt_over_e * (fixed_t)(1.0 / 12.0);

    // 4-stage RK4 Butcher tableau + rk_u ping-pong bank schedule
    const StageConfig tableau[4] = {
        //   u_in, u_out, init, final, butcher_a_u, butcher_a_v,  butcher_b_u, butcher_b_v
        { BANK_A, BANK_B, true,  false, h_over_2e, h_over_2, h_over_6e, h_over_6 }, // k1
        { BANK_B, BANK_A, false, false, h_over_2e, h_over_2, h_over_3e, h_over_3 }, // k2
        { BANK_A, BANK_B, false, false, h_over_e,  h,        h_over_3e, h_over_3 }, // k3
        { BANK_B, BANK_A, false, true,  0,         0,        h_over_6e, h_over_6 }, // k4
    };
    // Have to access all in the same clock
    #pragma HLS ARRAY_PARTITION variable=tableau complete

TIMESTEP_LOOP:
    for (int step = 0; step < iteration_count; ++step) {
        // TODO look into "Strang-Split"
        // First half step  writes state[BANK_A]->state[BANK_B] (+ noise)
        // Second half step writes state[BANK_B]->state[BANK_A]
        HALF_STEP_LOOP:
        for (int half = 0; half < 2; ++half) {
            const Bank base_bank = (half == 0) ? BANK_A : BANK_B;
            const Bank acc_bank  = (half == 0) ? BANK_B : BANK_A;

            STAGE_LOOP:
            for (int stage = 0; stage < 4; ++stage) {
                const bool add_noise = (half == 0) && tableau[stage].is_final;
                run_rk_stage(
                    mem,
                    edge_list,
                    tableau[stage],
                    base_bank,
                    acc_bank,
                    add_noise,
                    params,
                    rng_state
                );
            }
        }
    }
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
    fixed_t dt_over_e,
    fixed_t sigma_sqrt_dt,
    ap_uint<32> seed,
    bool reseed
) {
// HP Port connections
// depth pragmas are useful only in RTL simulation
// TODO check that this is the best configuration for the HP ports
#pragma HLS INTERFACE m_axi port=edge_list        offset=slave bundle=gmem0 depth=4000 // This has to have its own HP port (most critical)
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
#pragma HLS INTERFACE s_axilite port=dt_over_e
#pragma HLS INTERFACE s_axilite port=sigma_sqrt_dt
#pragma HLS INTERFACE s_axilite port=seed
#pragma HLS INTERFACE s_axilite port=reseed

    // Of course static to tell vitis that BRAM is fixed hardware
    // And to not crash csim for stack overflow.
    // Basically with these things in BRAM, we avoid
    // 1) random reads for state, which would make DRAM bus really slow
    // 2) We leave the DRAM bus free to fetch the (very long) sequential edge data, without intermittent halts to eftch state and/or degree
    // 3) Lutram would have zero latency, but it would fit way less.
    static WorkBuffers mem;
    // Separate banks so they can be read simultaneously
    #pragma HLS ARRAY_PARTITION variable=mem.state dim=1 complete
    #pragma HLS ARRAY_PARTITION variable=mem.rk_u  dim=1 complete
    #pragma HLS BIND_STORAGE variable=mem.state   type=ram_2p impl=bram
    #pragma HLS BIND_STORAGE variable=mem.rk_u    type=ram_2p impl=bram
    #pragma HLS BIND_STORAGE variable=mem.rk_eval type=ram_2p impl=bram
    #pragma HLS BIND_STORAGE variable=mem.out_deg type=ram_1p impl=bram

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
    if (reseed) {
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
        fixed_t u_in, v_in;
        u_in.range(31, 0) = bits.range(31, 0);
        v_in.range(31, 0) = bits.range(63, 32);

        // Always load into BANK_A since rk4 takes an even number of steps.
        mem.state[BANK_A][n] = { u_in, v_in };
        mem.rk_u[BANK_A][n]  = u_in;
        mem.out_deg[n]       = out_degrees_in[n];
    }

INTEGRATE:
    const ModelParams params = { neuron_count, edge_count, J, a, sigma_sqrt_dt };
    integrate(mem, edge_list, params, iteration_count, dt, dt_over_e, rng_state);

STORE_STATE:
    // Write back final state to DRAM (2 half-steps per iteration always land in mem.state[BANK_A])
    for (int n = 0; n < neuron_count; ++n) {
    #pragma HLS PIPELINE II=1
        // Always read from BANK_A since rk4 takes an even number of steps
        // The ping-pong approach of swapping banks did not work anyways.
        const State s = mem.state[BANK_A][n];

        StateBits bits = 0;
        bits.range(31, 0)  = s.u.range(31, 0);
        bits.range(63, 32) = s.v.range(31, 0);

        state_out[n] = bits;
    }
}
