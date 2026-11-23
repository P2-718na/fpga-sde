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

struct Deriv {
    fixed_t du;
    fixed_t dv;
};

static Deriv cpu_fhn_deriv(
    const State& s,
    fixed_t I,
    fixed_t a,
    fixed_t inv_e
)
{
    Deriv d;
    const fixed_t u = s.u;
    const fixed_t v = s.v;

    d.du = (u - u*u*u*(fixed_t)(1.0/3.0) - v + I) * inv_e;
    d.dv = u + a;

    return d;
}


static void cpu_rk4_step(
    State* state,
    int N,
    fixed_t dt,
    fixed_t J,
    fixed_t a,
    fixed_t inv_e
)
{
    const fixed_t dt_half  = dt * (fixed_t)0.5;
    const fixed_t dt_third = dt * (fixed_t)(1.0 / 3.0);
    const fixed_t dt_sixth = dt * (fixed_t)(1.0 / 6.0);

    static Deriv k1[MAX_NEURONS];
    static Deriv k2[MAX_NEURONS];
    static Deriv k3[MAX_NEURONS];
    static Deriv k4[MAX_NEURONS];
    static State s_stage[MAX_NEURONS];
    static State s_orig[MAX_NEURONS];

    for (int n = 0; n < N; ++n) {
        s_orig[n] = state[n];
    }

    // Stage 0: k1 evaluated at s_orig
    for (int n = 0; n < N; ++n) {
        const int left  = (n - 1 + N) % N;
        const int right = (n + 1) % N;
        fixed_t incoming_sum  = s_orig[left].u + s_orig[right].u;
        fixed_t outgoing_term = (fixed_t)2 * s_orig[n].u;
        fixed_t I = J * (incoming_sum - outgoing_term);

        k1[n] = cpu_fhn_deriv(s_orig[n], I, a, inv_e);
        s_stage[n].u = s_orig[n].u + k1[n].du * dt_half;
        s_stage[n].v = s_orig[n].v + k1[n].dv * dt_half;
    }

    // Stage 1: k2 evaluated at s_stage (from k1)
    for (int n = 0; n < N; ++n) {
        const int left  = (n - 1 + N) % N;
        const int right = (n + 1) % N;
        fixed_t incoming_sum  = s_stage[left].u + s_stage[right].u;
        fixed_t outgoing_term = (fixed_t)2 * s_stage[n].u;
        fixed_t I = J * (incoming_sum - outgoing_term);

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
        fixed_t incoming_sum  = s_stage[left].u + s_stage[right].u;
        fixed_t outgoing_term = (fixed_t)2 * s_stage[n].u;
        fixed_t I = J * (incoming_sum - outgoing_term);

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
        fixed_t incoming_sum  = s_stage[left].u + s_stage[right].u;
        fixed_t outgoing_term = (fixed_t)2 * s_stage[n].u;
        fixed_t I = J * (incoming_sum - outgoing_term);

        k4[n] = cpu_fhn_deriv(s_stage[n], I, a, inv_e);
    }

    // Final update (matching fixed-point accumulation in HW)
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

    constexpr int N = 17;
    constexpr int E = 2 * N;

    constexpr int TOTAL_STEPS = 5000;

    const fixed_t dt    = (fixed_t)0.001;
    const fixed_t J     = (fixed_t)0.1;
    const fixed_t a     = (fixed_t)1.3;
    const fixed_t inv_e = (fixed_t)10.0;

    // Disable noise for deterministic comparison
    const fixed_t sigma_sqrt_dt = (fixed_t)0.0;

    const ap_uint<32> seed = 12345678;
    const bool reseed = true;


    // --------------------------------------------------------
    // Allocate test arrays
    // --------------------------------------------------------

    StateBits state_in[2000];
    StateBits state_out[2000];

    EdgeBits edges[4000];

    NeuronIndex out_degrees[2000];


    // --------------------------------------------------------
    // Initial condition
    //
    // FHN equilibrium:
    //
    // u* = -a
    // v* = u* - u*^3 / 3
    // --------------------------------------------------------

    const float a_float = 1.3f;

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

    State reference[N];
    for (int n = 0; n < N; ++n) {
        reference[n] = unpack_state(state_in[n]);
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

    auto write_csv_step = [&](float t, const StateBits* hw_bits, const State* ref_states) {
        auto write_to = [&](std::ostream& os) {
            os << t;
            for (int n = 0; n < N; ++n) {
                State hw = unpack_state(hw_bits[n]);
                os << "," << (float)hw.u << "," << (float)hw.v
                   << "," << (float)ref_states[n].u << "," << (float)ref_states[n].v;
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

    float max_err_u = 0.0f;
    float max_err_v = 0.0f;

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

        // CPU reference step
        cpu_rk4_step(reference, N, dt, J, a, inv_e);

        const float sim_time = step * (float)dt;

        for (int n = 0; n < N; ++n) {
            State hw = unpack_state(state_out[n]);

            const float hw_u  = (float)hw.u;
            const float hw_v  = (float)hw.v;
            const float ref_u = (float)reference[n].u;
            const float ref_v = (float)reference[n].v;

            const float err_u = std::fabs(hw_u - ref_u);
            const float err_v = std::fabs(hw_v - ref_v);

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

    bool passed = (max_err_u < 1e-3f) && (max_err_v < 1e-3f);

    if (passed) {
        std::cout << "\nTEST PASSED\n";
        return 0;
    } else {
        std::cout << "\nTEST FAILED\n";
        return 1;
    }
}