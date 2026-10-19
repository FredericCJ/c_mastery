# T72 component substitutions and evidence

Build each learner followthrough C file with its supplied header/test and the
learner's actual earlier coroutines artifact. query_follow uses co_query as an
equivalence target; effect_follow uses COEffect; fiber_follow uses the real
fiber C/assembly backend. channel_follow additionally imports async_follow and
the learner's actual messaging artifact (MsgHub). Source paths must be explicit
in exported workspaces; assessor paths are never default dependencies.

The author runner tools/run_coroutines_followthrough.py deliberately selects
assessor implementations and records their hashes. That is author execution,
not execution against a learner's imported artifact. Repeat with learner sources
and record exact import identities before claiming the original integration
gate. The original X-T72 rubric and delayed minimum still apply.

The actual local Linux socket/readiness adapter in labs/modern_composition,
especially MC-REAL-IO in its CONTRACTS.md, supplies a different composition:
one owner serializes COAsync while a joined readiness worker guarantees its
synchronous disarm prerequisite. Its fallible close contract does not change
COAsync to asynchronous disarm. Compare its ownership trace with CAFrame's
explicit pending acknowledgement rather than treating their APIs as equivalent.

The parser and expression extensions here are original bounded mechanisms.
No external parser, effect or coroutine library replaces them. The explicit
threaded adapter depends on POSIX; the guarded fiber depends on the selected
Linux x86-64 ABI; neither is part of a strict freestanding target image.

## Actual final five-import adapter

integration_follow.h declares ci_parse_cursor and ci_receive. The source map is:

| Artifact and original producer | Concrete imported implementation | Use |
| --- | --- | --- |
| bytes.cursor, PR-T03-02 | foundation.h / f_take | Stage cursor reads; commit position only after parser success |
| parser.integer, PR-T12-03 | foundation.h / f_parse_i32 | Strict complete signed-decimal payload conversion |
| io.reactor, PR-T24-04 | hosted.h / hs_reactor_sum | Collect two borrowed pipe handshakes, each announcing its payload byte contribution |
| io.deadline, PR-T24-05 | hosted.h / hs_read_until | Read combined payload against absolute CLOCK_MONOTONIC deadline and cancellation pipe |
| io.stream, PR-T19-04 | api_depth/stream/stream.h / sf_start, sf_pump | Own and transmit length/payload/checksum frame with short-write progress |

The last implementation is a newly authored concrete successor, not a previously
inspected accepted library. Its contract is registered separately and does not
retroactively modify acceptance. The first four use the actual earlier reference
sources in author runs. A learner substitutes their exported revisions, records
interface conversion and reruns the same histories before claiming learner
integration. Header/type compatibility alone is not that evidence.

ci_parse_cursor accepts a live FCursor and the original bounded parser DAG.
It parses the remaining region with co_parse, then advances a staged cursor
through f_take only on success. Mismatch, invalid graph and resource exhaustion
preserve position and consumed output. The parser's diagnostic step count may
change. A zero remaining region avoids pointer arithmetic on null.

ci_receive requires two distinct borrowed handshake pipes, one data pipe and
one cancellation pipe, all kept live and exclusively read by this operation.
Each handshake contributes one length byte; the sum must be1..8. These are
original local teaching participants. The earlier reactor has its own timeout
policy; the absolute deadline applies to the subsequent receive stage, not to
the handshake stage. This is a bounded blocking receive adapter: each COAsync
read callback invokes hs_read_until and may block until input, cancellation or
deadline. It is not the nonblocking readiness worker described above. There
are no asynchronous callbacks in this adapter, so disarm's quiescence is immediate.

COAsync accumulates exactly the announced bytes into owned frame storage. EOF
before completion rejects; deadline/cancel/system failures return IO with the
underlying port_result distinguished in CIProgress. Its accepted byte count is
partial progress, not rollback. Successful complete conversion stages a fresh
SFStream and publishes the numeric result and bound handle together. Stream
destination must be fresh, with no old handles; malformed decimal syntax leaves
all three outputs unchanged. Neither output publication nor failure can undo
consumed handshake/data bytes. CIProgress is diagnostic output on failure.

The declared exercise input has two one-byte contributions and payload "42";
it publishes integer42 and [2,'4','2',102]. Pump that frame into another original
pipe with quotas2,0,1,1. Check EOF after "4", malformed "4x", already expired
deadline, cancellation tied with readable data, and out-of-range length sum.
Cancellation wins its receive-stage tie and leaves unread data for observation.
No adapter closes borrowed descriptors. The test owns and closes every pipe.
An attacker controls only synthetic length/data/event order. A premature cursor
advance or incorrect published number is the bounded defect witness; repair and
rerun the matching mutant. Failure is not an arbitrary code-execution claim.

Run tools/run_coroutines_imports.py for six-profile reference/stub/two-mutant
checks of both the stream producer and composition. Record exact source pins,
partial counts and syscall outcomes. The original delayed variants, per-leaf
rubrics and modern cost review remain separate learner obligations. Storage is
bounded by one eight-byte read frame and one66-byte outgoing frame; syscall
latency and receiver backpressure have no hard real-time bound.
