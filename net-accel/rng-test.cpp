#include <fstream>
#include <iostream>

#include "rng.hpp"

static void seed_gaussian_rng(
    ap_uint<32> rng_state[GAUSS_UNIFORMS],
    ap_uint<32> seed
) {
    rng_state[0] = seed;
    rng_state[1] = seed ^ 0x9E3779B9u;
    rng_state[2] = seed ^ 0x243F6A88u;
    rng_state[3] = seed ^ 0xB7E15162u;
    rng_state[4] = seed ^ 0xDEADBEEFu;
    rng_state[5] = seed ^ 0xCAFEBABEu;
    rng_state[6] = seed ^ 0x8BADF00Du;
    rng_state[7] = seed ^ 0xFEEDFACEu;
}

int rng_test()
{
    constexpr int N_SAMPLES = 1000000;

   ap_uint<32> rng_state[GAUSS_UNIFORMS];

    ap_uint<32> seed = 123456789u;

    seed_gaussian_rng(rng_state, seed);
    std::ofstream output("gaussian_samples.txt");

    if (!output) {
        std::cerr << "Could not open output file.\n";
        return 1;
    }

    for (int i = 0; i < N_SAMPLES; ++i) {

        fixed_t sample = gaussian_clt(rng_state);

        // Conversion only for output / Mathematica analysis
        output << sample.to_double() << '\n';
    }

    output.close();

    std::cout << "Generated " << N_SAMPLES << " samples.\n";

    return 0;
}