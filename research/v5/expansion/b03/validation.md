# B03 first-wave validation

Actually executed on 2026-10-17: all five canonical briefs passed individual parse/validate structural checks. Reference compilation uses C17 and `-Wall -Wextra -Wpedantic -Werror`.

Executed through WSL from `/mnt/f/Learning/Curricula/c_mastery`:

```
cmake -S labs/b03_depth -B /tmp/cmastery-b03-assessor -DCMASTERY_ASSESSOR_ROOT=/mnt/f/Learning/Curricula/c_mastery/assessor -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/cmastery-b03-assessor -j4
ctest --test-dir /tmp/cmastery-b03-assessor --output-on-failure
```

Observed 11/11 passed: five semantic anchors, five compiled mutation controls and one actual two-thread SPSC smoke test with 20,000 monotonic payloads. The deadline anchor checks 4,194,304 admitted start/elapsed/delay combinations. The concurrency smoke is bounded observed behavior; the order-policy mutation is an explicit source-contract check, not an observed data race or an exhaustive C memory-model result.

The full source-bound compiler campaign is separate. No physical MMIO, interrupt timing, OS cancellation primitive, weak-architecture run, model checker result or learner mastery is claimed. State models do not replace remaining domain obligations, which stay explicit in b03_depth_disposition.json.
