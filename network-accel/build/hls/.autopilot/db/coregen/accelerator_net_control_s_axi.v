// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
`timescale 1ns/1ps
(* DowngradeIPIdentifiedWarnings="yes" *) module accelerator_net_control_s_axi
#(parameter
    C_S_AXI_ADDR_WIDTH = 8,
    C_S_AXI_DATA_WIDTH = 32
)(
    input  wire                          ACLK,
    input  wire                          ARESET,
    input  wire                          ACLK_EN,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0] AWADDR,
    input  wire                          AWVALID,
    output wire                          AWREADY,
    input  wire [C_S_AXI_DATA_WIDTH-1:0] WDATA,
    input  wire [C_S_AXI_DATA_WIDTH/8-1:0] WSTRB,
    input  wire                          WVALID,
    output wire                          WREADY,
    output wire [1:0]                    BRESP,
    output wire                          BVALID,
    input  wire                          BREADY,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0] ARADDR,
    input  wire                          ARVALID,
    output wire                          ARREADY,
    output wire [C_S_AXI_DATA_WIDTH-1:0] RDATA,
    output wire [1:0]                    RRESP,
    output wire                          RVALID,
    input  wire                          RREADY,
    output wire                          interrupt,
    output wire [63:0]                   row_ptr_ddr,
    output wire [63:0]                   col_idx_ddr,
    output wire [63:0]                   u_ddr,
    output wire [63:0]                   v_ddr,
    output wire [31:0]                   neuron_count,
    output wire [31:0]                   edge_count,
    output wire [31:0]                   timesteps,
    output wire [31:0]                   command,
    output wire [31:0]                   dt,
    output wire [31:0]                   a,
    output wire [31:0]                   J,
    output wire [31:0]                   inv_e,
    output wire [31:0]                   sigma_sqrt_dt,
    output wire [31:0]                   seed,
    output wire [0:0]                    reseed,
    output wire                          ap_start,
    input  wire                          ap_done,
    input  wire                          ap_ready,
    input  wire                          ap_idle
);
//------------------------Address Info-------------------
// Protocol Used: ap_ctrl_hs
//
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

//------------------------Parameter----------------------
localparam
    ADDR_AP_CTRL              = 8'h00,
    ADDR_GIE                  = 8'h04,
    ADDR_IER                  = 8'h08,
    ADDR_ISR                  = 8'h0c,
    ADDR_ROW_PTR_DDR_DATA_0   = 8'h10,
    ADDR_ROW_PTR_DDR_DATA_1   = 8'h14,
    ADDR_ROW_PTR_DDR_CTRL     = 8'h18,
    ADDR_COL_IDX_DDR_DATA_0   = 8'h1c,
    ADDR_COL_IDX_DDR_DATA_1   = 8'h20,
    ADDR_COL_IDX_DDR_CTRL     = 8'h24,
    ADDR_U_DDR_DATA_0         = 8'h28,
    ADDR_U_DDR_DATA_1         = 8'h2c,
    ADDR_U_DDR_CTRL           = 8'h30,
    ADDR_V_DDR_DATA_0         = 8'h34,
    ADDR_V_DDR_DATA_1         = 8'h38,
    ADDR_V_DDR_CTRL           = 8'h3c,
    ADDR_NEURON_COUNT_DATA_0  = 8'h40,
    ADDR_NEURON_COUNT_CTRL    = 8'h44,
    ADDR_EDGE_COUNT_DATA_0    = 8'h48,
    ADDR_EDGE_COUNT_CTRL      = 8'h4c,
    ADDR_TIMESTEPS_DATA_0     = 8'h50,
    ADDR_TIMESTEPS_CTRL       = 8'h54,
    ADDR_COMMAND_DATA_0       = 8'h58,
    ADDR_COMMAND_CTRL         = 8'h5c,
    ADDR_DT_DATA_0            = 8'h60,
    ADDR_DT_CTRL              = 8'h64,
    ADDR_A_DATA_0             = 8'h68,
    ADDR_A_CTRL               = 8'h6c,
    ADDR_J_DATA_0             = 8'h70,
    ADDR_J_CTRL               = 8'h74,
    ADDR_INV_E_DATA_0         = 8'h78,
    ADDR_INV_E_CTRL           = 8'h7c,
    ADDR_SIGMA_SQRT_DT_DATA_0 = 8'h80,
    ADDR_SIGMA_SQRT_DT_CTRL   = 8'h84,
    ADDR_SEED_DATA_0          = 8'h88,
    ADDR_SEED_CTRL            = 8'h8c,
    ADDR_RESEED_DATA_0        = 8'h90,
    ADDR_RESEED_CTRL          = 8'h94,
    WRIDLE                    = 2'd0,
    WRDATA                    = 2'd1,
    WRRESP                    = 2'd2,
    WRRESET                   = 2'd3,
    RDIDLE                    = 2'd0,
    RDDATA                    = 2'd1,
    RDRESET                   = 2'd2,
    ADDR_BITS                = 8;

//------------------------Local signal-------------------
    reg  [1:0]                    wstate = WRRESET;
    reg  [1:0]                    wnext;
    reg  [ADDR_BITS-1:0]          waddr;
    wire [C_S_AXI_DATA_WIDTH-1:0] wmask;
    wire                          aw_hs;
    wire                          w_hs;
    reg  [1:0]                    rstate = RDRESET;
    reg  [1:0]                    rnext;
    reg  [C_S_AXI_DATA_WIDTH-1:0] rdata;
    wire                          ar_hs;
    wire [ADDR_BITS-1:0]          raddr;
    // internal registers
    reg                           int_ap_idle = 1'b0;
    reg                           int_ap_ready = 1'b0;
    wire                          task_ap_ready;
    reg                           int_ap_done = 1'b0;
    wire                          task_ap_done;
    reg                           int_task_ap_done = 1'b0;
    reg                           int_ap_start = 1'b0;
    reg                           int_interrupt = 1'b0;
    reg                           int_auto_restart = 1'b0;
    reg                           auto_restart_status = 1'b0;
    wire                          auto_restart_done;
    reg                           int_gie = 1'b0;
    reg  [1:0]                    int_ier = 2'b0;
    reg  [1:0]                    int_isr = 2'b0;
    reg  [63:0]                   int_row_ptr_ddr = 'b0;
    reg  [63:0]                   int_col_idx_ddr = 'b0;
    reg  [63:0]                   int_u_ddr = 'b0;
    reg  [63:0]                   int_v_ddr = 'b0;
    reg  [31:0]                   int_neuron_count = 'b0;
    reg  [31:0]                   int_edge_count = 'b0;
    reg  [31:0]                   int_timesteps = 'b0;
    reg  [31:0]                   int_command = 'b0;
    reg  [31:0]                   int_dt = 'b0;
    reg  [31:0]                   int_a = 'b0;
    reg  [31:0]                   int_J = 'b0;
    reg  [31:0]                   int_inv_e = 'b0;
    reg  [31:0]                   int_sigma_sqrt_dt = 'b0;
    reg  [31:0]                   int_seed = 'b0;
    reg  [0:0]                    int_reseed = 'b0;

//------------------------Instantiation------------------


//------------------------AXI write fsm------------------
assign AWREADY = (wstate == WRIDLE);
assign WREADY  = (wstate == WRDATA);
assign BVALID  = (wstate == WRRESP);
assign BRESP   = 2'b00;  // OKAY
assign wmask   = { {8{WSTRB[3]}}, {8{WSTRB[2]}}, {8{WSTRB[1]}}, {8{WSTRB[0]}} };
assign aw_hs   = AWVALID & AWREADY;
assign w_hs    = WVALID & WREADY;

// wstate
always @(posedge ACLK) begin
    if (ARESET)
        wstate <= WRRESET;
    else if (ACLK_EN)
        wstate <= wnext;
end

// wnext
always @(*) begin
    case (wstate)
        WRIDLE:
            if (AWVALID)
                wnext = WRDATA;
            else
                wnext = WRIDLE;
        WRDATA:
            if (WVALID)
                wnext = WRRESP;
            else
                wnext = WRDATA;
        WRRESP:
            if (BREADY & BVALID)
                wnext = WRIDLE;
            else
                wnext = WRRESP;
        default:
            wnext = WRIDLE;
    endcase
end

// waddr
always @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (aw_hs)
            waddr <= {AWADDR[ADDR_BITS-1:2], {2{1'b0}}};
    end
end

//------------------------AXI read fsm-------------------
assign ARREADY = (rstate == RDIDLE);
assign RDATA   = rdata;
assign RRESP   = 2'b00;  // OKAY
assign RVALID  = (rstate == RDDATA);
assign ar_hs   = ARVALID & ARREADY;
assign raddr   = ARADDR[ADDR_BITS-1:0];

// rstate
always @(posedge ACLK) begin
    if (ARESET)
        rstate <= RDRESET;
    else if (ACLK_EN)
        rstate <= rnext;
end

// rnext
always @(*) begin
    case (rstate)
        RDIDLE:
            if (ARVALID)
                rnext = RDDATA;
            else
                rnext = RDIDLE;
        RDDATA:
            if (RREADY & RVALID)
                rnext = RDIDLE;
            else
                rnext = RDDATA;
        default:
            rnext = RDIDLE;
    endcase
end

// rdata
always @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (ar_hs) begin
            rdata <= 'b0;
            case (raddr)
                ADDR_AP_CTRL: begin
                    rdata[0] <= int_ap_start;
                    rdata[1] <= int_task_ap_done;
                    rdata[2] <= int_ap_idle;
                    rdata[3] <= int_ap_ready;
                    rdata[7] <= int_auto_restart;
                    rdata[9] <= int_interrupt;
                end
                ADDR_GIE: begin
                    rdata <= int_gie;
                end
                ADDR_IER: begin
                    rdata <= int_ier;
                end
                ADDR_ISR: begin
                    rdata <= int_isr;
                end
                ADDR_ROW_PTR_DDR_DATA_0: begin
                    rdata <= int_row_ptr_ddr[31:0];
                end
                ADDR_ROW_PTR_DDR_DATA_1: begin
                    rdata <= int_row_ptr_ddr[63:32];
                end
                ADDR_COL_IDX_DDR_DATA_0: begin
                    rdata <= int_col_idx_ddr[31:0];
                end
                ADDR_COL_IDX_DDR_DATA_1: begin
                    rdata <= int_col_idx_ddr[63:32];
                end
                ADDR_U_DDR_DATA_0: begin
                    rdata <= int_u_ddr[31:0];
                end
                ADDR_U_DDR_DATA_1: begin
                    rdata <= int_u_ddr[63:32];
                end
                ADDR_V_DDR_DATA_0: begin
                    rdata <= int_v_ddr[31:0];
                end
                ADDR_V_DDR_DATA_1: begin
                    rdata <= int_v_ddr[63:32];
                end
                ADDR_NEURON_COUNT_DATA_0: begin
                    rdata <= int_neuron_count[31:0];
                end
                ADDR_EDGE_COUNT_DATA_0: begin
                    rdata <= int_edge_count[31:0];
                end
                ADDR_TIMESTEPS_DATA_0: begin
                    rdata <= int_timesteps[31:0];
                end
                ADDR_COMMAND_DATA_0: begin
                    rdata <= int_command[31:0];
                end
                ADDR_DT_DATA_0: begin
                    rdata <= int_dt[31:0];
                end
                ADDR_A_DATA_0: begin
                    rdata <= int_a[31:0];
                end
                ADDR_J_DATA_0: begin
                    rdata <= int_J[31:0];
                end
                ADDR_INV_E_DATA_0: begin
                    rdata <= int_inv_e[31:0];
                end
                ADDR_SIGMA_SQRT_DT_DATA_0: begin
                    rdata <= int_sigma_sqrt_dt[31:0];
                end
                ADDR_SEED_DATA_0: begin
                    rdata <= int_seed[31:0];
                end
                ADDR_RESEED_DATA_0: begin
                    rdata <= int_reseed[0:0];
                end
            endcase
        end
    end
end


//------------------------Register logic-----------------
assign interrupt         = int_interrupt;
assign ap_start          = int_ap_start;
assign task_ap_done      = (ap_done && !auto_restart_status) || auto_restart_done;
assign task_ap_ready     = ap_ready && !int_auto_restart;
assign auto_restart_done = auto_restart_status && (ap_idle && !int_ap_idle);
assign row_ptr_ddr       = int_row_ptr_ddr;
assign col_idx_ddr       = int_col_idx_ddr;
assign u_ddr             = int_u_ddr;
assign v_ddr             = int_v_ddr;
assign neuron_count      = int_neuron_count;
assign edge_count        = int_edge_count;
assign timesteps         = int_timesteps;
assign command           = int_command;
assign dt                = int_dt;
assign a                 = int_a;
assign J                 = int_J;
assign inv_e             = int_inv_e;
assign sigma_sqrt_dt     = int_sigma_sqrt_dt;
assign seed              = int_seed;
assign reseed            = int_reseed;
// int_interrupt
always @(posedge ACLK) begin
    if (ARESET)
        int_interrupt <= 1'b0;
    else if (ACLK_EN) begin
        if (int_gie && (|int_isr))
            int_interrupt <= 1'b1;
        else
            int_interrupt <= 1'b0;
    end
end

// int_ap_start
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_start <= 1'b0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_AP_CTRL && WSTRB[0] && WDATA[0])
            int_ap_start <= 1'b1;
        else if (ap_ready)
            int_ap_start <= int_auto_restart; // clear on handshake/auto restart
    end
end

// int_ap_done
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_done <= 1'b0;
    else if (ACLK_EN) begin
            int_ap_done <= ap_done;
    end
end

// int_task_ap_done
always @(posedge ACLK) begin
    if (ARESET)
        int_task_ap_done <= 1'b0;
    else if (ACLK_EN) begin
        if (task_ap_done)
            int_task_ap_done <= 1'b1;
        else if (ar_hs && raddr == ADDR_AP_CTRL)
            int_task_ap_done <= 1'b0; // clear on read
    end
end

// int_ap_idle
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_idle <= 1'b0;
    else if (ACLK_EN) begin
            int_ap_idle <= ap_idle;
    end
end

// int_ap_ready
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_ready <= 1'b0;
    else if (ACLK_EN) begin
        if (task_ap_ready)
            int_ap_ready <= 1'b1;
        else if (ar_hs && raddr == ADDR_AP_CTRL)
            int_ap_ready <= 1'b0;
    end
end

// int_auto_restart
always @(posedge ACLK) begin
    if (ARESET)
        int_auto_restart <= 1'b0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_AP_CTRL && WSTRB[0])
            int_auto_restart <= WDATA[7];
    end
end

// auto_restart_status
always @(posedge ACLK) begin
    if (ARESET)
        auto_restart_status <= 1'b0;
    else if (ACLK_EN) begin
        if (int_auto_restart)
            auto_restart_status <= 1'b1;
        else if (ap_idle)
            auto_restart_status <= 1'b0;
    end
end

// int_gie
always @(posedge ACLK) begin
    if (ARESET)
        int_gie <= 1'b0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_GIE && WSTRB[0])
            int_gie <= WDATA[0];
    end
end

// int_ier
always @(posedge ACLK) begin
    if (ARESET)
        int_ier <= 1'b0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IER && WSTRB[0])
            int_ier <= WDATA[1:0];
    end
end

// int_isr[0]
always @(posedge ACLK) begin
    if (ARESET)
        int_isr[0] <= 1'b0;
    else if (ACLK_EN) begin
        if (int_ier[0] & ap_done)
            int_isr[0] <= 1'b1;
        else if (w_hs && waddr == ADDR_ISR && WSTRB[0])
            int_isr[0] <= int_isr[0] ^ WDATA[0]; // toggle on write
    end
end

// int_isr[1]
always @(posedge ACLK) begin
    if (ARESET)
        int_isr[1] <= 1'b0;
    else if (ACLK_EN) begin
        if (int_ier[1] & ap_ready)
            int_isr[1] <= 1'b1;
        else if (w_hs && waddr == ADDR_ISR && WSTRB[0])
            int_isr[1] <= int_isr[1] ^ WDATA[1]; // toggle on write
    end
end

// int_row_ptr_ddr[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_row_ptr_ddr[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROW_PTR_DDR_DATA_0)
            int_row_ptr_ddr[31:0] <= (WDATA[31:0] & wmask) | (int_row_ptr_ddr[31:0] & ~wmask);
    end
end

// int_row_ptr_ddr[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_row_ptr_ddr[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_ROW_PTR_DDR_DATA_1)
            int_row_ptr_ddr[63:32] <= (WDATA[31:0] & wmask) | (int_row_ptr_ddr[63:32] & ~wmask);
    end
end

// int_col_idx_ddr[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_col_idx_ddr[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_COL_IDX_DDR_DATA_0)
            int_col_idx_ddr[31:0] <= (WDATA[31:0] & wmask) | (int_col_idx_ddr[31:0] & ~wmask);
    end
end

// int_col_idx_ddr[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_col_idx_ddr[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_COL_IDX_DDR_DATA_1)
            int_col_idx_ddr[63:32] <= (WDATA[31:0] & wmask) | (int_col_idx_ddr[63:32] & ~wmask);
    end
end

// int_u_ddr[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_u_ddr[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_U_DDR_DATA_0)
            int_u_ddr[31:0] <= (WDATA[31:0] & wmask) | (int_u_ddr[31:0] & ~wmask);
    end
end

// int_u_ddr[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_u_ddr[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_U_DDR_DATA_1)
            int_u_ddr[63:32] <= (WDATA[31:0] & wmask) | (int_u_ddr[63:32] & ~wmask);
    end
end

// int_v_ddr[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_v_ddr[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_V_DDR_DATA_0)
            int_v_ddr[31:0] <= (WDATA[31:0] & wmask) | (int_v_ddr[31:0] & ~wmask);
    end
end

// int_v_ddr[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_v_ddr[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_V_DDR_DATA_1)
            int_v_ddr[63:32] <= (WDATA[31:0] & wmask) | (int_v_ddr[63:32] & ~wmask);
    end
end

// int_neuron_count[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_neuron_count[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_NEURON_COUNT_DATA_0)
            int_neuron_count[31:0] <= (WDATA[31:0] & wmask) | (int_neuron_count[31:0] & ~wmask);
    end
end

// int_edge_count[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_edge_count[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_EDGE_COUNT_DATA_0)
            int_edge_count[31:0] <= (WDATA[31:0] & wmask) | (int_edge_count[31:0] & ~wmask);
    end
end

// int_timesteps[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_timesteps[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_TIMESTEPS_DATA_0)
            int_timesteps[31:0] <= (WDATA[31:0] & wmask) | (int_timesteps[31:0] & ~wmask);
    end
end

// int_command[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_command[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_COMMAND_DATA_0)
            int_command[31:0] <= (WDATA[31:0] & wmask) | (int_command[31:0] & ~wmask);
    end
end

// int_dt[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_dt[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_DT_DATA_0)
            int_dt[31:0] <= (WDATA[31:0] & wmask) | (int_dt[31:0] & ~wmask);
    end
end

// int_a[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_a[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_A_DATA_0)
            int_a[31:0] <= (WDATA[31:0] & wmask) | (int_a[31:0] & ~wmask);
    end
end

// int_J[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_J[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_J_DATA_0)
            int_J[31:0] <= (WDATA[31:0] & wmask) | (int_J[31:0] & ~wmask);
    end
end

// int_inv_e[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_inv_e[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_INV_E_DATA_0)
            int_inv_e[31:0] <= (WDATA[31:0] & wmask) | (int_inv_e[31:0] & ~wmask);
    end
end

// int_sigma_sqrt_dt[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_sigma_sqrt_dt[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_SIGMA_SQRT_DT_DATA_0)
            int_sigma_sqrt_dt[31:0] <= (WDATA[31:0] & wmask) | (int_sigma_sqrt_dt[31:0] & ~wmask);
    end
end

// int_seed[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_seed[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_SEED_DATA_0)
            int_seed[31:0] <= (WDATA[31:0] & wmask) | (int_seed[31:0] & ~wmask);
    end
end

// int_reseed[0:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_reseed[0:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_RESEED_DATA_0)
            int_reseed[0:0] <= (WDATA[31:0] & wmask) | (int_reseed[0:0] & ~wmask);
    end
end

//synthesis translate_off
always @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (int_gie & ~int_isr[0] & int_ier[0] & ap_done)
            $display ("// Interrupt Monitor : interrupt for ap_done detected @ \"%0t\"", $time);
        if (int_gie & ~int_isr[1] & int_ier[1] & ap_ready)
            $display ("// Interrupt Monitor : interrupt for ap_ready detected @ \"%0t\"", $time);
    end
end
//synthesis translate_on

//------------------------Memory logic-------------------

endmodule
