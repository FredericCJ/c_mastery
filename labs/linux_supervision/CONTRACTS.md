# Linux process supervision

The supervisor runs as UID 1000 inside a disposable x86-64 Linux guest. The fixture prepares the cgroup leaf, PID and mount namespaces, loopback interfaces and experimental conditions. Implement the selected function in `src/`. Read `PROTOCOL.md` for the API, synchronization protocol, workload and serial fields.

## First run

1. In your exported workspace run `python3 guest.py setup`. This downloads checksum-pinned Ubuntu kernel and static BusyBox packages to your user cache. Run `python3 guest.py smoke --out build/smoke-1` to test a fresh guest. Set `CMASTERY_LINUX_GUEST_CACHE` to share an existing cache. Subsequent guest runs use local inputs.
2. Create `answers/CASE.json`, replacing CASE with the case in your exercise brief. Begin with a JSON object containing a nonempty `prediction` string. Write it before the first observation. Keep code, original prediction and later explanation together. Each case is a standalone workspace, and you can copy your own earlier helper implementations forward.
3. Build with `cmake -S . -B build/learn -DCMAKE_C_COMPILER=gcc` and `cmake --build build/learn`. Use `python3 submission.py observe CASE --program build/learn/supervise --out build/first-CASE`. Choose a fresh output directory per attempt. The runner preserves the dated prediction, serial transcript, kernel and executable identities and the guest exit results.
4. Read `build/first-CASE/guest/serial.txt`. Add an `explanation` string to the same answer, citing the relevant rows and the observation that discriminates the competing explanations. For telemetry also supply `diagnosis`, an object mapping each variant to its supported cause. The accepted labels are `none-observed`, `cpu-throttling`, and `pipe-backpressure`. Use `none-observed` for the baseline only when both discriminating signals are absent.
5. From the course checkout or learner archive run `python3 -m cmastery check WORKSPACE --compiler gcc --profile debug`. It rebuilds your supervisor and checks a fresh run. The CLI receipt binds the submission to that build. Static hosted C17 builds use GCC or Clang; the guest interfaces are Linux-specific. A skipped capability is reported as unavailable. For a separate offline check of an already recorded transcript run `python3 submission.py replay CASE PATH/TO/serial.txt`.

## Evidence

Use positive controls, a changed condition, and at least one independent observation. For example, distinguish the namespace-relative IDs by joining the outer observer's live namespace inode readings to the inner process and its worker's pipe report. The outer fixture samples the cgroup leaf before release and after reaping. An errno or a printed success marker alone cannot establish cause. In the capstone scope every CPU and application counter to the same finite batch and compare the reply count, checksum and digest across all three runs.

Offline replay checks the recorded fields. The live check records a fresh observed run. Written explanations and delayed transfer are assessed against the exercise rubric. The assessor's private reference, author runs, and grading variants are held outside the learner workspace.

The semantics of namespaces and task limits are documented in [pid_namespaces(7)](https://man7.org/linux/man-pages/man7/pid_namespaces.7.html), [network_namespaces(7)](https://man7.org/linux/man-pages/man7/network_namespaces.7.html) and the [kernel cgroup v2 documentation](https://docs.kernel.org/admin-guide/cgroup-v2.html).
