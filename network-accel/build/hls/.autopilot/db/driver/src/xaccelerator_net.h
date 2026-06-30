// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XACCELERATOR_NET_H
#define XACCELERATOR_NET_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xaccelerator_net_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
#ifdef SDT
    char *Name;
#else
    u16 DeviceId;
#endif
    u64 Control_BaseAddress;
} XAccelerator_net_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XAccelerator_net;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XAccelerator_net_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XAccelerator_net_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XAccelerator_net_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XAccelerator_net_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifdef SDT
int XAccelerator_net_Initialize(XAccelerator_net *InstancePtr, UINTPTR BaseAddress);
XAccelerator_net_Config* XAccelerator_net_LookupConfig(UINTPTR BaseAddress);
#else
int XAccelerator_net_Initialize(XAccelerator_net *InstancePtr, u16 DeviceId);
XAccelerator_net_Config* XAccelerator_net_LookupConfig(u16 DeviceId);
#endif
int XAccelerator_net_CfgInitialize(XAccelerator_net *InstancePtr, XAccelerator_net_Config *ConfigPtr);
#else
int XAccelerator_net_Initialize(XAccelerator_net *InstancePtr, const char* InstanceName);
int XAccelerator_net_Release(XAccelerator_net *InstancePtr);
#endif

void XAccelerator_net_Start(XAccelerator_net *InstancePtr);
u32 XAccelerator_net_IsDone(XAccelerator_net *InstancePtr);
u32 XAccelerator_net_IsIdle(XAccelerator_net *InstancePtr);
u32 XAccelerator_net_IsReady(XAccelerator_net *InstancePtr);
void XAccelerator_net_EnableAutoRestart(XAccelerator_net *InstancePtr);
void XAccelerator_net_DisableAutoRestart(XAccelerator_net *InstancePtr);

void XAccelerator_net_Set_row_ptr_ddr(XAccelerator_net *InstancePtr, u64 Data);
u64 XAccelerator_net_Get_row_ptr_ddr(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_col_idx_ddr(XAccelerator_net *InstancePtr, u64 Data);
u64 XAccelerator_net_Get_col_idx_ddr(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_u_ddr(XAccelerator_net *InstancePtr, u64 Data);
u64 XAccelerator_net_Get_u_ddr(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_v_ddr(XAccelerator_net *InstancePtr, u64 Data);
u64 XAccelerator_net_Get_v_ddr(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_neuron_count(XAccelerator_net *InstancePtr, u32 Data);
u32 XAccelerator_net_Get_neuron_count(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_edge_count(XAccelerator_net *InstancePtr, u32 Data);
u32 XAccelerator_net_Get_edge_count(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_timesteps(XAccelerator_net *InstancePtr, u32 Data);
u32 XAccelerator_net_Get_timesteps(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_command(XAccelerator_net *InstancePtr, u32 Data);
u32 XAccelerator_net_Get_command(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_dt(XAccelerator_net *InstancePtr, u32 Data);
u32 XAccelerator_net_Get_dt(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_a(XAccelerator_net *InstancePtr, u32 Data);
u32 XAccelerator_net_Get_a(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_J(XAccelerator_net *InstancePtr, u32 Data);
u32 XAccelerator_net_Get_J(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_inv_e(XAccelerator_net *InstancePtr, u32 Data);
u32 XAccelerator_net_Get_inv_e(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_sigma_sqrt_dt(XAccelerator_net *InstancePtr, u32 Data);
u32 XAccelerator_net_Get_sigma_sqrt_dt(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_seed(XAccelerator_net *InstancePtr, u32 Data);
u32 XAccelerator_net_Get_seed(XAccelerator_net *InstancePtr);
void XAccelerator_net_Set_reseed(XAccelerator_net *InstancePtr, u32 Data);
u32 XAccelerator_net_Get_reseed(XAccelerator_net *InstancePtr);

void XAccelerator_net_InterruptGlobalEnable(XAccelerator_net *InstancePtr);
void XAccelerator_net_InterruptGlobalDisable(XAccelerator_net *InstancePtr);
void XAccelerator_net_InterruptEnable(XAccelerator_net *InstancePtr, u32 Mask);
void XAccelerator_net_InterruptDisable(XAccelerator_net *InstancePtr, u32 Mask);
void XAccelerator_net_InterruptClear(XAccelerator_net *InstancePtr, u32 Mask);
u32 XAccelerator_net_InterruptGetEnabled(XAccelerator_net *InstancePtr);
u32 XAccelerator_net_InterruptGetStatus(XAccelerator_net *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
