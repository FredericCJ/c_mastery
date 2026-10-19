# Local original vulnerability demonstrations

These complete flawed examples are intentional learner input, not assessor
solutions. Only this process's synthetic8-byte heap object or local integer
sentinel is a target. Run in a disposable directory with no credentials/input
files/network. Attacker capability is selection of one fixed teaching transition,
not arbitrary addresses or a third-party program. Heap target requires ASan;
run each mode in a new subprocess, bounded timeout, and retain exact diagnostics.

`bounds` writes one past the allocation; `lifetime` reads after free; `double`
releases twice. A diagnostic establishes the detected local violation, not
arbitrary execution or elevation. `repaired` demonstrates one owner, in-bounds
access and no use after release. Extend repairs in learner code with failure
injection and regression tests; compare sanitizer-free runs without interpreting
silence as safety. The arena exercises supply deterministic property witnesses.

format_target deliberately supplies the correctly typed pointer for %n. Flawed
format interpretation changes sentinel91 to3; the constant-format repair prints
literal abc%n and preserves91. This is a bounded integrity demonstration with
defined arguments, not a demonstration of arbitrary-address writes. Removing
percent characters alters data and does not express the intended literal-copy
contract. Compare platform format restrictions; record mitigation and limits.

## Implemented continuation

The required [concrete continuation](EXTENSIONS.md) supplies the additional source,
fixtures, failure controls, objective criteria and bounded author execution.
Its evidence does not replace independent learner or physical obligations.

Required changed-width, safe-language comparison and scan-code continuations are specified in [TRANSFER.md](TRANSFER.md).
