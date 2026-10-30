"""Author entry point for the measurement gates.

The gates are defined once, in labs/support/measurement_gates.py, so that the
learner runner (labs/performance_followthrough/native.py, which must not import
anything from tools/) and the author campaigns apply exactly the same rules.
This module loads that file under a private name and re-exports its interface.
"""
from __future__ import annotations

import importlib.util
from pathlib import Path

_PATH = Path(__file__).resolve().parents[1] / "labs/support/measurement_gates.py"
_SPEC = importlib.util.spec_from_file_location("cm_support_measurement_gates", _PATH)
_MODULE = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(_MODULE)

SOURCE = _MODULE.SOURCE
TIMING_FLOOR_NS = _MODULE.TIMING_FLOOR_NS
timing_target_ns = _MODULE.timing_target_ns
calibration_state = _MODULE.calibration_state
evaluate_regime = _MODULE.evaluate_regime

__all__ = ["SOURCE", "TIMING_FLOOR_NS", "timing_target_ns", "calibration_state", "evaluate_regime"]
