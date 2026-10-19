# Equivalent mapped and explicit record reads

## Identifier, objective and families

Required successor for `AX-PR-T23-03-T23.09.b` and its existing delayed
sparse-random-access variant; B/C/D/F/G/H/I/J. Preserve original identity,
rubric and historical receipt. Explain why a mapping is not simply a cheaper
read with identical failure and lifetime assumptions.

## Inspected sources

Reuse the actually inspected Linux mmap/pread passages registered as
R-LMAP-MMAP and R-LIO-PREAD, plus the inspected original `map.h`, `pm_map`,
`pm_copy` and isolated `fault_helper.c`. The prior mapping campaign records
actual isolated SIGBUS after truncation and SIGSEGV after protection change.
That evidence is checked for source/log freshness and explicitly reused, not
invented as a new execution. This exercise adds the missing equivalent workload.

## Prerequisites and learner artifacts

Bring completed `PR-T23-03` mapping/borrow/generation work and its error ledger,
`PR-T23-02` short-I/O cursor, and the original logical-versus-rounded extent
derivation. Replace only `workload/record_stub.c`. Import the completed mapping
source/header explicitly for the comparative experiment. Do not substitute an
unchecked pointer loop for the validated mapped operation.

## Environment and assumptions

Hosted C17 on Linux, signed off_t of at least32 bits, local owned regular files
and libc/POSIX. Workload extent is at most1MiB; this guarantees representable
offsets on the chosen ABI. Record length is at most64 bytes. No other actor
changes file content/size during the comparison. Separate failure tests violate
that size assumption deliberately, in owned programs. No cache dropping, host
policy changes or physical-device control is authorized.

## Concrete task and inputs

Implement `pw_read_record`: validate logical extent/capacity before reading,
loop over short reads/EINTR, distinguish unexpected EOF from syscall failure,
and publish output only after obtaining the whole record. Supplied tests use
67 bytes containing1..67, a32-byte record at offset4, a record crossing logical
EOF, zero length at/beyond EOF, insufficient capacity, EINTR plus3-byte reads,
EIO after three bytes, and actual truncation to63 bytes. The shared descriptor
offset must remain67 after positioned reads.

The comparison creates a file of15 pages+13 bytes, maps16 pages, and writes
32 patterned bytes at the start of each even page. Holes read as zeros. Both
adapters validate a32-byte request at logical-end-minus8 as an error with output
unchanged. For each access pattern, verify8192 complete32-byte records against
each other and an independent expected byte function before timing. The first
pattern walks sequentially with wrap; the changed pattern uses the specified
32-bit LCG sequence across the logical extent. Eight paired repetitions use
ABBA order, with mapping setup reported separately.

## Why separate assessment is needed

A mapped last-page tail can be readable yet lie outside the logical file.
Accepting it gives a faster program with a weaker contract. A short pread must
not publish an incomplete record. A successfully established mapping still
depends on the backing file's later size and protections. Correct bytes alone
do not justify comparing different validation or failure policies.

## Invariants, errors, ownership and resource costs

Borrow the live fd and disjoint output for the call; externally serialize their
lifetimes. `logical<=1MiB`, `length<=64`, `offset<=logical` and
`length<=logical-offset` are checked before offset addition/conversion. Validate
capacity before any effect. Errors return positive errno: ERANGE for logical
bounds/capacity, EINVAL for contract size/null violations, ENODATA for actual
EOF before completion, otherwise the syscall's error. Output remains unchanged
on every error. Zero length at logical EOF succeeds without I/O. Retry EINTR;
no finite latency is promised under endless interruption. A64-byte automatic
staging buffer provides all-or-nothing output publication.

Mapped reads use the same32-byte copy and logical bound through `pm_copy`,
including the live generation. They require a stable file-size/protection
assumption. The timed loop includes each adapter's validation, copy and the
same digest; setup, independent byte verification and cleanup are outside it.
Both therefore transfer262144 application bytes/sample. Explicit reads perform
8192 observed pread dispatches in the stable ordinary case; mapped reads call
none. This is a wrapped-library-call observation, not a syscall-tracing claim
or proof that mapped loads cannot fault or invoke kernel work.

## Deliverables

Submit the record implementation, proof of unchanged-output errors and exact
short-read cursor trace. Supply every paired raw timing, CPU-time, page-fault
delta, wrapped pread count and digest. Explain mapping setup amortization and
the warm prevalidation state. Retain separate logical-EOF rejection, actual
short-EOF error and previously executed process-fault evidence. State which
controls must be repeated for random access, larger files, another filesystem
or a mutable backing object.

## Verification and pass/fail

Run `python workload/run_workload.py --mapping-source PATH --mapping-include DIR
--implementation PATH`. The normal record and benchmark targets print
`PW_PASS`. Compare exact output/error behavior and all8192 records of32 bytes
per workload before accepting a timing sample. The campaign
requires32 raw samples/profile and matching digests for both methods. No speed
ordering is a pass criterion. Debug, release and sanitizer measurements remain
separate distributions. The host's `_FORTIFY_SOURCE` lowering may call
`__pread_chk`; the harness wraps it as well as pread while preserving the real
checked call and bound. It does not disable fortification to make injection work.

Controls must compile then fail a named `PL_CHECK:`: logical-bound relaxation,
discarded short-read suffix, partial output publication and a mapping that uses
rounded extent. A crash/compiler/sanitizer failure is not semantic detection.
The unimplemented learner stub must also compile and fail. Reused isolated
fault outcomes require their original source and log hashes to remain current.

## Rubric

Level0 violates bounds/ownership or invents results. Level1 reports an ordinary
checksum/timing without equivalent error contracts. Level2 requires all explicit
read cases, mapped logical-EOF rejection, equal copied records and the distinction
between reported error and process fault. Level3 requires the original delayed
random-access investigation with independent justification of measurement,
cache, lifetime and failure controls. Author data cannot award learner level3.

## Hints and assessor separation

Start with an unsigned subtraction guard. Accumulate into private staging until
the record is complete. Keep logical length separate from mapping length. Then
separate setup, validation, transfer, digest and page-fault costs before comparing
timings. Private reference/control analysis is under
`assessor/posix_lifecycle_depth/workload`.

## Variants and integration

The named sparse random variant is executable over the same file, record sizes,
validation and output contract. Larger files, cold-cache experiments, concurrent
truncation or explicit synchronization require new stated assumptions and a new
campaign. Do not extrapolate this small warm workload to physical disk throughput
or an unconditional preference for mappings. Link actual target evidence only
when that target was used.

## Containment and security

The target is an original owned temporary file and teaching code. Malformed
record extents expose weaker validation; truncation exposes the changed backing
object assumption. The harmless success condition is an error-contract mismatch
or a separately isolated process signal, never claimed code execution. Repair
uses logical bounds, stable lifetime assumptions and atomic output publication.
No other user's files, host cache/security settings, devices or network services
are touched. The old deliberately faulting helpers remain isolated and unsolved.

## Validation status and limits

The source-bound campaign records actual compiler/commands, raw samples and
semantic controls. Warm prevalidation does not force a known CPU-cache or
physical-storage state. Host noise and sanitizer overhead remain visible; no
general speed theorem is inferred. External review, delayed independent learner
assessment and physical observations remain separate.
