#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

constexpr int N = 102400;

constexpr float DT = 1e-3f;
constexpr float T_END = 0.002f;

constexpr float A = 0.7f;
constexpr float INV_E = 1.0f / 0.08f;

constexpr float SIGMA = 0.1f;
constexpr uint32_t SEED = 12345678;

constexpr int GAUSS_UNIFORMS = 8;
// Slightly faster to use CLT than the std library thingy, but by like 10%
constexpr float CLT_SCALE = 1.22474487139f;

struct Neuron
{
    float u;
    float v;
    float I;
};

inline uint32_t xorshift32(uint32_t& state)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;

    return state;
}

inline float uniform01(uint32_t& state)
{
    return (xorshift32(state) >> 8)
        * (1.0f / 16777216.0f);
}

inline float gaussian_clt(
    uint32_t rng_state[GAUSS_UNIFORMS]
)
{
    float sum = 0.0f;

#pragma unroll
    for(int i = 0; i < GAUSS_UNIFORMS; ++i)
    {
        sum += uniform01(rng_state[i]);
    }

    return (sum - 4.0f) * CLT_SCALE;
}

int main()
{
    const int num_steps =
        static_cast<int>(T_END / DT);

    const float sigma_sqrt_dt =
        SIGMA * std::sqrt(DT);

    std::vector<Neuron> state(N);

    for(int i = 0; i < N; ++i)
    {
        state[i].u = 0.0f;
        state[i].v = 0.0f;
        state[i].I = 0.5f;
    }

    uint32_t rng_state[GAUSS_UNIFORMS] =
    {
        SEED,
        SEED ^ 0x9E3779B9u,
        SEED ^ 0x243F6A88u,
        SEED ^ 0xB7E15162u,
        SEED ^ 0xDEADBEEFu,
        SEED ^ 0xCAFEBABEu,
        SEED ^ 0x8BADF00Du,
        SEED ^ 0xFEEDFACEu
    };

    std::cout
        << "About to integrate for "
        << num_steps
        << " steps.\n";

    auto start =
        std::chrono::high_resolution_clock::now();

    for(int step = 0; step < num_steps; ++step)
    {
        for(int n = 0; n < N; ++n)
        {
            float u = state[n].u;
            float v = state[n].v;
            float I = state[n].I;

            float du =
                (u
                - u*u*u/3.0f
                - v
                + I)
                * INV_E;

            float dv =
                u + A;

            float noise =
                sigma_sqrt_dt *
                gaussian_clt(rng_state);

            state[n].u =
                u + du * DT + noise;

            state[n].v =
                v + dv * DT;

            state[n].I = 0.0f;
        }
    }

    auto end =
        std::chrono::high_resolution_clock::now();

    double elapsed =
        std::chrono::duration<double>(
            end - start
        ).count();

    long long total_jobs =
        static_cast<long long>(N)
        * num_steps;

    std::cout << "\n";
    std::cout << "================================\n";
    std::cout << "Simulation completed\n";
    std::cout << "================================\n";
    std::cout << "N             : " << N << "\n";
    std::cout << "Steps         : " << num_steps << "\n";
    std::cout << "Total jobs    : " << total_jobs << "\n";
    std::cout << "Elapsed       : " << elapsed << " s\n";
    std::cout << "MJobs/s       : "
              << total_jobs / elapsed / 1e6
              << "\n";
    std::cout << "Steps/s       : "
              << num_steps / elapsed
              << "\n";
    std::cout << "================================\n";

    return 0;
}