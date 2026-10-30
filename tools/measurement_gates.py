"""Author entry point for the measurement gates.

The gates and the shared protocol constants are defined once, in
labs/support/measurement_gates.py, so that the learner runner
(labs/performance_followthrough/native.py, which must not import anything from
tools/) and the author campaigns apply exactly the same rules. This module
loads that file under a private name and re-exports its interface.
"""
from __future__ import annotations

import importlib.util
from pathlib import Path

_PATH = Path(__file__).resolve().parents[1] / "labs/support/measurement_gates.py"
_SPEC = importlib.util.spec_from_file_location("cm_support_measurement_gates", _PATH)
_MODULE = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(_MODULE)

__all__ = ["SOURCE", "REQUIRED_PAIRS", "MAX_REPETITIONS", "CALIBRATION_HEADROOM", "MEASUREMENT_CFLAGS",
           "TIMING_FLOOR_NS", "clock_cost_ns", "timing_target_ns", "row_target_ns", "acquired",
           "calibration_state", "calibrate", "rebuild_calibration", "evaluate_regime"]
globals().update({name: getattr(_MODULE, name) for name in __all__})
