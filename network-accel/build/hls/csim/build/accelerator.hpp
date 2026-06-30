#ifndef ACCELERATOR_HPP
#define ACCELERATOR_HPP

#include <ap_fixed.h>
#include <ap_int.h>

typedef ap_fixed<32,8> fixed_t;

#define MAX_NEURONS 2048
#define MAX_EDGES   32768

#define GAUSS_UNIFORMS 8

#if GAUSS_UNIFORMS == 4
#define CLT_SCALE (fixed_t)1.73205080757
#elif GAUSS_UNIFORMS == 8
#define CLT_SCALE (fixed_t)1.22474487139
#elif GAUSS_UNIFORMS == 12
#define CLT_SCALE (fixed_t)1.0
#else
#error "Unsupported GAUSS_UNIFORMS"
#endif

enum Command {
    CMD_LOAD_GRAPH = 0,
    CMD_LOAD_STATE = 1,
    CMD_RUN        = 2,
    CMD_READ_STATE = 3
};

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
);

#endif