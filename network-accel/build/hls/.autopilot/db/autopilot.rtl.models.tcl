set SynModuleInfo {
  {SRCNAME accelerator_net_Pipeline_EDGE_LOOP MODELNAME accelerator_net_Pipeline_EDGE_LOOP RTLNAME accelerator_net_accelerator_net_Pipeline_EDGE_LOOP
    SUBMODULES {
      {MODELNAME accelerator_net_flow_control_loop_pipe_sequential_init RTLNAME accelerator_net_flow_control_loop_pipe_sequential_init BINDTYPE interface TYPE internal_upc_flow_control INSTNAME accelerator_net_flow_control_loop_pipe_sequential_init_U}
    }
  }
  {SRCNAME accelerator_net_Pipeline_SWAP MODELNAME accelerator_net_Pipeline_SWAP RTLNAME accelerator_net_accelerator_net_Pipeline_SWAP}
  {SRCNAME accelerator_net_Pipeline_STORE_STATE MODELNAME accelerator_net_Pipeline_STORE_STATE RTLNAME accelerator_net_accelerator_net_Pipeline_STORE_STATE}
  {SRCNAME accelerator_net_Pipeline_LOAD_STATE MODELNAME accelerator_net_Pipeline_LOAD_STATE RTLNAME accelerator_net_accelerator_net_Pipeline_LOAD_STATE}
  {SRCNAME accelerator_net_Pipeline_LOAD_ROW_PTR MODELNAME accelerator_net_Pipeline_LOAD_ROW_PTR RTLNAME accelerator_net_accelerator_net_Pipeline_LOAD_ROW_PTR}
  {SRCNAME accelerator_net_Pipeline_LOAD_COL_IDX MODELNAME accelerator_net_Pipeline_LOAD_COL_IDX RTLNAME accelerator_net_accelerator_net_Pipeline_LOAD_COL_IDX}
  {SRCNAME accelerator_net_Pipeline_COMPUTE_DEGREE MODELNAME accelerator_net_Pipeline_COMPUTE_DEGREE RTLNAME accelerator_net_accelerator_net_Pipeline_COMPUTE_DEGREE}
  {SRCNAME accelerator_net MODELNAME accelerator_net RTLNAME accelerator_net IS_TOP 1
    SUBMODULES {
      {MODELNAME accelerator_net_mul_27s_26ns_52_1_1 RTLNAME accelerator_net_mul_27s_26ns_52_1_1 BINDTYPE op TYPE mul IMPL auto LATENCY 0 ALLOW_PRAGMA 1}
      {MODELNAME accelerator_net_mul_28s_32s_56_2_1 RTLNAME accelerator_net_mul_28s_32s_56_2_1 BINDTYPE op TYPE mul IMPL auto LATENCY 1 ALLOW_PRAGMA 1}
      {MODELNAME accelerator_net_mul_32s_32s_56_2_1 RTLNAME accelerator_net_mul_32s_32s_56_2_1 BINDTYPE op TYPE mul IMPL auto LATENCY 1 ALLOW_PRAGMA 1}
      {MODELNAME accelerator_net_mul_32s_32s_64_2_1 RTLNAME accelerator_net_mul_32s_32s_64_2_1 BINDTYPE op TYPE mul IMPL auto LATENCY 1 ALLOW_PRAGMA 1}
      {MODELNAME accelerator_net_mul_64s_32s_80_5_1 RTLNAME accelerator_net_mul_64s_32s_80_5_1 BINDTYPE op TYPE mul IMPL auto LATENCY 4 ALLOW_PRAGMA 1}
      {MODELNAME accelerator_net_mul_64s_32s_94_5_1 RTLNAME accelerator_net_mul_64s_32s_94_5_1 BINDTYPE op TYPE mul IMPL auto LATENCY 4 ALLOW_PRAGMA 1}
      {MODELNAME accelerator_net_mul_94s_32s_104_5_1 RTLNAME accelerator_net_mul_94s_32s_104_5_1 BINDTYPE op TYPE mul IMPL auto LATENCY 4 ALLOW_PRAGMA 1}
      {MODELNAME accelerator_net_mul_118s_121ns_238_5_1 RTLNAME accelerator_net_mul_118s_121ns_238_5_1 BINDTYPE op TYPE mul IMPL auto LATENCY 4 ALLOW_PRAGMA 1}
      {MODELNAME accelerator_net_sub_238ns_238ns_238_2_1 RTLNAME accelerator_net_sub_238ns_238ns_238_2_1 BINDTYPE op TYPE sub IMPL fabric LATENCY 1}
      {MODELNAME accelerator_net_row_ptr_RAM_AUTO_1R1W RTLNAME accelerator_net_row_ptr_RAM_AUTO_1R1W BINDTYPE storage TYPE ram IMPL auto LATENCY 2 ALLOW_PRAGMA 1}
      {MODELNAME accelerator_net_col_idx_RAM_AUTO_1R1W RTLNAME accelerator_net_col_idx_RAM_AUTO_1R1W BINDTYPE storage TYPE ram IMPL auto LATENCY 2 ALLOW_PRAGMA 1}
      {MODELNAME accelerator_net_degree_RAM_AUTO_1R1W RTLNAME accelerator_net_degree_RAM_AUTO_1R1W BINDTYPE storage TYPE ram IMPL auto LATENCY 2 ALLOW_PRAGMA 1}
      {MODELNAME accelerator_net_u_a_RAM_AUTO_1R1W RTLNAME accelerator_net_u_a_RAM_AUTO_1R1W BINDTYPE storage TYPE ram IMPL auto LATENCY 2 ALLOW_PRAGMA 1}
      {MODELNAME accelerator_net_gmem0_m_axi RTLNAME accelerator_net_gmem0_m_axi BINDTYPE interface TYPE adapter IMPL m_axi}
      {MODELNAME accelerator_net_gmem1_m_axi RTLNAME accelerator_net_gmem1_m_axi BINDTYPE interface TYPE adapter IMPL m_axi}
      {MODELNAME accelerator_net_control_s_axi RTLNAME accelerator_net_control_s_axi BINDTYPE interface TYPE interface_s_axilite}
    }
  }
}
