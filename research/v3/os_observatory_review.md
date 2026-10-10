# Independent OS observatory and runner review

Reviewed 2026-10-10. Scope: `labs/os_observatory`, `assessor/os_observatory`, and the target/external-source/physical-capture/ThreadSanitizer additions in `cmastery/cli.py`. This review assesses the stated hosted teaching experiments and evidence boundaries, not learner mastery or a production OS.

Resolved findings:

- Replaced deprecated `ATOMIC_VAR_INIT` with `atomic_init`; Clang's warnings-as-errors previously stopped both debug and sanitizer builds.
- A directory containing a file named `cgroup.controllers` was insufficient evidence of a real cgroup. The runner now checks the containing cgroup2 mount, including nested mounts and escaped mount paths.
- A failed write to `cgroup.procs` could previously leave a child failure labeled as an observed constrained workload. A close-on-exec pipe now acknowledges actual entry and distinguishes entry/exec failure. Cleanup allows a bounded wait for killed descendants before removing the child cgroup.
- Mutation rejection alone could pass for an already-broken baseline. Both ordinary baseline mechanism cases must now pass before the two mutants are tested.
- Selecting `freertos-posix` for an unrelated T49 project could label an ordinary model test as a FreeRTOS comparison. The comparison target now declares its supported project IDs and the runner checks them. A target-filtered `--all-tests` run is described as the selected target suite.

Validation: all eleven family checks pass under GCC debug/release and Clang debug/ASan, and the untouched learner scaffold compiles but rejects its behavioral gates. Eight focused regressions in `os_observatory_review_checks.py` pass, covering target compatibility, missing/dirty pinned dependencies, failed TSAN runtime probing, submitted capture/tampering, false cgroup directories, denied entry, and mount resolution. The matrix aggregate was intentionally not accepted because other source files changed concurrently; `reviews/v3/lab_os_observatory.json` contains the passing scoped family results. Root will rerun global receipts after freeze.

Actual Linux and pinned FreeRTOS POSIX comparison programs both passed the eight-slot, 256-item, checksum-32640 workload. Their shutdown mechanisms differ: Linux closes and drains; FreeRTOS sends an explicit terminator. This is documented and does not establish identical API semantics or MCU timing. The process-local SCHED_OTHER probe ran. FIFO, RR, and DEADLINE probes returned unavailable with EPERM; delegated cgroup execution returned unavailable with EACCES. These are retained as unavailable, not passed. Commands and raw results are in `reviews/v3/os_observatory_review_runs.json`.

The physical importer only records submitted capture files and content hashes; acceptance still requires assessment. A changed source or copied artifact makes its reported receipt stale. No physical board experiment, privileged scheduling guarantee, hard real-time bound, or race-freedom proof was established by this review.
