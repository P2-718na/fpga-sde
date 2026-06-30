set moduleName accelerator_net_Pipeline_SWAP
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
set C_modelName {accelerator_net_Pipeline_SWAP}
set C_modelType { void 0 }
set ap_memory_interface_dict [dict create]
dict set ap_memory_interface_dict u_b { MEM_WIDTH 32 MEM_SIZE 8192 MASTER_TYPE BRAM_CTRL MEM_ADDRESS_MODE WORD_ADDRESS PACKAGE_IO port READ_LATENCY 1 }
dict set ap_memory_interface_dict u_a { MEM_WIDTH 32 MEM_SIZE 8192 MASTER_TYPE BRAM_CTRL MEM_ADDRESS_MODE WORD_ADDRESS PACKAGE_IO port READ_LATENCY 0 }
dict set ap_memory_interface_dict v_b { MEM_WIDTH 32 MEM_SIZE 8192 MASTER_TYPE BRAM_CTRL MEM_ADDRESS_MODE WORD_ADDRESS PACKAGE_IO port READ_LATENCY 1 }
dict set ap_memory_interface_dict v_a { MEM_WIDTH 32 MEM_SIZE 8192 MASTER_TYPE BRAM_CTRL MEM_ADDRESS_MODE WORD_ADDRESS PACKAGE_IO port READ_LATENCY 0 }
set C_modelArgList {
	{ neuron_count int 31 regular  }
	{ u_b int 32 regular {array 2048 { 1 3 } 1 1 } {global 0}  }
	{ u_a int 32 regular {array 2048 { 0 3 } 0 1 } {global 1}  }
	{ v_b int 32 regular {array 2048 { 1 3 } 1 1 } {global 0}  }
	{ v_a int 32 regular {array 2048 { 0 3 } 0 1 } {global 1}  }
}
set hasAXIMCache 0
set l_AXIML2Cache [list]
set AXIMCacheInstDict [dict create]
set C_modelArgMapList {[ 
	{ "Name" : "neuron_count", "interface" : "wire", "bitwidth" : 31, "direction" : "READONLY"} , 
 	{ "Name" : "u_b", "interface" : "memory", "bitwidth" : 32, "direction" : "READONLY", "extern" : 0} , 
 	{ "Name" : "u_a", "interface" : "memory", "bitwidth" : 32, "direction" : "WRITEONLY", "extern" : 0} , 
 	{ "Name" : "v_b", "interface" : "memory", "bitwidth" : 32, "direction" : "READONLY", "extern" : 0} , 
 	{ "Name" : "v_a", "interface" : "memory", "bitwidth" : 32, "direction" : "WRITEONLY", "extern" : 0} ]}
# RTL Port declarations: 
set portNum 21
set portList { 
	{ ap_clk sc_in sc_logic 1 clock -1 } 
	{ ap_rst sc_in sc_logic 1 reset -1 active_high_sync } 
	{ ap_start sc_in sc_logic 1 start -1 } 
	{ ap_done sc_out sc_logic 1 predone -1 } 
	{ ap_idle sc_out sc_logic 1 done -1 } 
	{ ap_ready sc_out sc_logic 1 ready -1 } 
	{ neuron_count sc_in sc_lv 31 signal 0 } 
	{ u_b_address0 sc_out sc_lv 11 signal 1 } 
	{ u_b_ce0 sc_out sc_logic 1 signal 1 } 
	{ u_b_q0 sc_in sc_lv 32 signal 1 } 
	{ u_a_address0 sc_out sc_lv 11 signal 2 } 
	{ u_a_ce0 sc_out sc_logic 1 signal 2 } 
	{ u_a_we0 sc_out sc_logic 1 signal 2 } 
	{ u_a_d0 sc_out sc_lv 32 signal 2 } 
	{ v_b_address0 sc_out sc_lv 11 signal 3 } 
	{ v_b_ce0 sc_out sc_logic 1 signal 3 } 
	{ v_b_q0 sc_in sc_lv 32 signal 3 } 
	{ v_a_address0 sc_out sc_lv 11 signal 4 } 
	{ v_a_ce0 sc_out sc_logic 1 signal 4 } 
	{ v_a_we0 sc_out sc_logic 1 signal 4 } 
	{ v_a_d0 sc_out sc_lv 32 signal 4 } 
}
set NewPortList {[ 
	{ "name": "ap_clk", "direction": "in", "datatype": "sc_logic", "bitwidth":1, "type": "clock", "bundle":{"name": "ap_clk", "role": "default" }} , 
 	{ "name": "ap_rst", "direction": "in", "datatype": "sc_logic", "bitwidth":1, "type": "reset", "bundle":{"name": "ap_rst", "role": "default" }} , 
 	{ "name": "ap_start", "direction": "in", "datatype": "sc_logic", "bitwidth":1, "type": "start", "bundle":{"name": "ap_start", "role": "default" }} , 
 	{ "name": "ap_done", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "predone", "bundle":{"name": "ap_done", "role": "default" }} , 
 	{ "name": "ap_idle", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "done", "bundle":{"name": "ap_idle", "role": "default" }} , 
 	{ "name": "ap_ready", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "ready", "bundle":{"name": "ap_ready", "role": "default" }} , 
 	{ "name": "neuron_count", "direction": "in", "datatype": "sc_lv", "bitwidth":31, "type": "signal", "bundle":{"name": "neuron_count", "role": "default" }} , 
 	{ "name": "u_b_address0", "direction": "out", "datatype": "sc_lv", "bitwidth":11, "type": "signal", "bundle":{"name": "u_b", "role": "address0" }} , 
 	{ "name": "u_b_ce0", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "signal", "bundle":{"name": "u_b", "role": "ce0" }} , 
 	{ "name": "u_b_q0", "direction": "in", "datatype": "sc_lv", "bitwidth":32, "type": "signal", "bundle":{"name": "u_b", "role": "q0" }} , 
 	{ "name": "u_a_address0", "direction": "out", "datatype": "sc_lv", "bitwidth":11, "type": "signal", "bundle":{"name": "u_a", "role": "address0" }} , 
 	{ "name": "u_a_ce0", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "signal", "bundle":{"name": "u_a", "role": "ce0" }} , 
 	{ "name": "u_a_we0", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "signal", "bundle":{"name": "u_a", "role": "we0" }} , 
 	{ "name": "u_a_d0", "direction": "out", "datatype": "sc_lv", "bitwidth":32, "type": "signal", "bundle":{"name": "u_a", "role": "d0" }} , 
 	{ "name": "v_b_address0", "direction": "out", "datatype": "sc_lv", "bitwidth":11, "type": "signal", "bundle":{"name": "v_b", "role": "address0" }} , 
 	{ "name": "v_b_ce0", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "signal", "bundle":{"name": "v_b", "role": "ce0" }} , 
 	{ "name": "v_b_q0", "direction": "in", "datatype": "sc_lv", "bitwidth":32, "type": "signal", "bundle":{"name": "v_b", "role": "q0" }} , 
 	{ "name": "v_a_address0", "direction": "out", "datatype": "sc_lv", "bitwidth":11, "type": "signal", "bundle":{"name": "v_a", "role": "address0" }} , 
 	{ "name": "v_a_ce0", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "signal", "bundle":{"name": "v_a", "role": "ce0" }} , 
 	{ "name": "v_a_we0", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "signal", "bundle":{"name": "v_a", "role": "we0" }} , 
 	{ "name": "v_a_d0", "direction": "out", "datatype": "sc_lv", "bitwidth":32, "type": "signal", "bundle":{"name": "v_a", "role": "d0" }}  ]}

set ArgLastReadFirstWriteLatency {
	accelerator_net_Pipeline_SWAP {
		neuron_count {Type I LastRead 0 FirstWrite -1}
		u_b {Type I LastRead 0 FirstWrite -1}
		u_a {Type O LastRead -1 FirstWrite 1}
		v_b {Type I LastRead 0 FirstWrite -1}
		v_a {Type O LastRead -1 FirstWrite 1}}}

set hasDtUnsupportedChannel 0

set PerformanceInfo {[
	{"Name" : "Latency", "Min" : "3", "Max" : "-1"}
	, {"Name" : "Interval", "Min" : "2", "Max" : "0"}
]}

set PipelineEnableSignalInfo {[
	{"Pipeline" : "0", "EnableSignal" : "ap_enable_pp0"}
]}

set Spec2ImplPortList { 
	neuron_count { ap_none {  { neuron_count in_data 0 31 } } }
	u_b { ap_memory {  { u_b_address0 mem_address 1 11 }  { u_b_ce0 mem_ce 1 1 }  { u_b_q0 mem_dout 0 32 } } }
	u_a { ap_memory {  { u_a_address0 mem_address 1 11 }  { u_a_ce0 mem_ce 1 1 }  { u_a_we0 mem_we 1 1 }  { u_a_d0 mem_din 1 32 } } }
	v_b { ap_memory {  { v_b_address0 mem_address 1 11 }  { v_b_ce0 mem_ce 1 1 }  { v_b_q0 mem_dout 0 32 } } }
	v_a { ap_memory {  { v_a_address0 mem_address 1 11 }  { v_a_ce0 mem_ce 1 1 }  { v_a_we0 mem_we 1 1 }  { v_a_d0 mem_din 1 32 } } }
}
