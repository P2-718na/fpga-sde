// tb.cpp

#include "accelerator.hpp"

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
// CPU reference: deterministic one-step update
// ============================================================

static State cpu_integrate_one(
    const State& old_s,
    fixed_t I,
    fixed_t dt,
    fixed_t a,
    fixed_t inv_e
)
{
    State result;

    const fixed_t u = old_s.u;
    const fixed_t v = old_s.v;

    const fixed_t du =
        (u
        - u*u*u*(fixed_t)(1.0/3.0)
        - v
        + I)
        * inv_e;

    const fixed_t dv = u + a;

    result.u = u + du * dt;
    result.v = v + dv * dt;

    return result;
}


// ============================================================
// Main test
// ============================================================

int main()
{
    constexpr int N = 8;
    constexpr int E = 2 * N;

    constexpr int ITERATIONS = 1;

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
    // Build CPU reference result
    // --------------------------------------------------------

    State reference[N];

    for (int n = 0; n < N; ++n) {

        State self =
            unpack_state(state_in[n]);

        const int left =
            (n - 1 + N) % N;

        const int right =
            (n + 1) % N;

        State s_left =
            unpack_state(state_in[left]);

        State s_right =
            unpack_state(state_in[right]);


        fixed_t incoming_sum =
            s_left.u + s_right.u;

        fixed_t outgoing_term =
            (fixed_t)2 * self.u;

        fixed_t I =
            J * (
                incoming_sum
                - outgoing_term
            );


        reference[n] =
            cpu_integrate_one(
                self,
                I,
                dt,
                a,
                inv_e
            );
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