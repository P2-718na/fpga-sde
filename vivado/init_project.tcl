# Initialize a clean Vivado project referencing the git-tracked block design in-place.

set vivado_dir [file dirname [file normalize [info script]]]
set repo_root  [file dirname $vivado_dir]
set ip_repo    [file normalize "$repo_root/net-accel/build/hls/impl/ip"]
set xdc_file   [file normalize "$repo_root/board/PYNQ-Z2 v1.0.xdc"]
set bd_file    [file normalize "$vivado_dir/bd/design_1/design_1.bd"]
set proj_dir   [file normalize "$vivado_dir/build"]

if { ![file exists "$ip_repo/component.xml"] } {
    puts "ERROR: Packaged HLS IP not found at $ip_repo"
    puts "Run 'just synth' and 'just package' first."
    exit 1
}

if { ![file exists $bd_file] } {
    puts "ERROR: Source block design not found at $bd_file"
    exit 1
}

file mkdir $proj_dir
create_project fhn_pynq $proj_dir -part xc7z020clg400-1 -force
set_property BOARD_PART tul.com.tw:pynq-z2:part0:1.0 [current_project]
set_property ip_repo_paths [list $ip_repo] [current_project]
update_ip_catalog -rebuild

# Reference design_1.bd in-place (never import_files) so Vivado GUI edits update the tracked BD directly.
read_bd $bd_file
open_bd_design [get_files design_1.bd]

set bd_ips [get_ips -quiet]
if { $bd_ips ne "" } {
    upgrade_ip -quiet $bd_ips
}
validate_bd_design
save_bd_design

set wrapper_file [make_wrapper -files [get_files design_1.bd] -top -force]
add_files -norecurse [list $wrapper_file]
set_property top design_1_wrapper [current_fileset]
update_compile_order -fileset sources_1

add_files -fileset constrs_1 -norecurse [list $xdc_file]

set_property strategy "Flow_PerfOptimized_high" [get_runs synth_1]
set_property strategy "Performance_ExtraTimingOpt" [get_runs impl_1]

puts "Initialized Vivado project: $proj_dir/fhn_pynq.xpr"
