"""Directed graph compiler for the FPGA edge-list and out-degree buffers. This file is AI generated."""

from __future__ import annotations

import numpy as np

from packing import pack_edges


def build_bidirectional_ring(neuron_count: int) -> tuple[np.ndarray, np.ndarray]:
    """Build hardware-packed buffers for a bidirectional ring of N neurons."""
    if neuron_count <= 0:
        raise ValueError("neuron_count must be positive")

    n = np.arange(neuron_count, dtype=np.intp)
    src = np.empty(2 * neuron_count, dtype=np.intp)
    dst = np.empty(2 * neuron_count, dtype=np.intp)
    src[0::2] = (n - 1) % neuron_count
    src[1::2] = (n + 1) % neuron_count
    dst[0::2] = n
    dst[1::2] = n
    return pack_edges(neuron_count, np.column_stack((src, dst)))
