"""On-board PYNQ driver for the net_accel FitzHugh-Nagumo HLS IP. This file is AI generated."""

from __future__ import annotations

import math
import time
from pathlib import Path

import numpy as np

from packing import (
    float_to_q824,
    pack_states,
    q824_to_float,
    q824_to_u32_reg,
    unpack_states,
    validate_packed_edges,
)

# Must match MAX_NEURONS in net-accel/accelerator.hpp (BRAM capacity).
MAX_NEURONS = 12288

# design_1.bd maps smartconnect_0 to HP0/HP1/HP2 across [0x0000_0000, 0x1800_0000) (384 MB).
MAX_SMARTCONNECT_PHYS_ADDR = 0x1800_0000


class FHNAccelerator:
    """Context-managed PYNQ wrapper for net_accel_0."""

    def __init__(
        self,
        bitfile: str | Path,
        ip_name: str = "net_accel_0",
        download: bool = True,
        *,
        _overlay_cls=None,
        _allocate_fn=None,
    ) -> None:
        if _overlay_cls is None or _allocate_fn is None:
            try:
                from pynq import Overlay, allocate
            except ImportError as exc:
                raise RuntimeError(
                    "Could not import 'pynq'. On the PYNQ board, run via "
                    "'sudo /usr/local/bin/pynq-python ...' so pynq-venv and XRT are active."
                ) from exc
            _overlay_cls = _overlay_cls or Overlay
            _allocate_fn = _allocate_fn or allocate

        self._allocate = _allocate_fn
        self.overlay = _overlay_cls(str(bitfile), download=download)
        self.accel = getattr(self.overlay, ip_name)

    def __enter__(self) -> FHNAccelerator:
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def close(self) -> None:
        if getattr(self, "overlay", None) is not None and hasattr(self.overlay, "free"):
            self.overlay.free()
        self.overlay = None

    def _set_addr64(self, name: str, phys_addr: int) -> None:
        # Write top-level *_1 (bits 31:0) and *_2 (bits 63:32) registers directly.
        rm = self.accel.register_map
        setattr(rm, f"{name}_1", int(phys_addr & 0xFFFFFFFF))
        setattr(rm, f"{name}_2", int((phys_addr >> 32) & 0xFFFFFFFF))

    @staticmethod
    def _check_cma_addr(name: str, buf) -> None:
        end_addr = int(buf.physical_address) + int(buf.nbytes)
        if end_addr > MAX_SMARTCONNECT_PHYS_ADDR:
            raise RuntimeError(
                f"CMA buffer '{name}' at 0x{buf.physical_address:08x}..0x{end_addr:08x} "
                f"exceeds SmartConnect 384 MB DDR window (< 0x{MAX_SMARTCONNECT_PHYS_ADDR:08x})."
            )

    @staticmethod
    def _free_buffer(buf) -> None:
        if buf is None:
            return
        if hasattr(buf, "freebuffer"):
            buf.freebuffer()
        elif hasattr(buf, "close"):
            buf.close()

    def run(
        self,
        u0: np.ndarray,
        v0: np.ndarray,
        edges_packed: np.ndarray,
        out_degrees: np.ndarray,
        *,
        steps: int,
        stride: int = 0,
        dt: float = 1e-3,
        J: float = 0.5,
        a: float = 1.3,
        epsilon: float = 0.1,
        sigma: float = 0.0,
        seed: int = 12345678,
        validate_edges: bool = True,
    ) -> dict[str, np.ndarray | float]:
        """Run integration on the FPGA; set stride > 0 to record trajectories."""
        u0_arr = np.asarray(u0, dtype=np.float64)
        v0_arr = np.asarray(v0, dtype=np.float64)
        edges_arr = np.asarray(edges_packed, dtype=np.uint64)
        deg_arr = np.asarray(out_degrees, dtype=np.uint32)

        n_neurons = u0_arr.shape[0] if u0_arr.ndim == 1 else 0
        if u0_arr.ndim != 1 or not (1 <= n_neurons <= MAX_NEURONS):
            raise ValueError(f"u0 must be 1-D with 1 <= N <= {MAX_NEURONS}")
        if v0_arr.shape != (n_neurons,):
            raise ValueError(f"v0 must have shape ({n_neurons},)")
        if deg_arr.shape != (n_neurons,):
            raise ValueError(f"out_degrees must have shape ({n_neurons},)")
        if validate_edges:
            validate_packed_edges(n_neurons, edges_arr)
        if steps < 1:
            raise ValueError("steps must be >= 1")
        if stride < 0:
            raise ValueError("stride must be >= 0")
        if dt <= 0 or epsilon <= 0 or sigma < 0:
            raise ValueError("dt > 0, epsilon > 0, and sigma >= 0 are required")

        n_edges = len(edges_arr)
        inv_e = 1.0 / epsilon
        sigma_sqrt_dt = sigma * math.sqrt(dt)

        record_traj = 0 < stride < steps
        if record_traj:
            chunks = [stride] * (steps // stride)
            if steps % stride:
                chunks.append(steps % stride)
        else:
            chunks = [steps]

        state_a = state_b = edge_buf = deg_buf = None
        try:
            state_a = self._allocate(shape=(n_neurons,), dtype=np.uint64)
            state_b = self._allocate(shape=(n_neurons,), dtype=np.uint64)
            edge_buf = self._allocate(shape=(n_edges,), dtype=np.uint64)
            deg_buf = self._allocate(shape=(n_neurons,), dtype=np.uint32)

            for name, buf in (
                ("state_a", state_a),
                ("state_b", state_b),
                ("edge_list", edge_buf),
                ("out_degrees", deg_buf),
            ):
                self._check_cma_addr(name, buf)

            state_a[:] = pack_states(u0_arr, v0_arr)
            edge_buf[:] = edges_arr
            deg_buf[:] = deg_arr

            state_a.flush()
            edge_buf.flush()
            deg_buf.flush()

            rm = self.accel.register_map
            self._set_addr64("edge_list", edge_buf.physical_address)
            self._set_addr64("out_degrees_in", deg_buf.physical_address)
            rm.neuron_count = int(n_neurons)
            rm.edge_count = int(n_edges)
            rm.dt = q824_to_u32_reg(dt)
            rm.J = q824_to_u32_reg(J)
            rm.a = q824_to_u32_reg(a)
            rm.inv_e = q824_to_u32_reg(inv_e)
            rm.sigma_sqrt_dt = q824_to_u32_reg(sigma_sqrt_dt)
            rm.seed = int(seed & 0xFFFFFFFF)

            if record_traj:
                n_frames = 1 + len(chunks)
                times = np.zeros(n_frames, dtype=np.float64)
                u_traj = np.empty((n_frames, n_neurons), dtype=np.float64)
                v_traj = np.empty((n_frames, n_neurons), dtype=np.float64)
                u_traj[0] = q824_to_float(float_to_q824(u0_arr))
                v_traj[0] = q824_to_float(float_to_q824(v0_arr))

            buf_in, buf_out = state_a, state_b
            elapsed_steps = 0
            fpga_elapsed_s = 0.0

            for idx, chunk_steps in enumerate(chunks):
                # Reseed the hardware PRNG only on the first chunk so noise stays continuous.
                rm.reseed = 1 if idx == 0 else 0
                rm.iteration_count = int(chunk_steps)
                self._set_addr64("state_in", buf_in.physical_address)
                self._set_addr64("state_out", buf_out.physical_address)

                t0 = time.perf_counter()
                rm.CTRL.AP_START = 1
                while not rm.CTRL.AP_DONE:
                    pass
                fpga_elapsed_s += time.perf_counter() - t0

                buf_out.invalidate()
                elapsed_steps += chunk_steps

                if record_traj:
                    u_step, v_step = unpack_states(buf_out)
                    times[idx + 1] = elapsed_steps * dt
                    u_traj[idx + 1] = u_step
                    v_traj[idx + 1] = v_step

                # Swap input/output buffer pointers instead of copying states back on ARM CPU.
                buf_in, buf_out = buf_out, buf_in

            if record_traj:
                result: dict[str, np.ndarray | float] = {
                    "u_final": u_traj[-1],
                    "v_final": v_traj[-1],
                    "fpga_elapsed_s": fpga_elapsed_s,
                    "times": times,
                    "u_traj": u_traj,
                    "v_traj": v_traj,
                }
            else:
                u_final, v_final = unpack_states(buf_in)
                result = {
                    "u_final": u_final,
                    "v_final": v_final,
                    "fpga_elapsed_s": fpga_elapsed_s,
                }
            return result
        finally:
            self._free_buffer(deg_buf)
            self._free_buffer(edge_buf)
            self._free_buffer(state_b)
            self._free_buffer(state_a)
