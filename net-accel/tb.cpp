// tb.cpp

#include "accelerator.hpp"
#include "rng.hpp"

#include <cmath>
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
// CPU reference: deterministic RK4 update
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


// ============================================================
// Main test
// ============================================================

int main()
{
    //rng_test();
    //return 0;

    constexpr int N = 17;
    constexpr int E = 2 * N;

    constexpr int ITERATIONS = 1;

    const fixed_t dt    = (fixed_t)0.001;
    const fixed_t J     = (fixed_t)0.;
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
    // Build CPU reference result (RK4)
    // --------------------------------------------------------

    State reference[N];
    for (int n = 0; n < N; ++n) {
        reference[n] = unpack_state(state_in[n]);
    }

    const fixed_t dt_half  = dt * (fixed_t)0.5;
    const fixed_t dt_third = dt * (fixed_t)(1.0 / 3.0);
    const fixed_t dt_sixth = dt * (fixed_t)(1.0 / 6.0);

    for (int t = 0; t < ITERATIONS; ++t) {
        Deriv k1[N], k2[N], k3[N], k4[N];
        State s_stage[N];
        State s_orig[N];

        for (int n = 0; n < N; ++n) {
            s_orig[n] = reference[n];
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
            reference[n].u = s_orig[n].u + k1[n].du * dt_sixth + k2[n].du * dt_third + k3[n].du * dt_third + k4[n].du * dt_sixth;
            reference[n].v = s_orig[n].v + k1[n].dv * dt_sixth + k2[n].dv * dt_third + k3[n].dv * dt_third + k4[n].dv * dt_sixth;
        }
    }


    // --------------------------------------------------------
    // Call DUT
    // --------------------------------------------------------

    std::cout << "\nCalling net_accel...\n" << std::endl;

    net_accel(
        state_in,
        edges,
        out_degrees,
        state_out,
        N,
        E,
        ITERATIONS,
        dt,
        J,
        a,
        inv_e,
        sigma_sqrt_dt,
        seed,
        reseed
    );

    std::cout << "net_accel returned.\n" << std::endl;


    // --------------------------------------------------------
    // Compare
    // --------------------------------------------------------

    bool passed = true;

    std::cout << "\nResults:\n";

    for (int n = 0; n < N; ++n) {

        State hw =
            unpack_state(state_out[n]);

        const float hw_u =
            (float)hw.u;

        const float hw_v =
            (float)hw.v;

        const float ref_u =
            (float)reference[n].u;

        const float ref_v =
            (float)reference[n].v;


        const float err_u =
            std::fabs(hw_u - ref_u);

        const float err_v =
            std::fabs(hw_v - ref_v);


        std::cout
            << "n=" << n
            << "  HW=("
            << hw_u << ", "
            << hw_v << ")"
            << "  REF=("
            << ref_u << ", "
            << ref_v << ")"
            << "  ERR=("
            << err_u << ", "
            << err_v << ")"
            << "\n";


        // Since CPU reference uses the same fixed-point operations,
        // these should normally match exactly or nearly exactly.
        if (err_u > 1e-5f || err_v > 1e-5f) {
            passed = false;
        }
    }


    if (passed) {
        std::cout << "\nTEST PASSED\n";
        return 0;
    }
    else {
        std::cout << "\nTEST FAILED\n";
        return 1;
    }
}