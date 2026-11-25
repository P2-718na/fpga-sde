# Stochastic FitzHugh-Nagumo Network Accelerator (PYNQ-Z2)

HLS accelerator (`net_accel`) and host/board tooling for integrating stochastic FitzHugh-Nagumo neuron networks on the PYNQ-Z2 FPGA.

## Repository Layout

- `net-accel/` — Vitis HLS C++ implementation (`net_accel`, Q8.24 fixed-point RK4 + Gaussian CLT noise) and C testbench.
- `vivado/` — Declarative Vivado CLI flow (`init_project.tcl`, `build_bitstream.tcl`) and source-controlled block design (`vivado/bd/design_1/design_1.bd` + `ui/`). Generated project files (`vivado/build/`) and exported artifacts (`vivado/export/net_accel.{bit,hwh}`) are gitignored.
- `python/` — Python driver (`accel.py`), Q8.24 packing (`packing.py`), directed graph compiler (`graph.py`), CLI (`cli.py`), and host unit tests (`tests/`).
- `scripts/` — Board setup (`setup_board.sh`) and HLS plotting helpers.
- `board/` — PYNQ-Z2 constraints and board files.

## Hardware Build Workflow (HLS -> Vivado)

1. Simulate and synthesize the HLS IP:
   ```powershell
   just csim
   just synth
   just package
   ```
2. Build the FPGA bitstream and `.hwh` into `vivado/export/net_accel.{bit,hwh}`:
   ```powershell
   just bitstream          # timing-optimized (Performance_ExtraTimingOpt)
   just bitstream fast     # faster iteration (Flow_RuntimeOptimized)
   just bitstream default  # Vivado defaults
   ```
3. To inspect or edit the block design in the Vivado GUI:
   ```powershell
   just vivado
   ```
   `vivado/build/fhn_pynq.xpr` references `vivado/bd/design_1/design_1.bd` in-place (never copied), so saving the block design in the Vivado GUI updates the git-tracked `.bd` directly while keeping generated IPs/runs gitignored under `vivado/build/` and `vivado/bd/design_1/`.
