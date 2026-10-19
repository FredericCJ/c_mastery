# MCU capstone assessor analysis

This file is withheld from learner exports. The original twelve criterion rubrics,
seven-day named variants and capstone shared contract remain normative. The
reference campaign is author evidence only. A level2 response must reconstruct the
specific mechanism below and explain every failing control; a level3 response must
independently transfer it under the exact named delayed task, defend assumptions
and identify evidence that would falsify the conclusion. Running supplied answers
or reporting a green test cannot earn independent mastery. Require the four
deliverables in the public contract and the original objective-specific rubric.

## T20.01.b

The same P3 parser/encoder, P4 ring/clock and CsService operate behind acquisition,
foreground and output-lease interfaces. The six samples produce108 identical wire
bytes under sequential and grouped scheduling. Ring capacity4 means three usable
slots, not four. ISR acquisition never calls a blocking output routine. The ordinary
status/out-parameter contract returns `CF_PENDING` for no progress and does not
manufacture a sample. The actual errors.result comparison has matching value/error
semantics and callback suppression, but target header/whole-object inspection
exposes setjmp/longjmp dependencies. Runtime tags do not create static ownership.

The delayed polling-to-DMA task imports actual PdTransfer. Cancellation retains
both64-byte buffers through a late device write, disabled-but-not-drained state and
matching completion observation. Stale generation1 cannot complete generation2.
Only matching EN0/FIFOempty/TC admits finalization; cancelled data is not framed.
Require an ownership/context diagram and explain why numeric address admission is
not proof of a live allocation or physical bus reachability. Hints: distinguish
request from acknowledgement; draw the last possible writer; identify the exact
point at which a new epoch may reuse memory.

## T30.07.b

The required synthetic ledger sums1896, so4096 leaves2200. For2048, adding160 makes
2056; refusal must preserve outputs and prevent enable. This number is not the
reference image's actual RAM size. Inspect each actual map and static `.su` record.
The explicit selected bound has no recursion/reentrant callbacks and at most two
distinct nested handlers. Three copies of the sum of all compiled static frames,
including discarded functions, plus two36-byte basic-frame/alignment allowances
and128 bytes conservatively overcount this selected program. It is deliberately
loose, not a measured high-water mark. A differently optimized image needs its own
records; passing a64KiB test image does not establish admission under4096.

The actual Xf producer reports basic8 and extended26 hardware words: FP increases
each frame by72 bytes before software saves/alignment. For two such frames the
delta is144. Lazy reservation does not make FP values readable. A level3 answer
changes the nesting/FPU ABI assumptions and recomputes all terms rather than
adding144 to a number whose premises no longer hold. Hint sequence: separate
static sections from runtime stack; enumerate simultaneous contexts; identify
which tool observations are upper bounds and which are merely samples.

## T35.01.a

The threat table distinguishes CRC-malformed input, CRC-valid unauthorized command,
telemetry flood, downstream stall, clock timeout and reset. BOOT, RUNNING,
RECOVERING and SAFE_STOP force safe actuator0 outside admitted RUNNING operation.
No command effect is applied until an owned bounded queue slot is admitted.
Authorization is a trusted argument in this local teaching target; a payload flag
cannot supply it. The authority mutant causes a harmless synthetic state violation.

Delayed replay retains the latest sequence/value across recovery. An identical
repeat is idempotent; conflicting value or older sequence refuses. This namespace
retires at65535 instead of silently wrapping. The update-slot serial-number policy
is a different explicitly bounded contract, not permission to reuse command
numbers. Hints: separate integrity from authority; place effect after admission;
list which replay state survives each reset policy.

## T27.10.b

Both compiler families build original startup. Actual MPS2 reset begins with
poisoned data/BSS. Copy-bound and omitted-BSS mutants reach named startup rejection;
the wrong vector is rejected from bytes before execution. Reordered initialized
sections and a32-byte-aligned object require linker-derived copy limits and pass
the same entry predicates. A link-only check would miss an incorrect loop.

The F439 image separately imports the real P4 reset/clock/USART configuration and
selected Pd target adapter. It is cross-built with explicit helper closure and
correct flash origin; it is not run on MPS2 as if MPS2 were an F439. Require the
student to identify that distinction. Hint sequence: independently inspect the
first vector words; map LMA versus VMA; poison RAM to distinguish omitted startup
from a convenient emulator initialization.

## T28.11.b

Host sequential/grouped sample paths are byte-equal. The selected board source
connects actual P4 PendSV acquisition, actual Pd DMA2 memory-to-memory copy and
actual P4 polling/DMA1 USART2 output. It releases the Cs output lease only after
the imported transmission completion contract. Pending/full/timeout paths retain
the appropriate owner or explicitly count rejection; timeout cannot feed progress.
Board execution remains a required physical gate, not supplied evidence.

For delayed register semantics use the original three-flag model, not an invented
F439 register. Direct W1C acknowledgement preserves the other pending flag without
consuming read-clear state. The generic read/OR/write mutant both consumes the
read-clear bit and clears the unrelated W1C bit. The fix is a register-specific
operation; adding `volatile` does not fix the protocol. Hints: describe each read
and write independently; compute the actual bus value; compare the imported port's
selected register contract before transferring the repair.

## T31.08.b

The dossier binds source, build flags, ELF, map, extracted binary and vector bytes.
The checker re-derives the binary from the named ELF and rejects a stale ELF/new
binary pair even when each individual digest is valid. Actual stripped load bytes
must remain equal to the original image. The external symbols' hash and actual GNU
debuglink CRC bind the association; matching filenames do not. The ordinary and
changed records are generated from real artifacts, not filled-in example hashes.

The dedicated-board procedure separately requires as-built board/manual/errata
qualification, authorization, deployment verification, cold power-cycle and bound
instrument captures. A build checksum is not readback from a deployed device.
The author result remains physical=not-run. Hints: draw the source→ELF→binary edges;
replace one edge while preserving valid independent hashes; remove symbols and
show which identity remains available to the debugger.

## T31.09.b

CandidateB generation8 cannot replace committedA generation7 at cuts0..4; only
cut5 with compatible metadata, CRC and a trusted verification predicate permits
B. The author model supplies that predicate, not a signature implementation or an
attacker-writable authorization flag. Corrupt or untrusted candidates refuse.

The changed commit uses two separately written complementary words. Either torn
half rejects. Generation order is a16-bit serial window smaller than32768:
65535→0 can be newer, but equal or exactly half-range separation is ambiguous and
refuses. This is a logical cut model; physical flash shared erase domains, program
granularity and reset behavior need separate evidence. Hints: make publication
last; enumerate each persistent prefix; prove why the admitted serial window
excludes two contradictory newest choices.

## T31.10.c

Feed decisions for1,3,2,3 are false,true,false,true. Accumulating unrelated epochs
would incorrectly turn incomplete progress into health. First-fault identity stays
fixed. Two recovery attempts are permitted; the third enters safe stop. Recovery
requires acquisition/notification quiescence and no DMA/output lease; closing an
epoch accounts accepted/delivered/discarded before reset. No timeout is treated as
proof that a writer is gone.

Crash-record cut positions leave the prior committed record until all fields and
commit validate. No record means absent/incomplete capture, not absence of a fault.
The32-bit sequence retires before wrap. A counterexample must distinguish a reset
during capture from clean startup and explain which external effects cannot be
undone. Hints: separate progress epochs; retain the first cause; test every store
prefix rather than only complete and empty records.

## T35.08.c

Ordinary C pointers cross a purported module boundary; the host counterexample
changes a live synthetic object. The actual selected MPU guest creates a
higher-priority32-byte privileged-only region over user-accessible RAM. An
unprivileged store causes the specific MemManage fault at the known instruction,
and the guard remains unchanged. The no-MPU control changes it and rejects. The
handler only resumes this purpose-built instruction, not arbitrary faults.

The delayed SVC handler validates offset/count into a fixed live mailbox, copies
once into owned storage, then changes privilege. Later mailbox mutation does not
change the copied request. Caller/DMA exclusion is a real premise; this is not a
generic safe arbitrary-pointer copy. MPU CPU permissions do not constrain every
DMA/debug/NMI access. Hints: identify who can execute the instruction; separate
address validity from lifetime; identify every writer between validation and copy.

## T30.10.c

65530→6 is12 ticks under one-wrap/interval assumptions,0.75us at16MHz and1.5us at8MHz.
Converting to integer microseconds too early loses precision. The exact changed
history with an extra wrap is65548 ticks, observationally indistinguishable from12
without an epoch or interval bound. The native case uses actual CPFelapsed16 and
CPFClock; it does not measure a physical GPIO. The source-bound target budget is
not a WCET proof. Require separate timer resolution, clock uncertainty, observed
maximum, interference model and analytical claim. Hints: derive integer ticks
first; attach epochs to samples; vary clock and interference independently.

## T33.10.c

The claim matrix requires host-oracle parser evidence, actual target helper closure,
device-model DMA lifetime, target-executed startup, reset-cut model and safe-state
model. Eight original negative substitutions reject, including host timestamps
and ELF hashes mislabelled as physical capture/readback. The changed CPU-only
emulator must invalidate a claim of peripheral support even though it executes
the instruction set. Labels alone do not verify arbitrary submitted evidence;
the assessor must inspect the bound logs and scope. Hints: name the property before
choosing a tool; list what the tool cannot observe; repeat after changing the model.

## T35.12.b

The sealed matrix combines malformed/unauthorized bytes, full queues, retained
DMA/output leases, update cuts and bounded recovery. The private changed schedule
feeds a valid A5-bearing payload in13 bytes split7/6, then faults with an active
lease, presents stale completion, and finally acknowledges a five-byte irreversible
prefix under cancel. Recovery counts one accepted/discarded frame and preserves
first fault71. A late acknowledgement may end ownership but cannot retract those
five external bytes. The same reference must still admit ordinary valid work.

This is author execution of a sealed task, not independent review. A later assessor
must choose the original named delayed variant after at least seven days, withhold
this source, and require explanations and revised invariants. Physical/source gaps
must remain explicit in the final evidence dossier. Hints: reorder completion
without changing identity; keep the first cause; audit all claims against the
actual source and observation layer.
