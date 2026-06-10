#include "accelerator.hpp"

static void unpack_job(
    JobPacked packed,
    fixed_t in[NUM_INPUTS]
)
{
#pragma HLS INLINE

    for(int i=0;i<NUM_INPUTS;i++) {
    #pragma HLS UNROLL

        ap_int<32> tmp =
            packed.range(32*i+31,32*i);

        in[i].range() = tmp;
    }
}

static ResultPacked pack_result(
    fixed_t out[NUM_OUTPUTS]
)
{
#pragma HLS INLINE

    ResultPacked packed = 0;

    for(int i=0;i<NUM_OUTPUTS;i++) {
    #pragma HLS UNROLL

        packed.range(32*i+31,32*i) =
            out[i].range();
    }

    return packed;
}

static void compute_job(
    fixed_t in[NUM_INPUTS],
    fixed_t out[NUM_OUTPUTS],
    fixed_t dt,
    fixed_t a,
    fixed_t inv_e
)
{
#pragma HLS INLINE

    fixed_t u     = in[0];
    fixed_t u_dot = in[1];
    fixed_t v     = in[2];
    fixed_t v_dot = in[3];
    fixed_t I     = in[4];

    fixed_t u_new = u + u_dot * dt;

    // I believe the /3 gets compiled out im too bothered to check it myself
    fixed_t u_dot_new = (u - u*u*u/(fixed_t)3 - v + I) * inv_e;

    fixed_t v_new = v + v_dot * dt;

    fixed_t v_dot_new = u + a;

    out[0] = u_new;
    out[1] = u_dot_new;
    out[2] = v_new;
    out[3] = v_dot_new;
}

void accelerator(
    JobPacked* jobs,
    ResultPacked* results,
    int num_jobs,
    fixed_t dt,
    fixed_t a,
    fixed_t inv_e
) {

    // TODO CHECK BURST SIZE

    
#pragma HLS INTERFACE m_axi port=jobs    offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=results offset=slave bundle=gmem1

#pragma HLS INTERFACE s_axilite port=jobs
#pragma HLS INTERFACE s_axilite port=results
#pragma HLS INTERFACE s_axilite port=num_jobs

#pragma HLS INTERFACE s_axilite port=dt
#pragma HLS INTERFACE s_axilite port=a
#pragma HLS INTERFACE s_axilite port=inv_e

#pragma HLS INTERFACE s_axilite port=return

MAIN_LOOP:
    for(int i=0;i<num_jobs;i++) {
    #pragma HLS PIPELINE II=1

        fixed_t in[NUM_INPUTS];
        fixed_t out[NUM_OUTPUTS];

        // Access all array elements simultaneously
        #pragma HLS ARRAY_PARTITION variable=in complete
        #pragma HLS ARRAY_PARTITION variable=out complete

        JobPacked j = jobs[i];

        unpack_job(j, in);

        compute_job(
            in,
            out,
            dt,
            a,
            inv_e
        );

        results[i] = pack_result(out);
    }
}