# This script segment is generated automatically by AutoPilot

set axilite_register_dict [dict create]
set port_control {
row_ptr_ddr { 
	dir I
	width 64
	depth 1
	mode ap_none
	offset 16
	offset_end 27
}
col_idx_ddr { 
	dir I
	width 64
	depth 1
	mode ap_none
	offset 28
	offset_end 39
}
u_ddr { 
	dir I
	width 64
	depth 1
	mode ap_none
	offset 40
	offset_end 51
}
v_ddr { 
	dir I
	width 64
	depth 1
	mode ap_none
	offset 52
	offset_end 63
}
neuron_count { 
	dir I
	width 32
	depth 1
	mode ap_none
	offset 64
	offset_end 71
}
edge_count { 
	dir I
	width 32
	depth 1
	mode ap_none
	offset 72
	offset_end 79
}
timesteps { 
	dir I
	width 32
	depth 1
	mode ap_none
	offset 80
	offset_end 87
}
command { 
	dir I
	width 32
	depth 1
	mode ap_none
	offset 88
	offset_end 95
}
dt { 
	dir I
	width 32
	depth 1
	mode ap_none
	offset 96
	offset_end 103
}
a { 
	dir I
	width 32
	depth 1
	mode ap_none
	offset 104
	offset_end 111
}
J { 
	dir I
	width 32
	depth 1
	mode ap_none
	offset 112
	offset_end 119
}
inv_e { 
	dir I
	width 32
	depth 1
	mode ap_none
	offset 120
	offset_end 127
}
sigma_sqrt_dt { 
	dir I
	width 32
	depth 1
	mode ap_none
	offset 128
	offset_end 135
}
seed { 
	dir I
	width 32
	depth 1
	mode ap_none
	offset 136
	offset_end 143
}
reseed { 
	dir I
	width 1
	depth 1
	mode ap_none
	offset 144
	offset_end 151
}
ap_start { }
ap_done { }
ap_ready { }
ap_idle { }
interrupt {
}
}
dict set axilite_register_dict control $port_control


