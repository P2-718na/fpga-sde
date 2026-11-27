"""Host-side unit tests for Q8.24 packing, graph compilation, and driver registers. Generated with Antigravity (Google DeepMind)."""

from __future__ import annotations

import unittest
from types import SimpleNamespace

import numpy as np

from driver import MAX_NEURONS, FHNAccelerator
from graph import build_bidirectional_ring
from packing import (
    float_to_q824,
    pack_edges,
    pack_states,
    q824_to_float,
    q824_to_u32_reg,
    unpack_states,
    validate_packed_edges,
)


class _FakeCtrl:
    def __init__(self, on_start):
        self._on_start = on_start
        self._ap_start = 0
        self.AP_DONE = 0

    @property
    def AP_START(self) -> int:
        return self._ap_start

    @AP_START.setter
    def AP_START(self, value: int) -> None:
        self._ap_start = value
        if value == 1:
            self._on_start()
            self.AP_DONE = 1


class _StrictRegisterMap:
    """Strictly accepts only the exact net_accel register names from design_1.hwh."""

    _FIELDS = {
        "CTRL",
        "state_in_1",
        "state_in_2",
        "edge_list_1",
        "edge_list_2",
        "out_degrees_in_1",
        "out_degrees_in_2",
        "state_out_1",
        "state_out_2",
        "neuron_count",
        "edge_count",
        "iteration_count",
        "dt",
        "J",
        "a",
        "dt_over_e",
        "sigma_sqrt_dt",
        "seed",
        "reseed",
    }

    def __init__(self, on_start):
        object.__setattr__(self, "CTRL", _FakeCtrl(on_start))
        for name in self._FIELDS - {"CTRL"}:
            object.__setattr__(self, name, 0)

    def __setattr__(self, name: str, value) -> None:
        if name not in self._FIELDS:
            raise AttributeError(f"Unexpected register '{name}'")
        if not isinstance(value, int) or value < 0 or value > 0xFFFFFFFF:
            raise ValueError(f"Register '{name}' expects uint32, got {value!r}")
        object.__setattr__(self, name, value)


class _FakeBuffer(np.ndarray):
    def __new__(cls, shape, dtype, phys_addr: int):
        obj = np.zeros(shape, dtype=dtype).view(cls)
        obj.physical_address = phys_addr
        obj.flushed = False
        obj.invalidated = False
        obj.freed = False
        return obj

    def flush(self) -> None:
        self.flushed = True

    def invalidate(self) -> None:
        self.invalidated = True

    def freebuffer(self) -> None:
        self.freed = True


class PackingTests(unittest.TestCase):
    def test_q824_round_trip_and_bounds(self) -> None:
        vals = np.array([-128.0, -1.3, 0.0, 0.5, 127.9999999], dtype=np.float64)
        q = float_to_q824(vals)
        self.assertEqual(q.dtype, np.int32)
        np.testing.assert_allclose(q824_to_float(q), vals, atol=1e-7)

        self.assertEqual(q824_to_u32_reg(-1.0), 0xFF000000)

        with self.assertRaises(ValueError):
            float_to_q824(128.0)
        with self.assertRaises(ValueError):
            float_to_q824(-128.0001)
        with self.assertRaises(ValueError):
            float_to_q824(np.nan)

    def test_state_pack_unpack_round_trip(self) -> None:
        u = np.array([-1.3, 0.0, 1.5, -0.1])
        v = np.array([-0.567, 0.125, -2.25, 3.75])
        packed = pack_states(u, v)
        self.assertEqual(packed.dtype, np.uint64)
        u_out, v_out = unpack_states(packed)
        np.testing.assert_allclose(u_out, u, atol=1e-7)
        np.testing.assert_allclose(v_out, v, atol=1e-7)

    def test_validate_packed_edges_guards(self) -> None:
        edges, _ = pack_edges(3, np.array([[0, 1], [1, 1], [2, 2]]))
        validate_packed_edges(3, edges, np.array([0, 1, 1, 2]))

        # Clearing last bit on the final edge must fail
        no_final_last = edges.copy()
        no_final_last[-1] &= np.uint64(0xFFFFFFFF)
        with self.assertRaises(ValueError):
            validate_packed_edges(3, no_final_last)

        # Missing a last=1 terminator (fewer than N segments) must fail
        fewer_segments = edges.copy()
        fewer_segments[0] &= np.uint64(0xFFFFFFFF)
        with self.assertRaises(ValueError):
            validate_packed_edges(3, fewer_segments)

        # Extra last=1 terminator (more than N segments) must fail
        extra_segment = edges.copy()
        extra_segment[1] |= np.uint64(1) << np.uint64(32)
        with self.assertRaises(ValueError):
            validate_packed_edges(3, extra_segment)

        # Out-of-range source index must fail
        bad_src = edges.copy()
        bad_src[0] = np.uint64(3) | (np.uint64(1) << np.uint64(32))
        with self.assertRaises(ValueError):
            validate_packed_edges(3, bad_src)

        # Missing neuron in dst_sorted / mid-neuron last=1 must fail
        bad_dst = np.array([0, 0, 2, 2], dtype=np.int64)
        bad_edges = np.array(
            [
                0 | (1 << 32),
                1 | (1 << 32),
                2 | (0 << 32),
                0 | (1 << 32),
            ],
            dtype=np.uint64,
        )
        with self.assertRaises(ValueError):
            validate_packed_edges(3, bad_edges, bad_dst)


class GraphTests(unittest.TestCase):
    def test_bidirectional_ring(self) -> None:
        edges, deg = build_bidirectional_ring(5)
        self.assertEqual(edges.dtype, np.uint64)
        self.assertEqual(deg.dtype, np.uint32)
        self.assertEqual(len(edges), 10)
        np.testing.assert_array_equal(deg, np.full(5, 2, dtype=np.uint32))

        sources = (edges & np.uint64(0xFFFFFFFF)).astype(np.int64)
        lasts = ((edges >> np.uint64(32)) & np.uint64(1)).astype(np.int64)
        self.assertEqual(int(lasts.sum()), 5)
        np.testing.assert_array_equal(sources, np.array([4, 1, 0, 2, 1, 3, 2, 4, 3, 0]))
        np.testing.assert_array_equal(lasts, np.array([0, 1, 0, 1, 0, 1, 0, 1, 0, 1]))

    def test_directed_graph_with_isolated_node(self) -> None:
        # 4 nodes: 0 -> 1, 2 -> 1; nodes 0, 2, 3 have in-degree 0 and need self-loops.
        edges, deg = pack_edges(4, np.array([[0, 1], [2, 1]], dtype=np.int32))
        edges_i64, deg_i64 = pack_edges(4, np.array([[0, 1], [2, 1]], dtype=np.int64))
        np.testing.assert_array_equal(edges, edges_i64)
        np.testing.assert_array_equal(deg, deg_i64)
        self.assertEqual(len(edges), 5)
        lasts = ((edges >> np.uint64(32)) & np.uint64(1)).astype(np.int64)
        self.assertEqual(int(lasts.sum()), 4)
        # Conservative out-degrees: original out_deg=[1, 0, 1, 0] + self-loops on [0, 2, 3]
        np.testing.assert_array_equal(deg, np.array([2, 0, 2, 1], dtype=np.uint32))


class AcceleratorDriverTests(unittest.TestCase):
    def _make_mock_env(self, base_phys: int = 0x0100_0000):
        allocated: list[_FakeBuffer] = []
        runs: list[dict[str, int]] = []
        next_addr = [base_phys]

        def fake_allocate(shape, dtype):
            buf = _FakeBuffer(shape, dtype, next_addr[0])
            next_addr[0] += max(4096, buf.nbytes)
            allocated.append(buf)
            return buf

        def on_start():
            rm = ip.register_map
            runs.append(
                {
                    "state_in": (rm.state_in_2 << 32) | rm.state_in_1,
                    "state_out": (rm.state_out_2 << 32) | rm.state_out_1,
                    "iteration_count": rm.iteration_count,
                    "reseed": rm.reseed,
                    "J": rm.J,
                    "dt_over_e": rm.dt_over_e,
                }
            )
            in_buf = next(b for b in allocated if b.physical_address == runs[-1]["state_in"])
            out_buf = next(b for b in allocated if b.physical_address == runs[-1]["state_out"])
            # Echo state_in -> state_out with +1 iteration marker on u
            u_in, v_in = unpack_states(in_buf)
            out_buf[:] = pack_states(u_in + 0.01 * rm.iteration_count, v_in)

        ip = SimpleNamespace(register_map=_StrictRegisterMap(on_start))

        class FakeOverlay:
            def __init__(self, bitfile: str, download: bool = True):
                self.bitfile = bitfile
                self.download = download
                self.net_accel_0 = ip
                self.freed = False

            def free(self) -> None:
                self.freed = True

        return FakeOverlay, fake_allocate, allocated, runs

    def test_strided_ping_pong_and_cleanup(self) -> None:
        overlay_cls, alloc_fn, allocated, runs = self._make_mock_env()
        edges, deg = build_bidirectional_ring(4)
        u0 = np.zeros(4)
        v0 = np.ones(4) * -0.5

        with FHNAccelerator(
            "dummy.bit", _overlay_cls=overlay_cls, _allocate_fn=alloc_fn
        ) as accel:
            res = accel.run(u0, v0, edges, deg, steps=5, stride=2, J=-0.25)

        self.assertEqual(len(runs), 3)
        self.assertEqual([r["iteration_count"] for r in runs], [2, 2, 1])
        self.assertEqual([r["reseed"] for r in runs], [1, 0, 0])
        self.assertEqual(runs[0]["J"], q824_to_u32_reg(-0.25))
        self.assertEqual(runs[0]["dt_over_e"], q824_to_u32_reg(1e-3 / 0.1))

        # Ping-pong verification: output buffer of step i becomes input buffer of step i+1
        self.assertEqual(runs[1]["state_in"], runs[0]["state_out"])
        self.assertEqual(runs[2]["state_in"], runs[1]["state_out"])
        self.assertEqual(runs[2]["state_in"], runs[0]["state_in"])
        self.assertEqual(runs[2]["state_out"], runs[0]["state_out"])

        self.assertEqual(res["times"].tolist(), [0.0, 0.002, 0.004, 0.005])
        np.testing.assert_allclose(res["u_final"], 0.05, atol=1e-6)
        np.testing.assert_allclose(res["v_final"], -0.5, atol=1e-6)
        self.assertTrue(all(buf.freed for buf in allocated))

    def test_cma_address_window_guard(self) -> None:
        overlay_cls, alloc_fn, allocated, _ = self._make_mock_env(base_phys=0x2000_0000)
        edges, deg = build_bidirectional_ring(4)
        with FHNAccelerator(
            "dummy.bit", _overlay_cls=overlay_cls, _allocate_fn=alloc_fn
        ) as accel:
            with self.assertRaises(RuntimeError):
                accel.run(np.zeros(4), np.zeros(4), edges, deg, steps=10)
        self.assertTrue(all(buf.freed for buf in allocated))

    def test_max_neurons_guard(self) -> None:
        overlay_cls, alloc_fn, _, _ = self._make_mock_env()
        edges, deg = build_bidirectional_ring(4)
        with FHNAccelerator(
            "dummy.bit", _overlay_cls=overlay_cls, _allocate_fn=alloc_fn
        ) as accel:
            with self.assertRaises(ValueError):
                accel.run(
                    np.zeros(MAX_NEURONS + 1),
                    np.zeros(MAX_NEURONS + 1),
                    edges,
                    deg,
                    steps=1,
                )

    def test_invalid_packed_edges_guard(self) -> None:
        overlay_cls, alloc_fn, allocated, runs = self._make_mock_env()
        edges, deg = build_bidirectional_ring(4)
        bad_edges = edges.copy()
        bad_edges[-1] &= np.uint64(0xFFFFFFFF)
        with FHNAccelerator(
            "dummy.bit", _overlay_cls=overlay_cls, _allocate_fn=alloc_fn
        ) as accel:
            with self.assertRaises(ValueError):
                accel.run(np.zeros(4), np.zeros(4), bad_edges, deg, steps=1)
            self.assertEqual(allocated, [])

            res = accel.run(
                np.zeros(4), np.zeros(4), bad_edges, deg, steps=1, validate_edges=False
            )
        self.assertEqual(len(runs), 1)
        self.assertEqual(res["u_final"].shape, (4,))
        self.assertTrue(all(buf.freed for buf in allocated))

    def test_stiff_epsilon_uses_dt_over_e_without_q824_overflow(self) -> None:
        overlay_cls, alloc_fn, allocated, runs = self._make_mock_env()
        edges, deg = build_bidirectional_ring(4)
        with FHNAccelerator(
            "dummy.bit", _overlay_cls=overlay_cls, _allocate_fn=alloc_fn
        ) as accel:
            accel.run(np.zeros(4), np.zeros(4), edges, deg, steps=1, dt=1e-6, epsilon=1e-4)
        self.assertEqual(len(runs), 1)
        self.assertEqual(runs[0]["dt_over_e"], q824_to_u32_reg(1e-6 / 1e-4))
        self.assertTrue(all(buf.freed for buf in allocated))


if __name__ == "__main__":
    unittest.main()
