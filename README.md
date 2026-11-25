# Stochastic FitzHugh-Nagumo Network Accelerator (PYNQ-Z2)

HLS accelerator (`net_accel`) and host/board tooling for integrating stochastic FitzHugh-Nagumo neuron networks on the PYNQ-Z2 FPGA.

## Repository Layout

- `net-accel/` — Vitis HLS C++ implementation (`net_accel`, Q8.24 fixed-point RK4 + Gaussian CLT noise) and C testbench.
- `vivado/` — Declarative Vivado CLI flow (`init_project.tcl`, `build_bitstream.tcl`) and source-controlled block design (`vivado/bd/design_1/design_1.bd` + `ui/`). Generated project files (`vivado/build/`) and exported artifacts (`vivado/export/net_accel.{bit,hwh}`) are gitignored.
- `python/` — Board Python driver (`driver.py`), Q8.24/edge packing (`packing.py`), directed graph compiler (`graph.py`), on-board runner (`run_ring.py`), and host unit tests (`tests/`).
- `scripts/` — Board setup (`setup_board.sh`), ring plotting CLI (`plot-ring.py`), and HLS plotting helpers.
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

## Running on the PYNQ-Z2

On PYNQ images, the `pynq` package lives inside `/usr/local/share/pynq-venv` and requires `root` + XRT environment variables (`xrt_setup.sh`). `just setup-board` installs a wrapper at `/usr/local/bin/pynq-python` with passwordless `sudo` and copies your SSH public key.

1. One-time board setup (board connected via USB-Ethernet at `192.168.3.1`, or set `$env:PYNQ_HOST`):
   ```powershell
   just setup-board
   ```
2. Deploy `python/*.py` and `net_accel.{bit,hwh}` to `/home/xilinx/fhn-fpga`:
   ```powershell
   just deploy
   ```
3. Run a ring benchmark on the board and pull the `.npz` result back to your PC:
   ```powershell
   just run-ring
   just run-ring -N 4096 --steps 10000 --stride 100 -J 0.5 --sigma 0.05 --impulse-u -0.1
   ```
4. Plot the fetched result locally:
   ```powershell
   just plot-ring
   ```

## Local Tests

```powershell
just test
```
