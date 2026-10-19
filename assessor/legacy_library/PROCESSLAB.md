# Withheld process-state and clock analysis

Bindings: AX-PR-T12-04-T12.14.a/b/c and their unchanged DELAYED identities.
The public specification supplies mechanism, inputs and acceptance obligations;
withhold this analysis and processlab*.c/producer files from learner exports.

The time wrapper rejects -1 before comparison, converts before subtraction and
publishes last. Integer subtraction followed by casting is not a repair for
signed overflow. Floating subtraction can lose low tick bits; a valid alternative
uses checked integer differences only under a stated integer clock_t profile.
Reject claims that CLOCKS_PER_SEC fixes granularity, that process CPU time must
equal elapsed time, or that a multicore aggregate cannot exceed elapsed time.
The twelve actual samples include warmup, three CPU/wait/thread repetitions,
one-CPU threads and owned process contention. Neither a noisy ratio nor a small
observed maximum is a pass/fail semantic oracle. Require the workload checksum,
exact affinity policy and successful teardown; timing-only output can conceal
optimized-away work or leaked competitors.

getenv returns a borrow. Bounds must be checked before committing any bytes, and
NUL is part of the owned representation. The mutant omits that NUL and fails a
pre-filled destination check, rather than relying on an accidental crash. A
correct snapshot made under a lock survives later protected updates. Copies
made without synchronizing lookup remain invalid in the presence of writers.
The fixture has1000 writes and1000 snapshots and one shared mutex; it does not
prove that an arbitrary application/library honors that mutex. A startup copy
with no later environment mutation is acceptable if failure/ownership and all
callers are covered.

Normal events are BA, quick events YX, immediate events empty. Normal buffered
stdout must contain "buffered:"; the other cases retain either observed choice
per the documented implementation contract. The event descriptor is a different
owned file from stdout, avoiding undefined active-handle interaction. Exit
handler output alone says nothing about crash recovery, file durability or
cleanup of every resource. system is reasoning-only: untrusted command text
can be interpreted as syntax by the host command processor; changing quoting
without a complete grammar is not a portable proof. No real shell target is
necessary to assess that distinction.

For UTC2000 the expected copied calendar is February29. A later conversion may
overwrite static library storage but must not change the caller's earlier copy.
time32 rejects2040 under the selected glibc ABI with EOVERFLOW; time64 accepts
year140 in tm_year. Require actual ELF symbol/loader evidence and retained
runtime hashes. Rebuilding a64-bit host executable with a32-bit integer typedef
would not test this ABI. ISO does not require time_t to encode seconds since1970.

After at least7days provide the three original changed tasks and fresh timestamp
and scheduling premises, not these results. Ask the learner to choose a coherent
environment policy when a plugin cannot participate in the lock, to explain
whether clock conversion loses information on a hypothetical wider integer
clock, and to propose a range-checked interchange format. Full credit requires
revised premises, implementation/analysis and evidence limits, not copying the
native table. Reject assertions that an unrun foreign representation or a finite
contention sample demonstrates portability/fairness. No learner/reviewer/board
evidence is inferred from the reference campaign.
