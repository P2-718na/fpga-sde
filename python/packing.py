"""Fixed-point Q8.24 (ap_fixed<32,8>) conversions and 64-bit state/edge packing. Generated with Antigravity (Google DeepMind)."""

from __future__ import annotations

import numpy as np

Q_FRAC_BITS = 24
Q_SCALE = 1 << Q_FRAC_BITS
Q_MIN_INT = -(1 << 31)
Q_MAX_INT = (1 << 31) - 1
Q_MIN = Q_MIN_INT / Q_SCALE
Q_MAX = Q_MAX_INT / Q_SCALE


def float_to_q824(x: float | np.ndarray) -> np.int32 | np.ndarray:
    """Convert scalar or array from float to signed 32-bit Q8.24 integers."""
    arr = np.asarray(x, dtype=np.float64)
    if not np.all(np.isfinite(arr)):
        raise ValueError("Q8.24 value must be finite (no NaN or Inf)")

    scaled = np.round(arr * Q_SCALE)
    if np.any(scaled < Q_MIN_INT) or np.any(scaled > Q_MAX_INT):
        raise ValueError(f"Value out of Q8.24 range [{Q_MIN}, {Q_MAX}]")

    result = scaled.astype(np.int32)
    return np.int32(result) if np.ndim(x) == 0 else result


def q824_to_float(x: int | np.ndarray) -> float | np.ndarray:
    """Convert signed 32-bit Q8.24 integers back to float64."""
    arr = np.asarray(x, dtype=np.int32)
    out = arr.astype(np.float64) / Q_SCALE
    return float(out) if np.ndim(x) == 0 else out


def q824_to_u32_reg(x: float) -> int:
    """Encode a scalar float as unsigned 32-bit Q8.24 bits for AXI-Lite writes.

    pynq's s_axilite RegisterMap rejects negative Python ints on unsigned
    32-bit fields, so signed two's-complement bits must be masked to uint32.
    """
    return int(float_to_q824(float(x))) & 0xFFFFFFFF


def pack_states(u: np.ndarray, v: np.ndarray) -> np.ndarray:
    """Pack (u, v) float arrays into 64-bit words: bits[31:0]=u, bits[63:32]=v."""
    u_arr = np.asarray(u, dtype=np.float64)
    v_arr = np.asarray(v, dtype=np.float64)
    if u_arr.ndim != 1 or u_arr.shape != v_arr.shape:
        raise ValueError("u and v must be 1-D arrays of identical length")

    u_bits = float_to_q824(u_arr).view(np.uint32).astype(np.uint64)
    v_bits = float_to_q824(v_arr).view(np.uint32).astype(np.uint64)
    return u_bits | (v_bits << np.uint64(32))


def unpack_states(words: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    """Unpack 64-bit state words into (u, v) float64 arrays."""
    raw = np.asarray(words, dtype=np.uint64)
    u_int = (raw & np.uint64(0xFFFFFFFF)).astype(np.uint32).view(np.int32)
    v_int = (raw >> np.uint64(32)).astype(np.uint32).view(np.int32)
    return q824_to_float(u_int), q824_to_float(v_int)


def validate_packed_edges(
    neuron_count: int,
    edge_words: np.ndarray,
    dst_sorted: np.ndarray | None = None,
) -> None:
    """Validate that packed edges cover all neurons 0..N-1 and each ends with last=1."""
    if neuron_count <= 0:
        raise ValueError("neuron_count must be positive")

    edges = np.asarray(edge_words, dtype=np.uint64)
    if edges.ndim != 1 or len(edges) < neuron_count:
        raise ValueError("edge_words must be 1-D with at least neuron_count edges")

    src = edges & np.uint64(0xFFFFFFFF)
    last = (edges >> np.uint64(32)) & np.uint64(1)

    if np.any(src >= np.uint64(neuron_count)):
        raise ValueError(f"Packed edge source indices must be in [0, {neuron_count})")
    if last[-1] != np.uint64(1):
        raise ValueError("Packed edge list must end with an edge flagged last=1")

    last_indices = np.flatnonzero(last == 1)
    if len(last_indices) != neuron_count:
        raise ValueError(
            f"Expected {neuron_count} neuron segments ending with last=1, got {len(last_indices)}"
        )

    if dst_sorted is not None:
        dst = np.asarray(dst_sorted, dtype=np.intp)
        if dst.ndim != 1 or len(dst) != len(edges):
            raise ValueError("dst_sorted must be 1-D and match edge_words length")
        if len(dst) > 1 and not np.all(dst[:-1] <= dst[1:]):
            raise ValueError("dst_sorted must be sorted in non-decreasing order")

        expected_neurons = np.arange(neuron_count, dtype=np.intp)
        run_starts = np.concatenate((np.array([0], dtype=np.intp), last_indices[:-1] + 1))
        if not (
            np.array_equal(dst[run_starts], expected_neurons)
            and np.array_equal(dst[last_indices], expected_neurons)
        ):
            raise ValueError("Each neuron 0..N-1 must have at least one edge and end with last=1")


def pack_edges(neuron_count: int, edges: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    """Compile directed edges (src, dst) into hardware-ordered buffers.

    Returns (edge_list_u64, out_degrees_u32) where:
      - edge_list packs bits[31:0]=src, bit[32]=last, sorted by destination node.
      - out_degrees holds the conservative out-degree d_out(n) for each neuron.
    """
    if neuron_count <= 0:
        raise ValueError("neuron_count must be positive")

    edges_arr = np.asarray(edges, dtype=np.intp)
    if edges_arr.size == 0:
        edges_arr = edges_arr.reshape(0, 2)
    elif edges_arr.ndim != 2 or edges_arr.shape[1] != 2:
        raise ValueError("edges must have shape (E, 2) of (src, dst) pairs")

    if len(edges_arr) > 0:
        if np.any(edges_arr < 0) or np.any(edges_arr >= neuron_count):
            raise ValueError(f"Edge node indices must be in [0, {neuron_count})")
        src = edges_arr[:, 0]
        dst = edges_arr[:, 1]
    else:
        src = np.empty(0, dtype=np.intp)
        dst = np.empty(0, dtype=np.intp)

    out_degrees = np.bincount(src, minlength=neuron_count).astype(np.uint32)
    in_degrees = np.bincount(dst, minlength=neuron_count)

    # Hardware compute_flows requires every neuron 0..N-1 to emit one last=1 edge
    # so I_stream and state_stream stay in lockstep. A self-loop (n -> n) with
    # out_degrees[n] += 1 cancels identically in J * (sum_m u_m - d_out * u_n).
    missing = np.flatnonzero(in_degrees == 0)
    if len(missing) > 0:
        src = np.concatenate([src, missing])
        dst = np.concatenate([dst, missing])
        out_degrees[missing] += np.uint32(1)

    order = np.argsort(dst, kind="mergesort")
    src_sorted = src[order].astype(np.uint64)
    dst_sorted = dst[order]

    last = np.empty(len(dst_sorted), dtype=np.uint64)
    last[:-1] = (dst_sorted[:-1] != dst_sorted[1:]).astype(np.uint64)
    last[-1] = np.uint64(1)

    edge_words = (src_sorted & np.uint64(0xFFFFFFFF)) | (last << np.uint64(32))
    validate_packed_edges(neuron_count, edge_words, dst_sorted)
    return edge_words, out_degrees
