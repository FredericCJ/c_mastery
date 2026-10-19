# T11 assessor analysis and withheld transfer package

Use this with the thirty original AX criteria and their exact `-DELAYED` IDs.
This file is assessor-only. Public `free.c`, `services.c` and `audit_image.py`
are intentionally unfinished; their rejection is expected. The source files in
this directory are reference artifacts, never learner submissions. Author runs
do not supply independent review, learner performance or the seven-day delay.

Start with the learner's prediction, not the reference output. Require a trace
or argument tied to each listed criterion. All original ordinary and transfer
requirements remain mandatory. A reusable artifact may satisfy two criteria
only if the assessor separately examines their different claims.

## T11.01.a–c: boundaries and conditional claims

The decimal parser can have no allocation or runtime import while its test
process is hosted and linked to libc. Freestanding is a language/environment
contract, not a spelling convention for an entry symbol. The raw Linux image
has custom startup and no libc but explicitly depends on kernel mappings,
stack setup and syscall behavior. Admitting the compiler-runtime archive changes
the runtime policy without making that image hosted or allocation-dependent.
An acquisition callback removes direct knowledge of its implementation; the
actual original host adapter calls malloc and frees after return. The absence
of malloc from `callback.o` is the counterexample, not evidence of whole-call
allocation freedom.

For .01.a assess all five axes for all five specimens, with the boundary stated.
The full campaign emits five concrete dependency manifests with actual header
hashes, imported object reports, final reports, service/ownership statements and
separate classifications. It imports the unchanged T10 decimal implementation;
no reconstructed stand-in receives integration credit. The distribution's default
stack protection added an import in the decimal translation unit's formatters.
Its default object stays recorded; the bare component explicitly disables that
instrumentation while its hosted harness keeps normal CRT/library dependencies.
Do not infer either closure from source includes alone.
For .01.b require both archive and whole-image closure, and identify the host
snapshot oracle separately. For .01.c require the explicit callback producer
contract plus headers, imports, final origins, syscalls and storage acquisition.
The transfer changes an actual producer or execution boundary. A diagram
without those bindings earns at most level1.

## T11.02.a–c: distinct compiler stages

`__STDC_HOSTED__` records the compilation mode; it does not audit link inputs.
The driver experiments separately demonstrate retained CRT and libc inputs for
the three exclusions. `-nostdinc` tests declaration availability, not whether
code generation needs support. The bad LP64 header has an8-byte `uint32_t`;
the static assertion is the intended translation rejection. A missing compiler
or sysroot is not an equivalent rejection. For .02.c transfer, inspect the actual
type-size symbol on both host and Arm instead of merely checking filenames.

The first Clang/lld no-library draft retained `PT_INTERP` and faulted in the
host loader. The passing successor explicitly selects static linkage and checks
segments before execution. Preserve that failure as a causal example: successful
link, no undefined symbols and no `DT_NEEDED` did not remove the interpreter.
No platform-independent meaning is assigned to the observed loader crash.

## T11.03.a–c: lowering and provider ownership

The fixed-size and runtime-selected operations have identical byte contracts
but different possible lowerings. O0/O2/Os/LTO and actual alignment change the
evidence that must be revisited. Treat “no helper observed” as a statement about
one generated object/image. The standard-symbol provider delegates only to
original primitives and retains exact argument order and return type.

The first hosted optimized provider produced a conditional self-loop in its
`memset` body. The corrected build compiles the provider under its own restricted
contract; the hosted consumer remains a separate experiment. The deliberate
assembly recursive specimen is inspected without running it. Distinguish direct
function recursion from a normal branch to an offset inside the same body.
This distinction caught a false positive in the first audit parser.

.03.a requires inspection of several sizes and aligned32-byte output, .03.b
requires the bounded runtime lengths0..64 and actual byte oracle, and .03.c
requires signatures, ABI, final closure and duplicate ownership rejection.
Passing one64-byte case does not satisfy these other obligations.

## T11.04.a–c: arithmetic derivation and corrected premise

Let P_k be the k-bit processed prefix. Initially q=r=P_0=0. At an ordinary
iteration append bit b, let u=2r+b, select t=(u>=d), then r'=u-t*d and
q'=2q+t. From0<=r<d follows0<=u<2d, hence t is0 or1 and0<=r'<d. Also
P_{k+1}=q'd+r'. More strongly r<=P_k<2^k. Before every shift in exactly128
iterations k<=127, so r<2^127. There is no129th-bit carry in this domain.

LSYS-T11-CARRY-2026-01 corrects the earlier explanation while retaining the
accepted implementation. Deleting its redundant carry test is an **equivalent
mutant**. Reject any assessor claim that a finite test killed it as semantically
wrong. Conversely, in the streaming API the only bound is r<d: the sample
r=2^127+3,d=2^127+7,b=1 yields u=2^128+7 and (qbit,r')=(1,2^127).
The low128-bit subtraction works because after the required single subtraction
the true result is representable. An explicit high limb or a correctly proved
comparison-before-shift technique is also acceptable.

`fr_divmod128` refuses divisor0 and preserves outputs; accepted `mi_divide`
returns0. Source-level division by zero has neither contract. LLVM's selected
helper operates on its own TI type/ABI and has no promised local refusal status.
The native original provider uses the same nonzero ABI and traps a direct zero
call; the Arm environment provides the documented div0 hook. The Arm assembly
adapter preserves stack alignment and returns q in r0/r1 and r in r2/r3.
An ordinary C struct return would introduce a different hidden-result convention.
The direct zero-helper assembly probe is legal as an ABI experiment; C `n/0`
is not used to reach it.

For .04.a require actual object observations for division, multiplication and
conversion, and prior exclusion of exceptional signed/conversion inputs. For
.04.b require the selected helper declaration/type, disassembly and executable
Arm adapter. For .04.c require both invariant proofs, actual baseline import,
independent Python integer reconstruction and cost records. The helper-policy
benchmark uses identical nonzero inputs/checksums; refusing the operation is a
different capability. Compare section bytes and compiler stack reports as well
as the full host timing distribution. Do not promote elapsed maximum to WCET.

## T11.05.a–c: support introduced by verification

The aligned host128-bit atomic requires a helper and reports not lock-free in
the recorded environment. Arm64-bit stores also expose helper requirements.
Those observations do not prove the helper uses a particular lock strategy;
reject the ISR proposal without an adequate implementation/reentrancy bound.
Do not manufacture misalignment and then reason about the result as defined C.

The ASan positive control originally optimized away its malloc/store/load, so
the process returned37 without a diagnostic. The corrected specimen retains
the actual volatile memory access. A symbol alone would have missed this vacuity.
The UBSan control checks a real overflow diagnostic; profiling and coverage
must create fresh files for the current run. Do not reuse old `gmon.out`.

The sanitized host-only build does not instrument the archive's internal
loads/stores; its claim differs from the separately instrumented component.
The original no-library add-overflow handler has the emitted ABI and exits91.
It implements only that declared fail-stop operation, not arbitrary UBSan state.
For stack protection under LTO, retain the original guard/failure provider
explicitly: the first draft discarded it before implicit references appeared.
There is no production security claim for a constant teaching guard value.

## T11.06.a–c: memory proof and detection limits

Copy maintains that the already-written prefix equals the original source
prefix and that unread source bytes remain available under nonoverlap.
Move chooses forward order for lower destinations and backward order for higher
ones under its declared address model. In the offset API those comparisons are
between bounded integers in one arena. Byte comparison treats every byte as
unsigned and promises only sign. Checked admission must precede any write.

The1785 triples enumerate extents within one16-byte arena, not all C pointer
representations, alignments or lifetimes. Guard checking is independent of the
snapshot contents. The signed-byte and forward-copy mutants compile and fail
the intended assertions. Alternative staging implementations are valid if their
storage contract and resource costs satisfy the project, but delegating to host
memmove does not meet the first-principles requirement.

.06.c requires objects and final images under both compilers and LTO, including
rejection of the recursive provider before execution. The call-graph parser
normalizes exact function-entry edges separately from internal loop branches.
Changed compiler support is a rerun obligation, never inherited evidence.

## T11.07.a–c: concrete library and transport contracts

The missing-hook link establishes seven actual environment imports of the
selected newlib-nano formatted-output proposal. Source inspection traces printf
through the reentrancy/FILE setup and stream writer, plus buffer allocation
fallback and write/heap wrappers. It is not a whole printf or allocator audit.
The actual guest uses a caller-owned24-byte stdio buffer, accepts9 bytes in at
least five writes, and surfaces a later refusal through fflush/ferror. Absent
heap/read/stat/close/seek facilities are executed through refusing hooks with
unchanged supplied output. No fake success is used to satisfy linkage.

The original raw24-byte service has a much smaller representation and no FILE,
formatting or heap-extension dependency. That contrast does not remove the
need for a live output transport. Its progress invariant is0<=offset<=used<=24;
accepted bytes are permanent progress even when the returned status is terminal.
The bounded arena is monotone and checks padding without overflowing `used+n`.
Exhaustion preserves both offset and output pointer.

Host sink, semihost service and CMSDK UART are different contracts. The UART
image has no BKPT instruction and the host receiver stops the VM only after
collecting its exact frame. The selected semihost profile assumes the QEMU
extended-exit operation. Debugger availability is a provisioning condition,
not guessed by executing a trap on arbitrary hardware. Missing physical UART
or debugger observations are not manufactured from emulator success.

## T11.08.a–c: initialization and retirement

Validate all RAM intervals, ROM extent, stack alignment and pairwise overlap
before copying any byte. The33 cuts cover interrupted initialization including
zero progress and completion. Replaying data/BSS initialization is idempotent
because its immutable ROM and zero source remain available; retained bytes
belong to a different lifecycle. A real reset requires a separate target proof.

Four explicit task instances demonstrate that initialization metadata, storage
creation and hook invocation are distinct. Early and duplicate hooks reject.
Destroy marks a handle inactive; reuse increments generation; UINT_MAX retires
the slot. Shared storage is the compiled mutant. General concurrent access would
need synchronization absent from this single-owner model. Do not call this
automatic ELF TLS, language-enforced ownership or a general thread runtime.

The retry client must distinguish NOT_READY from UNSUPPORTED. Repeatedly retrying
permanent refusal is not recovery. Future service readiness requires an explicit
owner transition and must not retroactively legitimize an early hook.

## T11.09.a–c: audit assurance boundary

The unchanged accepted auditor passes a fully linked original forbidden helper:
there are no undefined symbols, dynamic libraries or interpreter. The successor
checks origin policy and final symbols/edges. With LTO, original input identities
and retained generated machine objects are necessary because code may be
inlined or renamed. The inlined forbidden input is rejected by provenance even
when no forbidden function name survives. This is a trusted build-capture
contract, not a proof against a malicious compiler or forged evidence collector.

Reject reports with missing fields, failed tool status, invalid/truncated text,
overlong lines/data, mismatched hashes, missing symbols or unsupported map syntax.
Strip-symbol and changed-policy controls must fail. A nonzero semantic test is
not interchangeable with an intentional unresolved-link or malformed-report
rejection. The recursive specimen must never run; a crash would not establish
that the dependency audit detected recursion.

The declared matrix includes GCC/Clang O0/O2/Os plus supported LTO, stack
instrumentation and their combination. Per-profile source, object, map and final
evidence is required. References to another profile's green row earn no credit.
Indirect calls and arbitrary inlined behavior retain explicit limitations.

## T11.10.a–c: independent transfer and evidence categories

The snapshot, unsigned ordering and unbounded-integer oracles do not invoke the
implementation to compute expectations. The host arithmetic generator supplies
literal independent quotient/remainder vectors to the target. The target links
only the declared implementation/provider; the host oracle never becomes its
runtime dependency. The two-byte transport quota tests framing/composition.

The support matrix must separate native x86 execution, Cortex-M4 emulation,
other compile-only profiles, pointer-order assumptions and untested interrupt/
physical contexts. A portable offset contract removes unrelated-pointer ordering;
it retains live arena, extent, integer-width and single-owner assumptions.

For delayed assessment, withhold this file and generated reference outputs.
Select the existing criterion-specific changed task, provide its actual public
contracts/fixtures, record artifact hashes and scheduling date, and assess only
after the original minimum7days. Use the exact corresponding level3 rubric;
do not substitute one narrow target test for the other leaf objectives.

Graduated hints: first ask which boundary owns the disputed property; next ask
for its smallest state transition or object artifact; finally supply one missing
precondition, without the algorithm or expected trace. Valid alternate solutions
include different division representations, staging under an allowed storage
bound and equivalent fail-closed audit implementations. Reject superficial
completion that confuses a successful reference run with independent performance.
