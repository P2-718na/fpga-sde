#include "sde-accel.hpp"

static void read_jobs(
    Job* jobs,
    hls::stream<Job>& job_stream,
    int num_jobs
) {

READ_LOOP:
    for (int i = 0; i < num_jobs; i++) {

#pragma HLS PIPELINE II=1

        Job j = jobs[i];

        job_stream.write(j);
    }
}

static void compute_jobs(
    hls::stream<Job>& job_stream,
    hls::stream<Result>& result_stream,
    int num_jobs
) {

COMPUTE_LOOP:
    for (int i = 0; i < num_jobs; i++) {

#pragma HLS PIPELINE II=1

        Job j = job_stream.read();

        Result r;

        fixed_t sum = 0;

        for (int k = 0; k < NUM_INPUTS; k++) {
#pragma HLS UNROLL
            sum += j.in[k];
        }

        for (int k = 0; k < NUM_OUTPUTS; k++) {
#pragma HLS UNROLL
            r.out[k] = sum + k;
        }

        result_stream.write(r);
    }
}

static void write_results(
    Result* results,
    hls::stream<Result>& result_stream,
    int num_jobs
) {

WRITE_LOOP:
    for (int i = 0; i < num_jobs; i++) {

#pragma HLS PIPELINE II=1

        Result r = result_stream.read();

        results[i] = r;
    }
}

void accelerator(
    Job* jobs,
    Result* results,
    int num_jobs
) {

#pragma HLS INTERFACE m_axi port=jobs    offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=results offset=slave bundle=gmem

#pragma HLS INTERFACE s_axilite port=jobs
#pragma HLS INTERFACE s_axilite port=results
#pragma HLS INTERFACE s_axilite port=num_jobs
#pragma HLS INTERFACE s_axilite port=return

#pragma HLS DATAFLOW

    hls::stream<Job> job_stream;
    hls::stream<Result> result_stream;

#pragma HLS STREAM variable=job_stream depth=FIFO_DEPTH
#pragma HLS STREAM variable=result_stream depth=FIFO_DEPTH

    read_jobs(jobs, job_stream, num_jobs);

    compute_jobs(job_stream, result_stream, num_jobs);

    write_results(results, result_stream, num_jobs);
}