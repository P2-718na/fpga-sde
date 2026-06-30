#include "accelerator.hpp"

//////////////////////////////////////////////////////////
// BRAM STORAGE
//////////////////////////////////////////////////////////

static uint32_t row_ptr[MAX_NEURONS + 1];
static uint16_t col_idx[MAX_EDGES];

static uint16_t degree[MAX_NEURONS];

static fixed_t u_a[MAX_NEURONS];
static fixed_t v_a[MAX_NEURONS];

static fixed_t u_b[MAX_NEURONS];
static fixed_t v_b[MAX_NEURONS];


//////////////////////////////////////////////////////////
// RNG
//////////////////////////////////////////////////////////

static ap_uint<32> xorshift32(ap_uint<32>& state)
{
#pragma HLS INLINE

    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;

    return state;
}

static fixed_t uniform01(ap_uint<32>& state)
{
#pragma HLS INLINE

    ap_uint<32> r = xorshift32(state);

    ap_ufixed<24,0> frac;
    frac.range(23,0) = r.range(31,8);

    return (fixed_t)frac;
}

static fixed_t gaussian_clt(
    ap_uint<32> rng_state[GAUSS_UNIFORMS]
)
{
#pragma HLS INLINE

    fixed_t sum = 0;

    for(int i=0;i<GAUSS_UNIFORMS;i++) {
#pragma HLS UNROLL
        sum += uniform01(rng_state[i]);
    }

    return (sum - (fixed_t)(GAUSS_UNIFORMS/2))
           * CLT_SCALE;
}

//////////////////////////////////////////////////////////
// TOP
//////////////////////////////////////////////////////////

void accelerator_net(
    uint32_t* row_ptr_ddr,
    uint16_t* col_idx_ddr,

    fixed_t* u_ddr,
    fixed_t* v_ddr,

    int neuron_count,
    int edge_count,
    int timesteps,

    int command,

    fixed_t dt,
    fixed_t a,
    fixed_t J,
    fixed_t inv_e,
    fixed_t sigma_sqrt_dt,

    ap_uint<32> seed,
    bool reseed
)
{
#pragma HLS INTERFACE m_axi port=row_ptr_ddr bundle=gmem0
#pragma HLS INTERFACE m_axi port=col_idx_ddr bundle=gmem0

#pragma HLS INTERFACE m_axi port=u_ddr bundle=gmem1
#pragma HLS INTERFACE m_axi port=v_ddr bundle=gmem1

#pragma HLS INTERFACE s_axilite port=row_ptr_ddr
#pragma HLS INTERFACE s_axilite port=col_idx_ddr

#pragma HLS INTERFACE s_axilite port=u_ddr
#pragma HLS INTERFACE s_axilite port=v_ddr

#pragma HLS INTERFACE s_axilite port=neuron_count
#pragma HLS INTERFACE s_axilite port=edge_count
#pragma HLS INTERFACE s_axilite port=timesteps

#pragma HLS INTERFACE s_axilite port=command

#pragma HLS INTERFACE s_axilite port=dt
#pragma HLS INTERFACE s_axilite port=a
#pragma HLS INTERFACE s_axilite port=J
#pragma HLS INTERFACE s_axilite port=inv_e
#pragma HLS INTERFACE s_axilite port=sigma_sqrt_dt

#pragma HLS INTERFACE s_axilite port=seed
#pragma HLS INTERFACE s_axilite port=reseed

#pragma HLS INTERFACE s_axilite port=return


/*
#pragma HLS ARRAY_PARTITION variable=u_a cyclic factor=4
#pragma HLS ARRAY_PARTITION variable=u_b cyclic factor=4
#pragma HLS ARRAY_PARTITION variable=v_a cyclic factor=
#pragma HLS ARRAY_PARTITION variable=v_b cyclic factor=
*/


//////////////////////////////////////////////////////////
// RNG
//////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////
// LOAD GRAPH
//////////////////////////////////////////////////////////

    if(command == CMD_LOAD_GRAPH)
    {
    LOAD_ROW_PTR:
        for(int i=0;i<neuron_count+1;i++)
        {
#pragma HLS PIPELINE II=1
            row_ptr[i] = row_ptr_ddr[i];
        }

    LOAD_COL_IDX:
        for(int i=0;i<edge_count;i++)
        {
#pragma HLS PIPELINE II=1
            col_idx[i] = col_idx_ddr[i];
        }

    COMPUTE_DEGREE:
        for(int n=0;n<neuron_count;n++)
        {
#pragma HLS PIPELINE II=1
            degree[n] =
                row_ptr[n+1]
                - row_ptr[n];
        }

        return;
    }

//////////////////////////////////////////////////////////
// LOAD STATE
//////////////////////////////////////////////////////////

    if(command == CMD_LOAD_STATE)
    {
    LOAD_STATE:
        for(int i=0;i<neuron_count;i++)
        {
#pragma HLS PIPELINE II=1

            u_a[i] = u_ddr[i];
            v_a[i] = v_ddr[i];
        }

        return;
    }

//////////////////////////////////////////////////////////
// READ STATE
//////////////////////////////////////////////////////////

    if(command == CMD_READ_STATE)
    {
    STORE_STATE:
        for(int i=0;i<neuron_count;i++)
        {
#pragma HLS PIPELINE II=1

            u_ddr[i] = u_a[i];
            v_ddr[i] = v_a[i];
        }

        return;
    }

//////////////////////////////////////////////////////////
// RUN
//////////////////////////////////////////////////////////

    if(command == CMD_RUN)
    {
    TIMESTEP_LOOP:
        for(int step=0; step<timesteps; step++)
        {

        NEURON_LOOP:
            for(int n=0; n<neuron_count; n++)
            {
                fixed_t inflow = 0;

            EDGE_LOOP:
                for(
                    int e = row_ptr[n];
                    e < row_ptr[n+1];
                    e++
                )
                {
#pragma HLS PIPELINE II=1

                    const int m =
                        col_idx[e];

                    inflow += u_a[m];
                }

                const fixed_t u =
                    u_a[n];

                const fixed_t v =
                    v_a[n];

                const fixed_t I =
                    J * (
                        inflow
                        - (fixed_t)degree[n] * u
                    );

                const fixed_t du =
                    (
                        u
                        - u*u*u/(fixed_t)3
                        - v
                        + I
                    ) * inv_e;

                const fixed_t dv =
                    u + a;

                const fixed_t noise =
                    sigma_sqrt_dt
                    * gaussian_clt(rng_state);

                u_b[n] =
                    u
                    + du*dt
                    + noise;

                v_b[n] =
                    v
                    + dv*dt;
            }

        SWAP:
            for(int i=0;i<neuron_count;i++)
            {
#pragma HLS PIPELINE II=1

                u_a[i] = u_b[i];
                v_a[i] = v_b[i];
            }
        }
    }
}