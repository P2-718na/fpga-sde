set windows-shell := ["powershell.exe", "-NoLogo", "-Command"]

default:
    @just --list

# Run HLS C-Synthesis for net-accel
synth:
    cd net-accel; v++ -c --mode hls --config hls_config.cfg --work_dir build

# Run HLS C-Simulation for net-accel
csim:
    cd net-accel; vitis-run --mode hls --csim --config hls_config.cfg --work_dir build

# Run HLS C/RTL Co-simulation for net-accel
cosim:
    cd net-accel; vitis-run --mode hls --cosim --config hls_config.cfg --work_dir build

alias compile := synth
alias sim := csim
