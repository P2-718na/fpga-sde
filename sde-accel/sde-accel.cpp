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
    fixed_t out[NUM_OUTPUTS]
)
{
#pragma HLS INLINE

    //fixed_t sum = 0;

    for(int i=0;i<NUM_INPUTS;i++) {
#pragma HLS UNROLL
       // sum += in[i];
    }

    for(int i=0;i<NUM_OUTPUTS;i++) {
#pragma HLS UNROLL
        out[i] = in[i] + (fixed_t)1;
    }
}

void accelerator(
    JobPacked* jobs,
    ResultPacked* results,
    int num_jobs
)
{
#pragma HLS INTERFACE m_axi port=jobs    offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=results offset=slave bundle=gmem1

#pragma HLS INTERFACE s_axilite port=jobs
#pragma HLS INTERFACE s_axilite port=results
#pragma HLS INTERFACE s_axilite port=num_jobs
#pragma HLS INTERFACE s_axilite port=return

MAIN_LOOP:
    for(int i=0;i<num_jobs;i++) {

#pragma HLS PIPELINE II=1

        fixed_t in[NUM_INPUTS];
        fixed_t out[NUM_OUTPUTS];

#pragma HLS ARRAY_PARTITION variable=in complete
#pragma HLS ARRAY_PARTITION variable=out complete

        JobPacked j = jobs[i];

        unpack_job(j, in);

        compute_job(in, out);

        results[i] = pack_result(out);
    }
}