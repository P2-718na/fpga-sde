set moduleName accelerator_net_Pipeline_COMPUTE_DEGREE
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
set C_modelName {accelerator_net_Pipeline_COMPUTE_DEGREE}
set C_modelType { void 0 }
set ap_memory_interface_dict [dict create]
dict set ap_memory_interface_dict row_ptr { MEM_WIDTH 32 MEM_SIZE 8196 MASTER_TYPE BRAM_CTRL MEM_ADDRESS_MODE WORD_ADDRESS PACKAGE_IO port READ_LATENCY 1 }
dict set ap_memory_interface_dict degree { MEM_WIDTH 8 MEM_SIZE 2048 MASTER_TYPE BRAM_CTRL MEM_ADDRESS_MODE WORD_ADDRESS PACKAGE_IO port READ_LATENCY 0 }
set C_modelArgList {
	{ neuron_count int 32 regular  }
	{ row_ptr int 32 regular {array 2049 { 1 1 } 1 1 } {global 0}  }
	{ degree int 8 regular {array 2048 { 0 3 } 0 1 } {global 1}  }
}
set hasAXIMCache 0
set l_AXIML2Cache [list]
set AXIMCacheInstDict [dict create]
set C_modelArgMapList {[ 
	{ "Name" : "neuron_count", "interface" : "wire", "bitwidth" : 32, "direction" : "READONLY"} , 
 	{ "Name" : "row_ptr", "interface" : "memory", "bitwidth" : 32, "direction" : "READONLY", "extern" : 0} , 
 	{ "Name" : "degree", "interface" : "memory", "bitwidth" : 8, "direction" : "WRITEONLY", "extern" : 0} ]}
# RTL Port declarations: 
set portNum 17
set portList { 
	{ ap_clk sc_in sc_logic 1 clock -1 } 
	{ ap_rst sc_in sc_logic 1 reset -1 active_high_sync } 
	{ ap_start sc_in sc_logic 1 start -1 } 
	{ ap_done sc_out sc_logic 1 predone -1 } 
	{ ap_idle sc_out sc_logic 1 done -1 } 
	{ ap_ready sc_out sc_logic 1 ready -1 } 
	{ neuron_count sc_in sc_lv 32 signal 0 } 
	{ row_ptr_address0 sc_out sc_lv 12 signal 1 } 
	{ row_ptr_ce0 sc_out sc_logic 1 signal 1 } 
	{ row_ptr_q0 sc_in sc_lv 32 signal 1 } 
	{ row_ptr_address1 sc_out sc_lv 12 signal 1 } 
	{ row_ptr_ce1 sc_out sc_logic 1 signal 1 } 
	{ row_ptr_q1 sc_in sc_lv 32 signal 1 } 
	{ degree_address0 sc_out sc_lv 11 signal 2 } 
	{ degree_ce0 sc_out sc_logic 1 signal 2 } 
	{ degree_we0 sc_out sc_logic 1 signal 2 } 
	{ degree_d0 sc_out sc_lv 8 signal 2 } 
}
set NewPortList {[ 
	{ "name": "ap_clk", "direction": "in", "datatype": "sc_logic", "bitwidth":1, "type": "clock", "bundle":{"name": "ap_clk", "role": "default" }} , 
 	{ "name": "ap_rst", "direction": "in", "datatype": "sc_logic", "bitwidth":1, "type": "reset", "bundle":{"name": "ap_rst", "role": "default" }} , 
 	{ "name": "ap_start", "direction": "in", "datatype": "sc_logic", "bitwidth":1, "type": "start", "bundle":{"name": "ap_start", "role": "default" }} , 
 	{ "name": "ap_done", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "predone", "bundle":{"name": "ap_done", "role": "default" }} , 
 	{ "name": "ap_idle", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "done", "bundle":{"name": "ap_idle", "role": "default" }} , 
 	{ "name": "ap_ready", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "ready", "bundle":{"name": "ap_ready", "role": "default" }} , 
 	{ "name": "neuron_count", "direction": "in", "datatype": "sc_lv", "bitwidth":32, "type": "signal", "bundle":{"name": "neuron_count", "role": "default" }} , 
 	{ "name": "row_ptr_address0", "direction": "out", "datatype": "sc_lv", "bitwidth":12, "type": "signal", "bundle":{"name": "row_ptr", "role": "address0" }} , 
 	{ "name": "row_ptr_ce0", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "signal", "bundle":{"name": "row_ptr", "role": "ce0" }} , 
 	{ "name": "row_ptr_q0", "direction": "in", "datatype": "sc_lv", "bitwidth":32, "type": "signal", "bundle":{"name": "row_ptr", "role": "q0" }} , 
 	{ "name": "row_ptr_address1", "direction": "out", "datatype": "sc_lv", "bitwidth":12, "type": "signal", "bundle":{"name": "row_ptr", "role": "address1" }} , 
 	{ "name": "row_ptr_ce1", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "signal", "bundle":{"name": "row_ptr", "role": "ce1" }} , 
 	{ "name": "row_ptr_q1", "direction": "in", "datatype": "sc_lv", "bitwidth":32, "type": "signal", "bundle":{"name": "row_ptr", "role": "q1" }} , 
 	{ "name": "degree_address0", "direction": "out", "datatype": "sc_lv", "bitwidth":11, "type": "signal", "bundle":{"name": "degree", "role": "address0" }} , 
 	{ "name": "degree_ce0", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "signal", "bundle":{"name": "degree", "role": "ce0" }} , 
 	{ "name": "degree_we0", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "signal", "bundle":{"name": "degree", "role": "we0" }} , 
 	{ "name": "degree_d0", "direction": "out", "datatype": "sc_lv", "bitwidth":8, "type": "signal", "bundle":{"name": "degree", "role": "d0" }}  ]}

set ArgLastReadFirstWriteLatency {
	accelerator_net_Pipeline_COMPUTE_DEGREE {
		neuron_count {Type I LastRead 0 FirstWrite -1}
		row_ptr {Type I LastRead 1 FirstWrite -1}
		degree {Type O LastRead -1 FirstWrite 2}}}

set hasDtUnsupportedChannel 0

set PerformanceInfo {[
	{"Name" : "Latency", "Min" : "-1", "Max" : "-1"}
	, {"Name" : "Interval", "Min" : "0", "Max" : "0"}
]}

set PipelineEnableSignalInfo {[
	{"Pipeline" : "0", "EnableSignal" : "ap_enable_pp0"}
]}

set Spec2ImplPortList { 
	neuron_count { ap_none {  { neuron_count in_data 0 32 } } }
	row_ptr { ap_memory {  { row_ptr_address0 mem_address 1 12 }  { row_ptr_ce0 mem_ce 1 1 }  { row_ptr_q0 mem_dout 0 32 }  { row_ptr_address1 MemPortADDR2 1 12 }  { row_ptr_ce1 MemPortCE2 1 1 }  { row_ptr_q1 MemPortDOUT2 0 32 } } }
	degree { ap_memory {  { degree_address0 mem_address 1 11 }  { degree_ce0 mem_ce 1 1 }  { degree_we0 mem_we 1 1 }  { degree_d0 mem_din 1 8 } } }
}
