"""Host-side CLI for plotting a saved .npz ring simulation result. This file is AI generated"""

from __future__ import annotations

import argparse

import matplotlib
import numpy as np


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="plot-ring",
        description="Plot a saved .npz ring simulation result.",
    )
    parser.add_argument("file", nargs="?", default="build/results/ring_result.npz")
    parser.add_argument("--save", help="Save plot image instead of opening a window")
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)

    if args.save:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    with np.load(args.file) as data:
        u_final = data["u_final"]
        v_final = data["v_final"]
        dt = float(data["dt"])
        steps = int(data["steps"])
        has_traj = "u_traj" in data and "times" in data
        u_traj = data["u_traj"] if has_traj else None
        times = data["times"] if has_traj else None

    nrows = 2 if has_traj else 1
    fig, axes = plt.subplots(nrows, 1, figsize=(11, 4 * nrows), constrained_layout=True)
    ax0 = axes[0] if has_traj else axes

    x = np.arange(len(u_final))
    ax0.plot(x, u_final, label="u")
    ax0.plot(x, v_final, label="v", alpha=0.75)
    ax0.set_xlabel("Neuron index")
    ax0.set_ylabel("State")
    ax0.set_title(f"FHN ring state at t = {steps * dt:.3f} (N={len(u_final)})")
    ax0.grid(True)
    ax0.legend()

    if has_traj and u_traj is not None and times is not None:
        ax1 = axes[1]
        im = ax1.imshow(
            u_traj,
            aspect="auto",
            origin="lower",
            extent=[0, len(u_final) - 1, float(times[0]), float(times[-1])],
            cmap="viridis",
        )
        ax1.set_xlabel("Neuron index")
        ax1.set_ylabel("Time")
        ax1.set_title("Activity heatmap u(neuron, t)")
        fig.colorbar(im, ax=ax1, label="u")

    if args.save:
        fig.savefig(args.save, dpi=150)
        print(f"Saved plot to {args.save}")
    else:
        plt.show()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
