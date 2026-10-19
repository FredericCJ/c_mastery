# T32 fault, tool, coverage and replay followthrough

This additive package retains PR-T32-04 through PR-T32-07, their original
criterion objects, hours, fixtures and minimum seven-day variants. The exact
task map is `criteria.json`. Read the original project before this contract.
The two default implementations remain incomplete. Build a learner version:
`python build.py --kind transaction --output tx`; substitute `coverage` for
the mutation component. An assessor source is never imported implicitly.
Hosted diagnostics, Python, GDB, pthreads and QEMU are harness dependencies;
they are not requirements of the C17 transaction component.

## Acquisition and external progress

Use `transaction.h` to make the distinction between local ownership and
external effects explicit. The normal acquisition order is 10,20,30. A
failure before index2 owns 10 and20, so rollback must release20 then10,
leave no live identities, and preserve every byte of the caller's owner
record. A successful call transfers all three identities; destroying that
record consumes its ownership exactly once. Counts alone cannot establish
this: releasing20 twice could have the correct total while leaking10.

The adapter has four explicit operations. Acquisition can fail; release is
infallible under its precondition; write reports -1 timeout, zero no progress,
or an exact positive committed prefix; cancellation is a separate predicate.
Input is at most16 live bytes. The transaction checks a positive count against
remaining before addition or forming the next pointer. Any other negative
count is invalid. Zero and timeout terminate the current operation immediately;
there is no hidden unbounded retry policy. Successful short returns strictly
increase committed bytes, bounding calls by16 for a nonempty message.

For five bytes with returns2,0, report ZERO and committed2. All local resources
roll back, but the two external bytes remain published. Cancellation observed
after the first2 has the same residual prefix with a distinct CANCEL status.
Do not promise external atomicity from local cleanup. A protocol needing
atomic publication requires a remote staging/commit/abort design and a
recovery contract for ambiguous acknowledgements; that is a changed API.

The fourth-resource challenge inserts15 between10 and20, producing
10,15,20,30. Enumerate all five outcomes, including success. Validate acquired
and released identity sequences, not just counts. The public tests attempt
same-instance reentry from the writer: BUSY must return before acquisition or
output mutation. This is a diagnostic guard, not a same-process memory sandbox.
The callback is still trusted to respect all C object bounds and lifetimes.
It must not overwrite control objects, longjmp out, destroy the transaction,
or call it concurrently. Unsupported reentrancy remains prohibited by contract.

`process_adapter.c` implements the changed structural boundary with one owned
child and a pipe. It admits exactly five response bytes: transaction token7,
request sequence9, and a three-byte big-endian count. The parent validates
extent, token, sequence and count<=5 before committing. Tests include an
oversized count, stale sequence and truncated reply; all children are reaped.
No pointer is serialized. The costs include copied bytes, framing, kernel
crossings, timeout policy and child/descriptor lifetime. A count response alone
does not attest that an external device received bytes; the example has no
external device. Protection is separate address spaces, not arbitrary trust
in child-supplied data. It is not a privilege or capability sandbox.

## Dynamic tool mechanisms and their costs

The original `diagnostic.c` has independently selected heap overrun,
use-after-free, uninitialized branch and signed overflow cases. Each runs in
a short-lived owned process. Its repaired compilation initializes storage,
keeps access within four integers, preserves lifetime through the read, and
avoids overflowing addition. Do not run the uninstrumented defective program
directly: its plain binary is intended for Memcheck. Do not run ASan under
Memcheck. The runner keeps native ASan, native UBSan, plain Memcheck and TSan
profiles distinct. The native race target uses two owned threads and a shared
counter; the repair protects the complete increment with the same mutex.

Record actual diagnostics rather than requiring one universal wording.
Addressability and initialization are different properties: ASan's silence
on the uninitialized branch does not establish a value was initialized.
Memcheck's definedness tracking reports a consequential uninitialized use.
UBSan's arithmetic report is a separate mechanism. TSan records selected
instrumented conflicting accesses, not all possible schedules or logical
atomicity. A startup failure, absent runtime, compiler rejection, killed
process or timeout is infrastructure evidence, never the required semantic
diagnosis. A known defective control and repaired control probe sensitivity.

`workload.c` performs exactly512 passes over4096 integers with a public modular
formula and checksum. Compare plain, ASan and UBSan code/data sizes, process
peak RSS, outer elapsed time and inner monotonic nanoseconds. Inputs and
workload stay fixed; preserve compiler, flags, binary and runtime options.
Three samples expose some variation. Outer `time` resolution is0.01s and
includes startup; peak RSS describes the process, not live payload or virtual
address reservation. Instrumented elapsed time is not production latency.

If instrumentation changes allocation layout, quarantine or scheduling and a
timing failure disappears, preserve both configurations. Do not infer a fix.
Use a named logical event relation and repeat with instrumented/uninstrumented
builds; retain an explicit unresolved allocator/timing hypothesis until
discriminating evidence exists. No timing threshold is fabricated here.
On a Cortex-M target lacking a hosted sanitizer runtime, run portable logic
on the supported Linux route and cross-build the profileC component separately.
This preserves a target gap: host sanitizer results do not cover MMIO, ISR
interleavings or actual target ABI. Compiler upgrades invalidate probe receipts;
rebuild positive and negative controls and compare mechanisms, not filenames.

## Static contexts, public admission and warning records

`packet_analysis.c` admits opcode1, count<=4 and exactly2+2count bytes. It
publishes only after validation. `packet_runtime.c` tests len0,1,2,9,10,11,
count4 and unchanged rejection output. The private decoder requires a
validated extent and count. Eva's default entry explores length0..10 with
a complete ten-byte object. Case1 deliberately bypasses validation with
count5; case2 checks length, calls a mutating callback, then uses the obsolete
range; case3 rechecks after that callback. Analyze cases1/2; do not run their
invalid accesses natively. Run public controls and compare the coverage scope.

A private precondition is not an implemented public check. A caller that
bypasses the wrapper must establish the same extent/count facts or use the
wrapper. Calling with count5 cannot be justified by writing count<=4 in a
comment. The callback case demonstrates why a local range proof expires when
another operation can mutate its source. Snapshot immutable input or revalidate
after the call; also state lifetime and concurrent-modification premises.

Retain the exact diagnostic, source hash, entry context, analyzer version,
precondition argument, responsible owner and invalidation regression in a
suppression record. The supplied record does not install a global suppression:
it rejects hiding the real invalid-entry alarm. Any narrowly accepted
precondition-based disposition is attached to that specific entry context.
If warning locations change after an analyzer upgrade, correlate function,
access mechanism, object extent and caller assumptions; do not match only line
numbers. Re-run valid and bypass callers before reusing the disposition.

## Coverage, meaningful mutations and equivalent predicates

For A&&B, TT,FT,TF yield true,false,false. In FT, B is supplied true but is not
evaluated. A's independent-effect pair is TT/FT, holding B fixed; B's is TT/TF,
holding A fixed. Statement, branch, condition evaluation, path and MC/DC goals
are different. TT/FF covers both decisions but cannot isolate either term.
`models.py` independently derives full truth relations, evaluated terms and
candidate pairs. For (A&&B)||C, derive new pairs with other input truths fixed,
and mark C skipped when A&&B succeeds. Reusing the old pair table is invalid.

`coverage.c` has a bounded queue, authorization result, rejection counter and
cleanup ledger. The weak suite pushes only two values, checks a rejected
return and ignores rejection state/cleanup. Capacity, hidden-effect and cleanup
mutants survive it. The strong suite adds exactly the fourth push, a fifth
refusal with preserved queue values, rejection count and zero live resources.
Capacity< versus<=, inverted authorization and omitted cleanup must be killed
by their intended assertions. A compile failure or unrelated diagnostic is
not a semantic kill. Required observations are public contents/count, effect
counter and resource ownership, not private ring indexes.

The hidden counter mutant changes data while the weak suite follows the same
branches. Compare its actual weak coverage map with the correct control before
adding a stronger assertion. Coverage percentage does not certify state
effects. The defensive count>4 return remains in source: initial count0 and
the sole successful push preserving count<=4 make it unreachable under this
API. Record that invariant as the exclusion; don't erase code for a score.

The runner probes installed GCC condition coverage and preserves gcno/gcda,
gcov text, source and binary identities. It separately compiles prime-path
instrumentation with limit1. A function skipped because a path limit is
exceeded is uninstrumented, not fully covered. Manual truth pairs remain a
separate argument and cannot be replaced with a branch percentage.

For integer count, count<5 and count<=4 are equivalent at every integer,
including an expanded domain containing5: both reject5. The exact delayed
question is deliberately a premise check, not permission to invent a
counterexample. count<4 differs at4. If the revised contract accepts5, both
original predicates are equally wrong; repair the contract boundary, then
reclassify mutations against it. A formerly excluded full-capacity boundary
is a coverage deficiency only if it is now admitted and observable.

## Debugger hypotheses and replay authority

The retained endian defect in `debug_component.c` reads a live two-byte object
but reverses wire order. Before execution predict three hypotheses: wrong
extent would show n!=2 or missing bytes; stale ownership would show inactive
owner/lifetime; bad encoding predicts bytes12,34, live_owner1, n2, returned
0x3412 instead of0x1234. Match GDB symbols to the exact ELF. Inspect arguments,
memory and disassembly. An optimized local may be unavailable: inspect the
System V x86-64 argument registers and the pointed-to bytes at function entry,
then follow actual instructions. Register names are platform-specific evidence.

Reducing to bytes00,01 preserves the same endian disagreement (256 versus1).
An equal-byte case00,00 passes both and is a negative discriminator. Length2
and live_owner1 are part of the minimal environment; deleting a byte violates
admission and removes the claim. Save original and reduced bytes directly.
Changing the generator while retaining its seed can change generated bytes;
the historical saved bytes, generator version and hash are the replay authority.

`release_case.c` has a separate release-only fault: acquisition is inside an
assert expression. NDEBUG removes its side effect; optimization alone is not
the cause. Debug/release and repaired builds run with GDB disabled and enabled.
The repair executes acquisition explicitly and checks its result. This is a
controlled configuration failure; do not invent a scheduling explanation.
For the changed schedule condition, reuse the separately identified original
pthread schedule target with six legal read/write orders. Its atomic load then
store can lose an update without a C data race; fetch_add repairs the operation.
Controlled event order is not arbitrary stress or a weak-memory proof.

## Host, emulator, board and traceability

The shared packet vector01,02,12,34,ab,cd sums to48641. `guest.c` decodes bytes
explicitly and invokes SVC. Run on identified QEMU mps2-an385 and lm3s6965evb
machines with the same Cortex-M3 image and separately bound commands. Successful
exception entry/return is evidence about each modeled core, not board routing,
UART baud, ADC accuracy or interrupt timing on physical hardware. The adapter
uses semihost output; it does not pretend to exercise a UART.

The dedicated board protocol in `BOARD_CAPTURE.md` is intentionally unfilled.
Reuse the exact bytes and logical sum while recollecting transport, endian,
clock and instrument observations. Native-layout decoding would give a
different sum on a little-endian host; an explicit big-endian byte fold works
independently of native representation. A simulated endian calculation is a
counterexample analysis, not an executed big-endian physical target.

Reject reports claiming ADC accuracy from host samples or physical baud from
simulated ticks. A calibrated voltage source, reference meter and analog setup
are required for ADC accuracy; baud needs a known timebase and capture method.
An unknown timebase cannot produce a defensible numerical accuracy bound.
The model's104us example is illustrative: combine the stated100ppm calibration
assumption and0.5us quantization before inverting the period interval.

Every criterion has its own teaching, artifact, observation selector and exact
changed condition. Retain raw failure before repair, versions, flags, source,
ELF/symbol/input/log hashes and commands. SHA256 of abc must match the supplied
fixture; the zero digest must fail. Source/tool/context changes invalidate
receipts. The guard deliberately rejects changed sources, logs, missing
mechanisms and invented physical or learner claims. If an independent reviewer
challenges an assumption, preserve the observations and narrow or revise the
claim; a review scenario is not an actual independent review.

Submit D1 predictions and contracts, D2 implementation or exact premise
analysis, D3 raw discriminating checks, and D4 the original delayed variant
after its minimum interval. Level0 misses the property; level1 repeats outputs
without effects/context; level2 meets the complete exact criterion with a
counterexample; level3 rebuilds the argument under the original changed
condition unaided. Author runs do not establish learner mastery.
