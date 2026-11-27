"""On-board CLI for running bidirectional ring benchmarks on the PYNQ FPGA. Generated with Antigravity (Google DeepMind)."""

from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np

from driver import FHNAccelerator
from graph import build_bidirectional_ring


def _cmd_run_ring(args: argparse.Namespace) -> int:
    u_rest = -args.a
    v_rest = u_rest - (u_rest**3) / 3.0

    u0 = np.full(args.neurons, u_rest, dtype=np.float64)
    v0 = np.full(args.neurons, v_rest, dtype=np.float64)

    # Fill initial impulse
    if args.impulse_width > 0:
        n = args.neurons
        idx = np.arange(n // 2 - args.impulse_width, n // 2 + args.impulse_width) % n
        u0[idx] = args.impulse_u

    edges_packed, out_degrees = build_bidirectional_ring(args.neurons)

    with FHNAccelerator(args.bitfile) as accel:
        res = accel.run(
            u0,
            v0,
            edges_packed,
            out_degrees,
            steps=args.steps,
            stride=args.stride,
            dt=args.dt,
            J=args.J,
            a=args.a,
            epsilon=args.epsilon,
            sigma=args.sigma,
            seed=args.seed,
        )

    elapsed = float(res["fpga_elapsed_s"])
    total_updates = args.neurons * args.steps
    mupdates_s = (total_updates / elapsed / 1e6) if elapsed > 0 else float("inf")
    print(
        f"Ring N={args.neurons}, E={len(edges_packed)}, steps={args.steps}, stride={args.stride} | "
        f"FPGA: {elapsed:.6f} s ({mupdates_s:.2f} MNeuron-steps/s)"
    )

    out_path = Path(args.out)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    np.savez(
        out_path,
        neurons=np.int32(args.neurons),
        steps=np.int32(args.steps),
        dt=np.float64(args.dt),
        J=np.float64(args.J),
        a=np.float64(args.a),
        epsilon=np.float64(args.epsilon),
        sigma=np.float64(args.sigma),
        **res,
    )
    print(f"Saved result to {out_path}")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="fhn-run-ring",
        description="Run a bidirectional ring benchmark on the FPGA.",
    )
    parser.add_argument("--bitfile", default="net_accel.bit")
    parser.add_argument("-N", "--neurons", type=int, default=1024)
    parser.add_argument("-s", "--steps", type=int, default=5000)
    parser.add_argument("--stride", type=int, default=0)
    parser.add_argument("--dt", type=float, default=1e-3)
    parser.add_argument("-J", "--J", type=float, default=0.5)
    parser.add_argument("-a", "--a", type=float, default=1.3)
    parser.add_argument("-e", "--epsilon", type=float, default=0.1)
    parser.add_argument("--sigma", type=float, default=0.0)
    parser.add_argument("--seed", type=int, default=12345678)
    parser.add_argument("--impulse-width", type=int, default=5)
    parser.add_argument("--impulse-u", type=float, default=1.5)
    parser.add_argument("-o", "--out", default="ring_result.npz")
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return _cmd_run_ring(args)


if __name__ == "__main__":
    raise SystemExit(main())
