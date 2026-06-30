#include <iostream>
#include "accelerator.hpp"

int main()
{
    const int N = 4;
    const int E = 4;

    uint32_t row_ptr_ddr[N + 1] = {
        0, 1, 2, 3, 4
    };

    uint16_t col_idx_ddr[E] = {
        1, 2, 3, 0
    };

    fixed_t u_ddr[N];
    fixed_t v_ddr[N];

    for(int i=0;i<N;i++) {
        u_ddr[i] = 0;
        v_ddr[i] = 0;
    }

    u_ddr[0] = 1.0;

    std::cout << "Initial state\n";

    for(int i=0;i<N;i++) {
        std::cout
            << i
            << " "
            << u_ddr[i].to_float()
            << " "
            << v_ddr[i].to_float()
            << "\n";
    }

    accelerator_net(
        row_ptr_ddr,
        col_idx_ddr,
        u_ddr,
        v_ddr,
        N,
        E,
        0,
        CMD_LOAD_GRAPH,
        0,
        0,
        0,
        0,
        0,
        0,
        false
    );

    accelerator_net(
        row_ptr_ddr,
        col_idx_ddr,
        u_ddr,
        v_ddr,
        N,
        E,
        0,
        CMD_LOAD_STATE,
        0,
        0,
        0,
        0,
        0,
        0,
        false
    );

    //
    // overwrite host memory
    //

    for(int i=0;i<N;i++) {
        u_ddr[i] = 0;
        v_ddr[i] = 0;
    }

    accelerator_net(
        row_ptr_ddr,
        col_idx_ddr,
        u_ddr,
        v_ddr,
        N,
        E,
        0,
        CMD_READ_STATE,
        0,
        0,
        0,
        0,
        0,
        0,
        false
    );

    std::cout << "\nAfter LOAD_STATE / READ_STATE\n";

    for(int i=0;i<N;i++) {
        std::cout
            << i
            << " "
            << u_ddr[i].to_float()
            << " "
            << v_ddr[i].to_float()
            << "\n";
    }

    accelerator_net(
        row_ptr_ddr,
        col_idx_ddr,
        u_ddr,
        v_ddr,
        N,
        E,
        1,
        CMD_RUN,
        (fixed_t)0.001,
        (fixed_t)1.3,
        (fixed_t)1.0,
        (fixed_t)10.0,
        (fixed_t)0.0,
        1234,
        true
    );

    accelerator_net(
        row_ptr_ddr,
        col_idx_ddr,
        u_ddr,
        v_ddr,
        N,
        E,
        0,
        CMD_READ_STATE,
        (fixed_t)0.001,
        (fixed_t)1.3,
        (fixed_t)1.0,
        (fixed_t)10.0,
        (fixed_t)0.0,
        1234,
        false
    );

    std::cout << "\nAfter RUN\n";

    for(int i=0;i<N;i++) {
        std::cout
            << i
            << " "
            << u_ddr[i].to_float()
            << " "
            << v_ddr[i].to_float()
            << "\n";
    }

    return 0;
}