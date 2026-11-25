// tb.cpp

#include "accelerator.hpp"
#include "rng.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>


// ============================================================
// Packing helpers
// ============================================================

static StateBits pack_state(float u, float v)
{
    fixed_t uf = (fixed_t)u;
    fixed_t vf = (fixed_t)v;

    StateBits bits = 0;

    bits.range(31, 0)  = uf.range(31, 0);
    bits.range(63, 32) = vf.range(31, 0);

    return bits;
}


static State unpack_state(StateBits bits)
{
    State s;

    s.u.range(31, 0) = bits.range(31, 0);
    s.v.range(31, 0) = bits.range(63, 32);

    return s;
}


static EdgeBits pack_edge(
    NeuronIndex from,
    bool last
)
{
    EdgeBits bits = 0;

    bits.range(31, 0)  = from;
    bits[32]           = last;

    return bits;
}


// ============================================================
// CPU reference: deterministic RK4 update (one step)
// ============================================================

struct RefState {
    double u;
    double v;
};

struct RefDeriv {
    double du;
    double dv;
};

static RefDeriv cpu_fhn_deriv(
    const RefState& s,
    double I,
    double a,
    double inv_e
)
{
    RefDeriv d;
    d.du = (s.u - (s.u * s.u * s.u) / 3.0 - s.v + I) * inv_e;
    d.dv = s.u + a;
    return d;
}


// Plain floating-point RK4 step (independent of fixed-point hardware factoring)
static void cpu_rk4_step(
    RefState* state,
    int N,
    double dt,
    double J,
    double a,
    double inv_e
)
{
    const double dt_half  = 0.5 * dt;
    const double dt_third = dt / 3.0;
    const double dt_sixth = dt / 6.0;

    static RefDeriv k1[MAX_NEURONS];
    static RefDeriv k2[MAX_NEURONS];
    static RefDeriv k3[MAX_NEURONS];
    static RefDeriv k4[MAX_NEURONS];
    static RefState s_stage[MAX_NEURONS];
    static RefState s_orig[MAX_NEURONS];

    for (int n = 0; n < N; ++n) {
        s_orig[n] = state[n];
    }

    // Stage 0: k1 evaluated at s_orig
    for (int n = 0; n < N; ++n) {
        const int left  = (n - 1 + N) % N;
        const int right = (n + 1) % N;
        const double I = J * (s_orig[left].u + s_orig[right].u - 2.0 * s_orig[n].u);

        k1[n] = cpu_fhn_deriv(s_orig[n], I, a, inv_e);
        s_stage[n].u = s_orig[n].u + k1[n].du * dt_half;
        s_stage[n].v = s_orig[n].v + k1[n].dv * dt_half;
    }

    // Stage 1: k2 evaluated at s_stage (from k1)
    for (int n = 0; n < N; ++n) {
        const int left  = (n - 1 + N) % N;
        const int right = (n + 1) % N;
        const double I = J * (s_stage[left].u + s_stage[right].u - 2.0 * s_stage[n].u);

        k2[n] = cpu_fhn_deriv(s_stage[n], I, a, inv_e);
    }
    for (int n = 0; n < N; ++n) {
        s_stage[n].u = s_orig[n].u + k2[n].du * dt_half;
        s_stage[n].v = s_orig[n].v + k2[n].dv * dt_half;
    }

    // Stage 2: k3 evaluated at s_stage (from k2)
    for (int n = 0; n < N; ++n) {
        const int left  = (n - 1 + N) % N;
        const int right = (n + 1) % N;
        const double I = J * (s_stage[left].u + s_stage[right].u - 2.0 * s_stage[n].u);

        k3[n] = cpu_fhn_deriv(s_stage[n], I, a, inv_e);
    }
    for (int n = 0; n < N; ++n) {
        s_stage[n].u = s_orig[n].u + k3[n].du * dt;
        s_stage[n].v = s_orig[n].v + k3[n].dv * dt;
    }

    // Stage 3: k4 evaluated at s_stage (from k3)
    for (int n = 0; n < N; ++n) {
        const int left  = (n - 1 + N) % N;
        const int right = (n + 1) % N;
        const double I = J * (s_stage[left].u + s_stage[right].u - 2.0 * s_stage[n].u);

        k4[n] = cpu_fhn_deriv(s_stage[n], I, a, inv_e);
    }

    for (int n = 0; n < N; ++n) {
        state[n].u = s_orig[n].u + k1[n].du * dt_sixth + k2[n].du * dt_third + k3[n].du * dt_third + k4[n].du * dt_sixth;
        state[n].v = s_orig[n].v + k1[n].dv * dt_sixth + k2[n].dv * dt_third + k3[n].dv * dt_third + k4[n].dv * dt_sixth;
    }
}


// ============================================================
// Main test
// ============================================================

int main()
{
    //rng_test();
    //return 0;

    constexpr int N = 33;
    constexpr int E = 2 * N;

    constexpr int TOTAL_STEPS = 400;

    /*
        const fixed_t dt    = (fixed_t)0.01;
        const fixed_t J     = (fixed_t)0.0;
        const fixed_t a     = (fixed_t)1.1;
        const fixed_t inv_e = (fixed_t)10.0;
        // Disable noise for deterministic comparison
        const fixed_t sigma_sqrt_dt = (fixed_t).05;
    */

    const fixed_t dt    = (fixed_t)0.01;
    const fixed_t J     = (fixed_t)2.0;
    const fixed_t a     = (fixed_t)1.02;
    const fixed_t inv_e = (fixed_t)10.0;

    // Disable noise for deterministic comparison
    const fixed_t sigma_sqrt_dt = (fixed_t)0.0;

    const ap_uint<32> seed = 12345678;
    const bool reseed = true;


    // --------------------------------------------------------
    // Allocate test arrays
    // --------------------------------------------------------

    static StateBits state_in[2000] = {};
    static StateBits state_out[2000] = {};

    static EdgeBits edges[4000] = {};

    static NeuronIndex out_degrees[2000] = {};


    // --------------------------------------------------------
    // Initial condition
    //
    // FHN equilibrium:
    //
    // u* = -a
    // v* = u* - u*^3 / 3
    // --------------------------------------------------------

    const float a_float = (float)a;

    const float u_rest = -a_float;

    const float v_rest =
        u_rest
        - (u_rest * u_rest * u_rest) / 3.0f;


    std::cout
        << "Rest state: u = "
        << u_rest
        << ", v = "
        << v_rest
        << "\n";


    for (int n = 0; n < N; ++n) {
        state_in[n] = pack_state(
            u_rest,
            v_rest
        );

        out_degrees[n] = 2;
    }

    // Excite neuron 0
    state_in[0] = pack_state(
        1.5f,
        v_rest
    );


    // --------------------------------------------------------
    // Build bidirectional ring
    //
    // Sorted by destination:
    //
    // destination 0:
    //     N-1 -> 0
    //     1   -> 0   last=1
    //
    // destination 1:
    //     0 -> 1
    //     2 -> 1     last=1
    //
    // etc.
    // --------------------------------------------------------

    int e = 0;

    for (int to = 0; to < N; ++to) {

        const int left =
            (to - 1 + N) % N;

        const int right =
            (to + 1) % N;


        edges[e++] = pack_edge(
            left,
            false
        );

        edges[e++] = pack_edge(
            right,
            true
        );
    }


    // --------------------------------------------------------
    // Sanity-check packed edge list
    // --------------------------------------------------------

    int last_count = 0;

    std::cout << "\nEdges:\n";

    for (int i = 0; i < E; ++i) {

        NeuronIndex from =
            edges[i].range(31, 0);
            
        bool last =
            edges[i][32];

        if (last)
            last_count++;

        std::cout
            << i
            << ": "
            << from
            << " last="
            << last
            << "\n";
    }


    std::cout
        << "\nlast_count = "
        << last_count
        << ", expected "
        << N
        << "\n";


    if (last_count != N) {
        std::cerr
            << "ERROR: stream token count mismatch\n";

        return 1;
    }

    // --------------------------------------------------------
    // Build CPU reference initial state
    // --------------------------------------------------------

    RefState reference[N];
    for (int n = 0; n < N; ++n) {
        State init_s = unpack_state(state_in[n]);
        reference[n] = { (double)init_s.u, (double)init_s.v };
    }


    // --------------------------------------------------------
    // Open CSV for trajectory output (wide format for gnuplot)
    // Saved in the build folder: net-accel/build/trajectory.csv
    // --------------------------------------------------------

    const char* csv_build_path = "../../../trajectory.csv";
    const char* csv_local_path = "trajectory.csv";

    std::ofstream csv_build(csv_build_path);
    std::ofstream csv_local(csv_local_path);

    auto write_csv_header = [&](std::ostream& os) {
        os << "time";
        for (int n = 0; n < N; ++n) {
            os << ",neuron_" << n << "_u"
               << ",neuron_" << n << "_v"
               << ",neuron_" << n << "_u_ref"
               << ",neuron_" << n << "_v_ref";
        }
        os << "\n";
    };

    if (csv_build.is_open()) {
        write_csv_header(csv_build);
        csv_build << std::fixed << std::setprecision(6);
    }
    if (csv_local.is_open()) {
        write_csv_header(csv_local);
        csv_local << std::fixed << std::setprecision(6);
    }

    auto write_csv_step = [&](float t, const StateBits* hw_bits, const RefState* ref_states) {
        auto write_to = [&](std::ostream& os) {
            os << t;
            for (int n = 0; n < N; ++n) {
                State hw = unpack_state(hw_bits[n]);
                os << "," << (float)hw.u << "," << (float)hw.v
                   << "," << ref_states[n].u << "," << ref_states[n].v;
            }
            os << "\n";
        };

        if (csv_build.is_open()) write_to(csv_build);
        if (csv_local.is_open()) write_to(csv_local);
    };

    // Record initial conditions (time = 0.0)
    write_csv_step(0.0f, state_in, reference);


    // --------------------------------------------------------
    // Simulation loop
    // --------------------------------------------------------

    std::cout << "\nRunning " << TOTAL_STEPS << " steps...\n";

    double max_err_u = 0.0;
    double max_err_v = 0.0;

    const double half_dt_d = 0.5 * (double)dt;
    const double J_d       = (double)J;
    const double a_d       = (double)a;
    const double inv_e_d   = (double)inv_e;

    for (int step = 1; step <= TOTAL_STEPS; ++step) {

        // HW step (1 iteration per call to capture trajectory)
        net_accel(
            state_in,
            edges,
            out_degrees,
            state_out,
            N,
            E,
            1,
            dt,
            J,
            a,
            inv_e,
            sigma_sqrt_dt,
            seed,
            (step == 1) // reseed on first step only
        );

        // CPU floating-point reference: two half-steps of size h = dt / 2
        cpu_rk4_step(reference, N, half_dt_d, J_d, a_d, inv_e_d);
        cpu_rk4_step(reference, N, half_dt_d, J_d, a_d, inv_e_d);

        const float sim_time = step * (float)dt;

        for (int n = 0; n < N; ++n) {
            State hw = unpack_state(state_out[n]);

            const double err_u = std::fabs((double)hw.u - reference[n].u);
            const double err_v = std::fabs((double)hw.v - reference[n].v);

            if (err_u > max_err_u) max_err_u = err_u;
            if (err_v > max_err_v) max_err_v = err_v;

            // Feed output state back into input for next step
            state_in[n] = state_out[n];
        }

        write_csv_step(sim_time, state_out, reference);

        if (step % 10 == 0 || step == TOTAL_STEPS) {
            std::cout << "Step " << step << "/" << TOTAL_STEPS
                      << " completed (time = " << sim_time << " s)\n";
        }
    }

    if (csv_build.is_open()) csv_build.close();
    if (csv_local.is_open()) csv_local.close();

    std::cout << "\nTrajectory written to build/trajectory.csv ("
              << (TOTAL_STEPS + 1) << " timesteps, "
              << (1 + 4 * N) << " columns)\n";


    // --------------------------------------------------------
    // Verification Summary
    // --------------------------------------------------------

    std::cout << "\nMax error over " << TOTAL_STEPS << " steps:\n"
              << "  max |u_hw - u_ref| = " << max_err_u << "\n"
              << "  max |v_hw - v_ref| = " << max_err_v << "\n";

    // --------------------------------------------------------
    // Stiff-parameter regression test (epsilon = 0.01, |du| > 150)
    // --------------------------------------------------------

    constexpr int STIFF_STEPS = 400;
    const fixed_t stiff_dt    = (fixed_t)0.0001;
    const fixed_t stiff_J     = (fixed_t)0.2;
    const fixed_t stiff_a     = (fixed_t)1.1;
    const fixed_t stiff_inv_e = (fixed_t)100.0;

    static StateBits stiff_in[2000] = {};
    static StateBits stiff_out[2000] = {};
    RefState stiff_ref[N];

    const double stiff_a_d = (double)stiff_a;
    const double stiff_u_rest = -stiff_a_d;
    const double stiff_v_rest = stiff_u_rest - (stiff_u_rest * stiff_u_rest * stiff_u_rest) / 3.0;

    for (int n = 0; n < N; ++n) {
        stiff_in[n] = pack_state((float)stiff_u_rest, (float)stiff_v_rest);
        State init_s = unpack_state(stiff_in[n]);
        stiff_ref[n] = { (double)init_s.u, (double)init_s.v };
    }
    stiff_in[0] = pack_state(1.5f, (float)stiff_v_rest);
    {
        State init_0 = unpack_state(stiff_in[0]);
        stiff_ref[0] = { (double)init_0.u, (double)init_0.v };
    }

    std::cout << "\nRunning stiff epsilon=0.01 check (" << STIFF_STEPS
              << " steps in a single net_accel call; note that in RTL cosim "
                 "transaction 401/401 will take ~as long as the first 400 calls combined)...\n";

    net_accel(
        stiff_in,
        edges,
        out_degrees,
        stiff_out,
        N,
        E,
        STIFF_STEPS,
        stiff_dt,
        stiff_J,
        stiff_a,
        stiff_inv_e,
        (fixed_t)0.0,
        seed,
        true
    );

    const double stiff_half_dt_d = 0.5 * (double)stiff_dt;
    const double stiff_J_d       = (double)stiff_J;
    const double stiff_inv_e_d   = (double)stiff_inv_e;
    for (int s = 0; s < STIFF_STEPS * 2; ++s) {
        cpu_rk4_step(stiff_ref, N, stiff_half_dt_d, stiff_J_d, stiff_a_d, stiff_inv_e_d);
    }

    double max_stiff_err_u = 0.0;
    double max_stiff_err_v = 0.0;
    for (int n = 0; n < N; ++n) {
        State hw = unpack_state(stiff_out[n]);
        const double eu = std::fabs((double)hw.u - stiff_ref[n].u);
        const double ev = std::fabs((double)hw.v - stiff_ref[n].v);
        if (eu > max_stiff_err_u) max_stiff_err_u = eu;
        if (ev > max_stiff_err_v) max_stiff_err_v = ev;
    }

    std::cout << "\nStiff epsilon=0.01 check over " << STIFF_STEPS << " steps vs double:\n"
              << "  max |u_hw - u_dbl| = " << max_stiff_err_u << "\n"
              << "  max |v_hw - v_dbl| = " << max_stiff_err_v << "\n";

    bool passed = (max_err_u < 5e-3) && (max_err_v < 5e-3)
               && (max_stiff_err_u < 1e-2) && (max_stiff_err_v < 1e-2);

    if (passed) {
        std::cout << "\nTEST PASSED\n";
        return 0;
    } else {
        std::cout << "\nTEST FAILED\n";
        return 1;
    }
}