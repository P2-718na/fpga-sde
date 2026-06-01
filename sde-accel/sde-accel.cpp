#include "example.hpp"

static void compute_job(
    fixed_t in[NUM_INPUTS],
    fixed_t out[NUM_OUTPUTS]
) {

#pragma HLS INLINE

    fixed_t sum = 0;

    // Parallel input accumulation
    for (int i = 0; i < NUM_INPUTS; i++) {
#pragma HLS UNROLL
        sum += in[i];
    }

    // Parallel output generation
    for (int o = 0; o < NUM_OUTPUTS; o++) {
#pragma HLS UNROLL
        out[o] = sum + o;
    }
}

void accelerator(
    fixed_t jobs[][NUM_INPUTS],
    fixed_t results[][NUM_OUTPUTS],
    int num_jobs
) {

#pragma HLS INTERFACE m_axi port=jobs    offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=results offset=slave bundle=gmem

#pragma HLS INTERFACE s_axilite port=jobs
#pragma HLS INTERFACE s_axilite port=results
#pragma HLS INTERFACE s_axilite port=num_jobs
#pragma HLS INTERFACE s_axilite port=return

MAIN_LOOP:
    for (int i = 0; i < num_jobs; i++) {

#pragma HLS PIPELINE II=1

#pragma HLS DEPENDENCE variable=jobs inter false
#pragma HLS DEPENDENCE variable=results inter false

        fixed_t in_local[NUM_INPUTS];
        fixed_t out_local[NUM_OUTPUTS];

#pragma HLS ARRAY_PARTITION variable=in_local complete
#pragma HLS ARRAY_PARTITION variable=out_local complete

        // Read inputs
        for (int k = 0; k < NUM_INPUTS; k++) {
#pragma HLS UNROLL
            in_local[k] = jobs[i][k];
        }

        // Compute
        compute_job(in_local, out_local);

        // Write outputs
        for (int k = 0; k < NUM_OUTPUTS; k++) {
#pragma HLS UNROLL
            results[i][k] = out_local[k];
        }
    }
}