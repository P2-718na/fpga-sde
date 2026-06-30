// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#ifdef SDT
#include "xparameters.h"
#endif
#include "xaccelerator_net.h"

extern XAccelerator_net_Config XAccelerator_net_ConfigTable[];

#ifdef SDT
XAccelerator_net_Config *XAccelerator_net_LookupConfig(UINTPTR BaseAddress) {
	XAccelerator_net_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XAccelerator_net_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XAccelerator_net_ConfigTable[Index].Control_BaseAddress == BaseAddress) {
			ConfigPtr = &XAccelerator_net_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XAccelerator_net_Initialize(XAccelerator_net *InstancePtr, UINTPTR BaseAddress) {
	XAccelerator_net_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XAccelerator_net_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XAccelerator_net_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XAccelerator_net_Config *XAccelerator_net_LookupConfig(u16 DeviceId) {
	XAccelerator_net_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XACCELERATOR_NET_NUM_INSTANCES; Index++) {
		if (XAccelerator_net_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XAccelerator_net_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XAccelerator_net_Initialize(XAccelerator_net *InstancePtr, u16 DeviceId) {
	XAccelerator_net_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XAccelerator_net_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XAccelerator_net_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

