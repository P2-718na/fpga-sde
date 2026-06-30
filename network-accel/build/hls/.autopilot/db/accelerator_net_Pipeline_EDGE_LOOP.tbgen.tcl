set moduleName accelerator_net_Pipeline_EDGE_LOOP
set isTopModule 0
set isCombinational 0
set isDatapathOnly 0
set isPipelined 1
set isPipelined_legacy 1
set pipeline_type loop_auto_rewind
set FunctionProtocol ap_ctrl_hs
set restart_counter_num 0
set isOneStateSeq 0
set ProfileFlag 0
set StallSigGenFlag 0
set isEnableWaveformDebug 1
set hasInterrupt 0
set DLRegFirstOffset 0
set DLRegItemOffset 0
set svuvm_can_support 1
set cdfgNum 10
set C_modelName {accelerator_net_Pipeline_EDGE_LOOP}
set C_modelType { void 0 }
set ap_memory_interface_dict [dict create]
dict set ap_memory_interface_dict col_idx { MEM_WIDTH 11 MEM_SIZE 65536 MASTER_TYPE BRAM_CTRL MEM_ADDRESS_MODE WORD_ADDRESS PACKAGE_IO port READ_LATENCY 1 }
dict set ap_memory_interface_dict u_a { MEM_WIDTH 32 MEM_SIZE 8192 MASTER_TYPE BRAM_CTRL MEM_ADDRESS_MODE WORD_ADDRESS PACKAGE_IO port READ_LATENCY 1 }
set C_modelArgList {
	{ sext_ln241 int 32 regular  }
	{ row_ptr_load int 32 regular  }
	{ inflow_out int 32 regular {pointer 1}  }
	{ col_idx int 11 regular {array 32768 { 1 3 } 1 1 } {global 0}  }
	{ u_a int 32 regular {array 2048 { 1 3 } 1 1 } {global 0}  }
}
set hasAXIMCache 0
set l_AXIML2Cache [list]
set AXIMCacheInstDict [dict create]
set C_modelArgMapList {[ 
	{ "Name" : "sext_ln241", "interface" : "wire", "bitwidth" : 32, "direction" : "READONLY"} , 
 	{ "Name" : "row_ptr_load", "interface" : "wire", "bitwidth" : 32, "direction" : "READONLY"} , 
 	{ "Name" : "inflow_out", "interface" : "wire", "bitwidth" : 32, "direction" : "WRITEONLY"} , 
 	{ "Name" : "col_idx", "interface" : "memory", "bitwidth" : 11, "direction" : "READONLY", "extern" : 0} , 
 	{ "Name" : "u_a", "interface" : "memory", "bitwidth" : 32, "direction" : "READONLY", "extern" : 0} ]}
# RTL Port declarations: 
set portNum 16
set portList { 
	{ ap_clk sc_in sc_logic 1 clock -1 } 
	{ ap_rst sc_in sc_logic 1 reset -1 active_high_sync } 
	{ ap_start sc_in sc_logic 1 start -1 } 
	{ ap_done sc_out sc_logic 1 predone -1 } 
	{ ap_idle sc_out sc_logic 1 done -1 } 
	{ ap_ready sc_out sc_logic 1 ready -1 } 
	{ sext_ln241 sc_in sc_lv 32 signal 0 } 
	{ row_ptr_load sc_in sc_lv 32 signal 1 } 
	{ inflow_out sc_out sc_lv 32 signal 2 } 
	{ inflow_out_ap_vld sc_out sc_logic 1 outvld 2 } 
	{ col_idx_address0 sc_out sc_lv 15 signal 3 } 
	{ col_idx_ce0 sc_out sc_logic 1 signal 3 } 
	{ col_idx_q0 sc_in sc_lv 11 signal 3 } 
	{ u_a_address0 sc_out sc_lv 11 signal 4 } 
	{ u_a_ce0 sc_out sc_logic 1 signal 4 } 
	{ u_a_q0 sc_in sc_lv 32 signal 4 } 
}
set NewPortList {[ 
	{ "name": "ap_clk", "direction": "in", "datatype": "sc_logic", "bitwidth":1, "type": "clock", "bundle":{"name": "ap_clk", "role": "default" }} , 
 	{ "name": "ap_rst", "direction": "in", "datatype": "sc_logic", "bitwidth":1, "type": "reset", "bundle":{"name": "ap_rst", "role": "default" }} , 
 	{ "name": "ap_start", "direction": "in", "datatype": "sc_logic", "bitwidth":1, "type": "start", "bundle":{"name": "ap_start", "role": "default" }} , 
 	{ "name": "ap_done", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "predone", "bundle":{"name": "ap_done", "role": "default" }} , 
 	{ "name": "ap_idle", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "done", "bundle":{"name": "ap_idle", "role": "default" }} , 
 	{ "name": "ap_ready", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "ready", "bundle":{"name": "ap_ready", "role": "default" }} , 
 	{ "name": "sext_ln241", "direction": "in", "datatype": "sc_lv", "bitwidth":32, "type": "signal", "bundle":{"name": "sext_ln241", "role": "default" }} , 
 	{ "name": "row_ptr_load", "direction": "in", "datatype": "sc_lv", "bitwidth":32, "type": "signal", "bundle":{"name": "row_ptr_load", "role": "default" }} , 
 	{ "name": "inflow_out", "direction": "out", "datatype": "sc_lv", "bitwidth":32, "type": "signal", "bundle":{"name": "inflow_out", "role": "default" }} , 
 	{ "name": "inflow_out_ap_vld", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "outvld", "bundle":{"name": "inflow_out", "role": "ap_vld" }} , 
 	{ "name": "col_idx_address0", "direction": "out", "datatype": "sc_lv", "bitwidth":15, "type": "signal", "bundle":{"name": "col_idx", "role": "address0" }} , 
 	{ "name": "col_idx_ce0", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "signal", "bundle":{"name": "col_idx", "role": "ce0" }} , 
 	{ "name": "col_idx_q0", "direction": "in", "datatype": "sc_lv", "bitwidth":11, "type": "signal", "bundle":{"name": "col_idx", "role": "q0" }} , 
 	{ "name": "u_a_address0", "direction": "out", "datatype": "sc_lv", "bitwidth":11, "type": "signal", "bundle":{"name": "u_a", "role": "address0" }} , 
 	{ "name": "u_a_ce0", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "signal", "bundle":{"name": "u_a", "role": "ce0" }} , 
 	{ "name": "u_a_q0", "direction": "in", "datatype": "sc_lv", "bitwidth":32, "type": "signal", "bundle":{"name": "u_a", "role": "q0" }}  ]}

set ArgLastReadFirstWriteLatency {
	accelerator_net_Pipeline_EDGE_LOOP {
		sext_ln241 {Type I LastRead 0 FirstWrite -1}
		row_ptr_load {Type I LastRead 0 FirstWrite -1}
		inflow_out {Type O LastRead -1 FirstWrite 3}
		col_idx {Type I LastRead 1 FirstWrite -1}
		u_a {Type I LastRead 2 FirstWrite -1}}}

set hasDtUnsupportedChannel 0

set PerformanceInfo {[
	{"Name" : "Latency", "Min" : "-1", "Max" : "-1"}
	, {"Name" : "Interval", "Min" : "0", "Max" : "0"}
]}

set PipelineEnableSignalInfo {[
	{"Pipeline" : "0", "EnableSignal" : "ap_enable_pp0"}
]}

set Spec2ImplPortList { 
	sext_ln241 { ap_none {  { sext_ln241 in_data 0 32 } } }
	row_ptr_load { ap_none {  { row_ptr_load in_data 0 32 } } }
	inflow_out { ap_vld {  { inflow_out out_data 1 32 }  { inflow_out_ap_vld out_vld 1 1 } } }
	col_idx { ap_memory {  { col_idx_address0 mem_address 1 15 }  { col_idx_ce0 mem_ce 1 1 }  { col_idx_q0 mem_dout 0 11 } } }
	u_a { ap_memory {  { u_a_address0 mem_address 1 11 }  { u_a_ce0 mem_ce 1 1 }  { u_a_q0 mem_dout 0 32 } } }
}
