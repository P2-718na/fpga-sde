# Build FPGA bitstream + HWH from the packaged HLS IP and export to vivado/export/net_accel.{bit,hwh}.
# Optional arg: timing (default) | fast | default

set vivado_dir [file dirname [file normalize [info script]]]
set repo_root  [file dirname $vivado_dir]
set ip_repo    [file normalize "$repo_root/net-accel/build/hls/impl/ip"]
set proj_xpr   [file normalize "$vivado_dir/build/fhn_pynq.xpr"]
set export_dir [file normalize "$vivado_dir/export"]

set mode [expr {$argc > 0 ? [lindex $argv 0] : "timing"}]
switch -- $mode {
    "timing" {
        set synth_strat "Flow_PerfOptimized_high"
        set impl_strat  "Performance_ExtraTimingOpt"
    }
    "fast" {
        set synth_strat "Flow_RuntimeOptimized"
        set impl_strat  "Flow_RuntimeOptimized"
    }
    "default" {
        set synth_strat "Vivado Synthesis Defaults"
        set impl_strat  "Vivado Implementation Defaults"
    }
    default {
        puts "ERROR: Unknown strategy preset '$mode'. Valid options: timing, fast, default"
        exit 1
    }
}

if { ![file exists "$ip_repo/component.xml"] } {
    puts "ERROR: Packaged HLS IP not found at $ip_repo"
    puts "Run 'just synth' and 'just package' first."
    exit 1
}

if { [file exists $proj_xpr] } {
    open_project $proj_xpr
} else {
    source "$vivado_dir/init_project.tcl"
}

# Refresh IP catalog and upgrade only net_accel so PS7/SmartConnect synthesis caches stay intact.
set_property ip_repo_paths [list $ip_repo] [current_project]
update_ip_catalog -rebuild
open_bd_design [get_files design_1.bd]

set net_ips [get_ips -quiet *net_accel*]
if { $net_ips ne "" } {
    upgrade_ip $net_ips
}
validate_bd_design
save_bd_design
generate_target all [get_files design_1.bd]
make_wrapper -files [get_files design_1.bd] -top -force

set_property strategy $synth_strat [get_runs synth_1]
set_property strategy $impl_strat [get_runs impl_1]
puts "Running Vivado build with mode='$mode' (synth='$synth_strat', impl='$impl_strat')"

catch { reset_run synth_1 }
launch_runs synth_1 -jobs 6
wait_on_run synth_1
if { [get_property PROGRESS [get_runs synth_1]] ne "100%" } {
    puts "ERROR: Synthesis failed (status: [get_property STATUS [get_runs synth_1]])"
    exit 1
}

launch_runs impl_1 -to_step write_bitstream -jobs 6
wait_on_run impl_1
if { [string first "write_bitstream Complete!" [get_property STATUS [get_runs impl_1]]] == -1 } {
    puts "ERROR: Implementation / bitstream failed (status: [get_property STATUS [get_runs impl_1]])"
    exit 1
}

puts "Timing summary: WNS = [get_property STATS.WNS [get_runs impl_1]] ns, TNS = [get_property STATS.TNS [get_runs impl_1]] ns"

set bit_src [file normalize "$vivado_dir/build/fhn_pynq.runs/impl_1/design_1_wrapper.bit"]
if { ![file exists $bit_src] } {
    puts "ERROR: Bitstream not found at $bit_src"
    exit 1
}

# When design_1.bd is referenced in-place outside .srcs, Vivado may write hw_handoff next to the .bd or under build/*.gen.
set hwh_candidates [concat \
    [glob -nocomplain "$vivado_dir/bd/design_1/hw_handoff/design_1.hwh"] \
    [glob -nocomplain "$vivado_dir/build/fhn_pynq.gen/sources_1/bd/design_1/hw_handoff/design_1.hwh"]]
if { [llength $hwh_candidates] == 0 } {
    puts "ERROR: Hardware handoff (.hwh) not found under vivado/bd/design_1 or vivado/build"
    exit 1
}
set hwh_src [lindex $hwh_candidates 0]

file mkdir $export_dir
file copy -force $bit_src "$export_dir/net_accel.bit"
file copy -force $hwh_src "$export_dir/net_accel.hwh"
puts "Exported: $export_dir/net_accel.bit and $export_dir/net_accel.hwh"
