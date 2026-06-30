// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xaccelerator_net.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XAccelerator_net_CfgInitialize(XAccelerator_net *InstancePtr, XAccelerator_net_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XAccelerator_net_Start(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_AP_CTRL) & 0x80;
    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XAccelerator_net_IsDone(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XAccelerator_net_IsIdle(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XAccelerator_net_IsReady(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XAccelerator_net_EnableAutoRestart(XAccelerator_net *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XAccelerator_net_DisableAutoRestart(XAccelerator_net *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_AP_CTRL, 0);
}

void XAccelerator_net_Set_row_ptr_ddr(XAccelerator_net *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_ROW_PTR_DDR_DATA, (u32)(Data));
    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_ROW_PTR_DDR_DATA + 4, (u32)(Data >> 32));
}

u64 XAccelerator_net_Get_row_ptr_ddr(XAccelerator_net *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_ROW_PTR_DDR_DATA);
    Data += (u64)XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_ROW_PTR_DDR_DATA + 4) << 32;
    return Data;
}

void XAccelerator_net_Set_col_idx_ddr(XAccelerator_net *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_COL_IDX_DDR_DATA, (u32)(Data));
    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_COL_IDX_DDR_DATA + 4, (u32)(Data >> 32));
}

u64 XAccelerator_net_Get_col_idx_ddr(XAccelerator_net *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_COL_IDX_DDR_DATA);
    Data += (u64)XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_COL_IDX_DDR_DATA + 4) << 32;
    return Data;
}

void XAccelerator_net_Set_u_ddr(XAccelerator_net *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_U_DDR_DATA, (u32)(Data));
    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_U_DDR_DATA + 4, (u32)(Data >> 32));
}

u64 XAccelerator_net_Get_u_ddr(XAccelerator_net *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_U_DDR_DATA);
    Data += (u64)XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_U_DDR_DATA + 4) << 32;
    return Data;
}

void XAccelerator_net_Set_v_ddr(XAccelerator_net *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_V_DDR_DATA, (u32)(Data));
    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_V_DDR_DATA + 4, (u32)(Data >> 32));
}

u64 XAccelerator_net_Get_v_ddr(XAccelerator_net *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_V_DDR_DATA);
    Data += (u64)XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_V_DDR_DATA + 4) << 32;
    return Data;
}

void XAccelerator_net_Set_neuron_count(XAccelerator_net *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_NEURON_COUNT_DATA, Data);
}

u32 XAccelerator_net_Get_neuron_count(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_NEURON_COUNT_DATA);
    return Data;
}

void XAccelerator_net_Set_edge_count(XAccelerator_net *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_EDGE_COUNT_DATA, Data);
}

u32 XAccelerator_net_Get_edge_count(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_EDGE_COUNT_DATA);
    return Data;
}

void XAccelerator_net_Set_timesteps(XAccelerator_net *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_TIMESTEPS_DATA, Data);
}

u32 XAccelerator_net_Get_timesteps(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_TIMESTEPS_DATA);
    return Data;
}

void XAccelerator_net_Set_command(XAccelerator_net *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_COMMAND_DATA, Data);
}

u32 XAccelerator_net_Get_command(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_COMMAND_DATA);
    return Data;
}

void XAccelerator_net_Set_dt(XAccelerator_net *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_DT_DATA, Data);
}

u32 XAccelerator_net_Get_dt(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_DT_DATA);
    return Data;
}

void XAccelerator_net_Set_a(XAccelerator_net *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_A_DATA, Data);
}

u32 XAccelerator_net_Get_a(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_A_DATA);
    return Data;
}

void XAccelerator_net_Set_J(XAccelerator_net *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_J_DATA, Data);
}

u32 XAccelerator_net_Get_J(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_J_DATA);
    return Data;
}

void XAccelerator_net_Set_inv_e(XAccelerator_net *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_INV_E_DATA, Data);
}

u32 XAccelerator_net_Get_inv_e(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_INV_E_DATA);
    return Data;
}

void XAccelerator_net_Set_sigma_sqrt_dt(XAccelerator_net *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_SIGMA_SQRT_DT_DATA, Data);
}

u32 XAccelerator_net_Get_sigma_sqrt_dt(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_SIGMA_SQRT_DT_DATA);
    return Data;
}

void XAccelerator_net_Set_seed(XAccelerator_net *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_SEED_DATA, Data);
}

u32 XAccelerator_net_Get_seed(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_SEED_DATA);
    return Data;
}

void XAccelerator_net_Set_reseed(XAccelerator_net *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_RESEED_DATA, Data);
}

u32 XAccelerator_net_Get_reseed(XAccelerator_net *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_RESEED_DATA);
    return Data;
}

void XAccelerator_net_InterruptGlobalEnable(XAccelerator_net *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_GIE, 1);
}

void XAccelerator_net_InterruptGlobalDisable(XAccelerator_net *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_GIE, 0);
}

void XAccelerator_net_InterruptEnable(XAccelerator_net *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_IER);
    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_IER, Register | Mask);
}

void XAccelerator_net_InterruptDisable(XAccelerator_net *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_IER);
    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_IER, Register & (~Mask));
}

void XAccelerator_net_InterruptClear(XAccelerator_net *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XAccelerator_net_WriteReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_ISR, Mask);
}

u32 XAccelerator_net_InterruptGetEnabled(XAccelerator_net *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_IER);
}

u32 XAccelerator_net_InterruptGetStatus(XAccelerator_net *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XAccelerator_net_ReadReg(InstancePtr->Control_BaseAddress, XACCELERATOR_NET_CONTROL_ADDR_ISR);
}

