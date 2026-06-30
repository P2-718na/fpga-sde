// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
// control
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read/COR)
//        bit 7  - auto_restart (Read/Write)
//        bit 9  - interrupt (Read)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0 - enable ap_done interrupt (Read/Write)
//        bit 1 - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0 - ap_done (Read/TOW)
//        bit 1 - ap_ready (Read/TOW)
//        others - reserved
// 0x10 : Data signal of row_ptr_ddr
//        bit 31~0 - row_ptr_ddr[31:0] (Read/Write)
// 0x14 : Data signal of row_ptr_ddr
//        bit 31~0 - row_ptr_ddr[63:32] (Read/Write)
// 0x18 : reserved
// 0x1c : Data signal of col_idx_ddr
//        bit 31~0 - col_idx_ddr[31:0] (Read/Write)
// 0x20 : Data signal of col_idx_ddr
//        bit 31~0 - col_idx_ddr[63:32] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of u_ddr
//        bit 31~0 - u_ddr[31:0] (Read/Write)
// 0x2c : Data signal of u_ddr
//        bit 31~0 - u_ddr[63:32] (Read/Write)
// 0x30 : reserved
// 0x34 : Data signal of v_ddr
//        bit 31~0 - v_ddr[31:0] (Read/Write)
// 0x38 : Data signal of v_ddr
//        bit 31~0 - v_ddr[63:32] (Read/Write)
// 0x3c : reserved
// 0x40 : Data signal of neuron_count
//        bit 31~0 - neuron_count[31:0] (Read/Write)
// 0x44 : reserved
// 0x48 : Data signal of edge_count
//        bit 31~0 - edge_count[31:0] (Read/Write)
// 0x4c : reserved
// 0x50 : Data signal of timesteps
//        bit 31~0 - timesteps[31:0] (Read/Write)
// 0x54 : reserved
// 0x58 : Data signal of command
//        bit 31~0 - command[31:0] (Read/Write)
// 0x5c : reserved
// 0x60 : Data signal of dt
//        bit 31~0 - dt[31:0] (Read/Write)
// 0x64 : reserved
// 0x68 : Data signal of a
//        bit 31~0 - a[31:0] (Read/Write)
// 0x6c : reserved
// 0x70 : Data signal of J
//        bit 31~0 - J[31:0] (Read/Write)
// 0x74 : reserved
// 0x78 : Data signal of inv_e
//        bit 31~0 - inv_e[31:0] (Read/Write)
// 0x7c : reserved
// 0x80 : Data signal of sigma_sqrt_dt
//        bit 31~0 - sigma_sqrt_dt[31:0] (Read/Write)
// 0x84 : reserved
// 0x88 : Data signal of seed
//        bit 31~0 - seed[31:0] (Read/Write)
// 0x8c : reserved
// 0x90 : Data signal of reseed
//        bit 0  - reseed[0] (Read/Write)
//        others - reserved
// 0x94 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XACCELERATOR_NET_CONTROL_ADDR_AP_CTRL            0x00
#define XACCELERATOR_NET_CONTROL_ADDR_GIE                0x04
#define XACCELERATOR_NET_CONTROL_ADDR_IER                0x08
#define XACCELERATOR_NET_CONTROL_ADDR_ISR                0x0c
#define XACCELERATOR_NET_CONTROL_ADDR_ROW_PTR_DDR_DATA   0x10
#define XACCELERATOR_NET_CONTROL_BITS_ROW_PTR_DDR_DATA   64
#define XACCELERATOR_NET_CONTROL_ADDR_COL_IDX_DDR_DATA   0x1c
#define XACCELERATOR_NET_CONTROL_BITS_COL_IDX_DDR_DATA   64
#define XACCELERATOR_NET_CONTROL_ADDR_U_DDR_DATA         0x28
#define XACCELERATOR_NET_CONTROL_BITS_U_DDR_DATA         64
#define XACCELERATOR_NET_CONTROL_ADDR_V_DDR_DATA         0x34
#define XACCELERATOR_NET_CONTROL_BITS_V_DDR_DATA         64
#define XACCELERATOR_NET_CONTROL_ADDR_NEURON_COUNT_DATA  0x40
#define XACCELERATOR_NET_CONTROL_BITS_NEURON_COUNT_DATA  32
#define XACCELERATOR_NET_CONTROL_ADDR_EDGE_COUNT_DATA    0x48
#define XACCELERATOR_NET_CONTROL_BITS_EDGE_COUNT_DATA    32
#define XACCELERATOR_NET_CONTROL_ADDR_TIMESTEPS_DATA     0x50
#define XACCELERATOR_NET_CONTROL_BITS_TIMESTEPS_DATA     32
#define XACCELERATOR_NET_CONTROL_ADDR_COMMAND_DATA       0x58
#define XACCELERATOR_NET_CONTROL_BITS_COMMAND_DATA       32
#define XACCELERATOR_NET_CONTROL_ADDR_DT_DATA            0x60
#define XACCELERATOR_NET_CONTROL_BITS_DT_DATA            32
#define XACCELERATOR_NET_CONTROL_ADDR_A_DATA             0x68
#define XACCELERATOR_NET_CONTROL_BITS_A_DATA             32
#define XACCELERATOR_NET_CONTROL_ADDR_J_DATA             0x70
#define XACCELERATOR_NET_CONTROL_BITS_J_DATA             32
#define XACCELERATOR_NET_CONTROL_ADDR_INV_E_DATA         0x78
#define XACCELERATOR_NET_CONTROL_BITS_INV_E_DATA         32
#define XACCELERATOR_NET_CONTROL_ADDR_SIGMA_SQRT_DT_DATA 0x80
#define XACCELERATOR_NET_CONTROL_BITS_SIGMA_SQRT_DT_DATA 32
#define XACCELERATOR_NET_CONTROL_ADDR_SEED_DATA          0x88
#define XACCELERATOR_NET_CONTROL_BITS_SEED_DATA          32
#define XACCELERATOR_NET_CONTROL_ADDR_RESEED_DATA        0x90
#define XACCELERATOR_NET_CONTROL_BITS_RESEED_DATA        1

