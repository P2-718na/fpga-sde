set windows-shell := ["powershell.exe", "-NoLogo", "-Command"]

default:
    @just --list

# Run HLS C-Simulation for net-accel
csim:
    cd net-accel; vitis-run --mode hls --csim --config hls_config.cfg --work_dir build

# Run HLS C-Synthesis for net-accel
synth:
    cd net-accel; vitis-run --mode hls --config hls_config.cfg --work_dir build

# Run HLS C/RTL Co-simulation for net-accel
cosim:
    cd net-accel; vitis-run --mode hls --cosim --config hls_config.cfg --work_dir build

# Package HLS IP for net-accel
package:
    cd net-accel; vitis-run --mode hls --package --config hls_config.cfg --work_dir build

alias sim := csim
