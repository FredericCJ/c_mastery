# PR-T62-08 required direct-I/O changed-condition experiment

Use the B06 shared contract and D1–D4 from TRANSFERS.md. Families B/C/F/H/I/J;
objectives T62.08.a/b/c plus T62.07.a's data-versus-directory distinction.
Prerequisites: owned-file progress loop, durability argument, integer extent
checks and the learner `it_direct_ok` from `transfer.h`. This is Linux C17
hosted work; statx, O_DIRECT, POSIX aligned allocation and fdatasync are explicit
extensions/dependencies. They do not implement the alignment admission mechanism.

Implement `it_direct_ok(address,length,offset,memory_alignment,offset_alignment)`
as an unsigned divisibility predicate: both alignments nonzero, address divisible
by memory alignment, length and offset divisible by offset alignment. Do not
assume power-of-two masks or that page alignment suffices. The caller supplies
live buffers and a truthful filesystem query; this integer predicate cannot
establish pointer lifetime. Unaligned requests select a buffered file before
submission. A direct I/O error after partial progress is not permission to replay
the whole operation through a different path; preserve the prefix or fail with
its exact progress. The supplied workload has exact admitted aligned extents and
completes each partial suffix on its original descriptor.

Supply `direct_probe.c` as an experiment input: create only owned temporary files,
query STATX_DIOALIGN, allocate according to observed memory alignment, use a
length satisfying observed offset alignment, and create a deliberately shifted
buffer with identical bytes. Run eight repetitions per path. Direct and fallback
use separate files and both require completed writes, fdatasync(file), and
fsync(parent directory) before timing ends. Reopen and compare every byte and
EOF with an independently generated i mod251 pattern. Never infer durability
from O_DIRECT alone; do not overlap buffered and direct I/O on the same file.

D5: retain actual statx mask/alignment, path selection, byte comparisons and16 raw
timing samples per profile. Run `python3 tools/run_b06_direct_io.py`. Lack of the
queried feature or failed direct writes is an explicit unexecuted-target gap,
not a silently successful skip. Report selected compiler/kernel/filesystem and
the lack of physical persistence evidence. Describe O(n) transfer work, bounded
buffer memory, copies whose existence is known versus merely suspected, and
latency with synchronization included. No privileged cache drop is permitted.

IO9 assesses each objective separately: a requires the alignment/ownership and
durability argument; b requires actual direct/fallback execution and unchanged
data contract; c requires changed alignment/offset admission and a justified
comparison. Level0 assumes direct means durable or duplicates partial writes;
level1 supplies throughput without equal contracts; level2 satisfies D5 and
the invariants; level3 independently adapts to a different filesystem/backend
after seven days. Hints: query the file, not just the page size; test three
alignment dimensions separately; put both synchronization operations inside
both timing regions. Assessor analysis is TRANSFER_ANALYSIS.md, IO9 below.

The runner selects the private Linux home experiment filesystem and records
findmnt; the initial /tmp tmpfs observation lacks STATX_DIOALIGN and remains
a failed feature observation, not an accepted direct-path run.

Only synthetic bytes in the program's newly created private experiment directory are authorized.
Results are source-bound in direct_io_validation.json, not external review or
learner/physical acceptance. Inspected OPEN-DIRECT and FSYNC primary passages
are registered in the B06 followup sources. An unavailable physical power-cut
rig remains a separate obligation; this experiment must not claim it.
