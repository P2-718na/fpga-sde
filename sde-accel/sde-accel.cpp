#include "accelerator.hpp"

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

static void compute_job(
    State& s,
    fixed_t I,
    fixed_t dt,
    fixed_t a,
    fixed_t inv_e,
    fixed_t sigma_sqrt_dt,
    ap_uint<32> rng_state[GAUSS_UNIFORMS]
)
{
#pragma HLS INLINE

    const fixed_t u = s.u;
    const fixed_t v = s.v;

    const fixed_t du = (u - u*u*u*(fixed_t)(1.0/3.0) - v + I) * inv_e;
    const fixed_t dv = u + a;
    // Qui non sto dividendo per epsilon per salvare risorse.
    // La temperatura lato python deve contenere il fattore giusto.
    const fixed_t noise = sigma_sqrt_dt * gaussian_clt(rng_state);
    
    s.u = u + du * dt + noise;
    s.v = v + dv * dt;
}

void accelerator(
    StateBits* state,
    fixed_t* I,
    int num_jobs,
    fixed_t dt,
    fixed_t a,
    fixed_t inv_e,
    fixed_t sigma_sqrt_dt,
    ap_uint<32> seed,
    bool reseed
)
{
// Make sure the packing is happening for state
#pragma HLS aggregate variable=state
    
#pragma HLS INTERFACE m_axi port=state offset=slave bundle=gmem0 // num_read_outstanding=16 num_write_outstanding=16 didn't help with II2
#pragma HLS INTERFACE m_axi port=I     offset=slave bundle=gmem1

#pragma HLS INTERFACE s_axilite port=state 
#pragma HLS INTERFACE s_axilite port=I
#pragma HLS INTERFACE s_axilite port=num_jobs

#pragma HLS INTERFACE s_axilite port=dt
#pragma HLS INTERFACE s_axilite port=a
#pragma HLS INTERFACE s_axilite port=inv_e
#pragma HLS INTERFACE s_axilite port=sigma_sqrt_dt
#pragma HLS INTERFACE s_axilite port=seed
#pragma HLS INTERFACE s_axilite port=reseed
#pragma HLS INTERFACE s_axilite port=return

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

    if(reseed)
    {
        rng_state[0] = seed;
        rng_state[1] = seed ^ 0x9E3779B9;
        rng_state[2] = seed ^ 0x243F6A88;
        rng_state[3] = seed ^ 0xB7E15162;
        rng_state[4] = seed ^ 0xDEADBEEF;
        rng_state[5] = seed ^ 0xCAFEBABE;
        rng_state[6] = seed ^ 0x8BADF00D;
        rng_state[7] = seed ^ 0xFEEDFACE;
    }

// Tell vitis that processing for all inputs can be done independently
#pragma HLS DEPENDENCE variable=state inter false

MAIN_LOOP:
    for(int i = 0; i < num_jobs; ++i)
    {
    #pragma HLS PIPELINE II=1

        PackedState s;

        // One 64-bit AXI read
        s.bits = state[i];

        compute_job(
            s.state,
            I[i],
            dt,
            a,
            inv_e,
            sigma_sqrt_dt,
            rng_state);

        // One 64-bit AXI write
        state[i] = s.bits;
    }
}