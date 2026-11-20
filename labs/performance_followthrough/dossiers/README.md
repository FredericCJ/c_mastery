# Running a performance dossier

The [worked example](../PMU_DOSSIERS.md) follows one pair from the public concurrent-state experiment through the inference it supports. The driver, equivalence tests and CPU-specific experiment specification live in [concurrent_state](concurrent_state/). The shared driver records equivalent work, acquisition status, clock calibration, scheduling, core placement and the raw counters for each process. Use `run_dossier.py test`, `run` and `analyze` with fresh output paths.

[The replay input](replay/layout.json) is a compact projection of a different historical layout experiment. It runs through `../native.py report` on a machine without PMU access. The source digest identifies the original row stream and the `index` on each trial identifies the archived row.
