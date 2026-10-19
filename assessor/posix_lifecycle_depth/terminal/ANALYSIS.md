# Private terminal and transport assessment analysis

The guard stores restoration duty before the effect. A failed initial get has
no valid snapshot; a failed set/readback does. Mutant1 accepts a partial-success
set that toggles ICANON; mutant2 claims restoration while leaving the raw state.
Independent named-field reads catch both. Lost PTY master makes actual restore
unavailable on the measured kernel; pending remains true rather than invented
success. The public harness owns that device, so closing it contains the failure.

The pipeline starts all three children before waiting. Child stage2 sends only
a small result, so the parent can reap then consume it without filling the
result pipe. Every stage closes all endpoints except its input/output. Mutant3
retains the parent's second data writer and prevents sink EOF; the deadline
triggers cleanup. Mutant4 observes exits with WNOWAIT but marks them reaped;
the independent public wait probe reaps those zombies and rejects the false
ownership report. Early sink exit causes bounded EPIPE propagation, not an
unreaped child. Six acquisition faults exercise every pipe/fork position.

The VMIN/VTIME test's child enrollment uses the exact child's blocked syscall,
not an IPC READY message sent before read. No-first-byte silence for MIN3/TIME1
must outlast the decisecond without completing; after A then B the interbyte
timer produces2 bytes. Host scheduling can lengthen these observations, so only
broad bounds and actual timings are recorded. This is a supplied kernel-semantics
investigation around the learner guard, not a new terminal-driver implementation.

The stream adapter imports one unchanged profile-C parser object. Its feed loop
must continue after REJECT without discarding the caller's remaining chunk and
must drain queue pressure without overwriting a completed parser frame. Mutant2
consumes the entire chunk after one rejection; segmentation cases lose later
valid frames. Mutant1 retains partial/queued parser state across close/open;
the eight-byte prior prefix plus old suffix becomes an unauthorized previous
session message. Epoch overflow refuses open rather than aliasing generations.
The CRC constants are checked separately with Python zlib. Same-object reuse
proves this adapter boundary for the chosen corpus; it is not a proof of the
parser's correctness or an authentication guarantee.

Assess each original criterion separately. Require ownership/timing derivations
and actual changed-condition traces, not merely the PT_PASS/PS_PASS text. All
reference results remain author evidence. External review, an unaided delayed
learner submission and dedicated physical serial/bus observations remain absent.
