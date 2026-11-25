set windows-shell := ["powershell.exe", "-NoLogo", "-Command"]
set script-interpreter := ["powershell.exe", "-NoLogo", "-ExecutionPolicy", "Bypass", "-File"]

board_host := env("PYNQ_HOST", "192.168.2.99")
board_user := env("PYNQ_USER", "xilinx")
board_dir  := "/home/xilinx/fhn-fpga"

default:
    @just --list

# Run HLS C-Simulation for net-accel
[group("vitis")]
[working-directory("net-accel")]
csim:
    vitis-run --mode hls --csim --config hls_config.cfg --work_dir build

# Run HLS C-Synthesis for net-accel
[group("vitis")]
[working-directory("net-accel")]
csynth:
    v++ -c --mode hls --config hls_config.cfg --work_dir build

# Run HLS C/RTL Co-simulation for net-accel
[group("vitis")]
[working-directory("net-accel")]
cosim:
    vitis-run --mode hls --cosim --config hls_config.cfg --work_dir build

# Package HLS IP for net-accel
[group("vitis")]
[working-directory("net-accel")]
package:
    vitis-run --mode hls --package --config hls_config.cfg --work_dir build

# Plot HLS C-sim trajectory with gnuplot
[group("vitis")]
plot:
    gnuplot -p scripts/plot-trajectory.gp

alias sim := csim
alias synth := csynth

# Open Vivado GUI (creates vivado/build/fhn_pynq.xpr from Tcl if missing)
[group("vivado")]
[working-directory("vivado/build")]
[script]
[extension("ps1")]
vivado:
    if (-not (Test-Path "fhn_pynq.xpr")) {
        vivado -mode batch -nojournal -nolog -source ../init_project.tcl
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
    vivado fhn_pynq.xpr

# Build bitstream + HWH into vivado/export/net_accel.{bit,hwh} (strategy: timing | fast | default)
[group("vivado")]
[working-directory("vivado/build")]
bitstream strategy="fast":
    vivado -mode batch -nojournal -nolog -source ../build_bitstream.tcl -tclargs {{strategy}}
