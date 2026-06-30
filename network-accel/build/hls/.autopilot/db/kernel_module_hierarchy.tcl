set ModuleHierarchy {[{
"Name" : "accelerator_net", "RefName" : "accelerator_net","ID" : "0","Type" : "sequential",
"SubInsts" : [
	{"Name" : "grp_accelerator_net_Pipeline_STORE_STATE_fu_569", "RefName" : "accelerator_net_Pipeline_STORE_STATE","ID" : "1","Type" : "sequential",
		"SubLoops" : [
		{"Name" : "STORE_STATE","RefName" : "STORE_STATE","ID" : "2","Type" : "pipeline"},]},
	{"Name" : "grp_accelerator_net_Pipeline_LOAD_STATE_fu_582", "RefName" : "accelerator_net_Pipeline_LOAD_STATE","ID" : "3","Type" : "sequential",
		"SubLoops" : [
		{"Name" : "LOAD_STATE","RefName" : "LOAD_STATE","ID" : "4","Type" : "pipeline"},]},
	{"Name" : "grp_accelerator_net_Pipeline_LOAD_ROW_PTR_fu_595", "RefName" : "accelerator_net_Pipeline_LOAD_ROW_PTR","ID" : "5","Type" : "sequential",
		"SubLoops" : [
		{"Name" : "LOAD_ROW_PTR","RefName" : "LOAD_ROW_PTR","ID" : "6","Type" : "pipeline"},]},
	{"Name" : "grp_accelerator_net_Pipeline_LOAD_COL_IDX_fu_605", "RefName" : "accelerator_net_Pipeline_LOAD_COL_IDX","ID" : "7","Type" : "sequential",
		"SubLoops" : [
		{"Name" : "LOAD_COL_IDX","RefName" : "LOAD_COL_IDX","ID" : "8","Type" : "pipeline"},]},
	{"Name" : "grp_accelerator_net_Pipeline_COMPUTE_DEGREE_fu_615", "RefName" : "accelerator_net_Pipeline_COMPUTE_DEGREE","ID" : "9","Type" : "sequential",
		"SubLoops" : [
		{"Name" : "COMPUTE_DEGREE","RefName" : "COMPUTE_DEGREE","ID" : "10","Type" : "pipeline"},]},],
"SubLoops" : [
	{"Name" : "TIMESTEP_LOOP","RefName" : "TIMESTEP_LOOP","ID" : "11","Type" : "no",
	"SubInsts" : [
	{"Name" : "grp_accelerator_net_Pipeline_SWAP_fu_543", "RefName" : "accelerator_net_Pipeline_SWAP","ID" : "12","Type" : "sequential",
			"SubLoops" : [
			{"Name" : "SWAP","RefName" : "SWAP","ID" : "13","Type" : "pipeline"},]},],
	"SubLoops" : [
	{"Name" : "NEURON_LOOP","RefName" : "NEURON_LOOP","ID" : "14","Type" : "no",
		"SubInsts" : [
		{"Name" : "grp_accelerator_net_Pipeline_EDGE_LOOP_fu_556", "RefName" : "accelerator_net_Pipeline_EDGE_LOOP","ID" : "15","Type" : "sequential",
				"SubLoops" : [
				{"Name" : "EDGE_LOOP","RefName" : "EDGE_LOOP","ID" : "16","Type" : "pipeline"},]},]},]},]
}]}