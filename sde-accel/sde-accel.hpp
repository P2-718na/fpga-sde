#ifndef ACCELERATOR_HPP
#define ACCELERATOR_HPP

#include <hls_stream.h>
#include <ap_fixed.h>

#define NUM_INPUTS   6
#define NUM_OUTPUTS  4
#define FIFO_DEPTH   64

typedef ap_fixed<32, 8> fixed_t;

struct Job {
    fixed_t in[NUM_INPUTS];
};

struct Result {
    fixed_t out[NUM_OUTPUTS];
};

void accelerator(
    Job* jobs,
    Result* results,
    int num_jobs
);

#endif