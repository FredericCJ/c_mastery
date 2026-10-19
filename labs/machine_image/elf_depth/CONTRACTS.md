# T09 successor: bounded ELF inspection and link-stage diagnosis

This required package extends the existing T09 projects without changing their
identifiers or accepted evidence. It covers all original T09.01–12 criteria and
their changed-condition packages. Each exact evidence scope is recorded separately;
author execution never substitutes for a learner attempt.
Use each original AX criterion and its separate seven-day DELAYED variant.

## Borrowed-file inspection

Prerequisites are T04 checked arithmetic, T05/T06 byte representation and lifetime,
T07 artifact provenance, and T08 ABI/data-model contracts. Implement `elf_depth.c`
against the supplied header; `inspect.c` is a hosted transport, not target code.
The implementation may use only its own byte decoding and arithmetic. It may not
call libelf or cast file storage to native ELF structs. The target core allocates
nothing, owns nothing and retains an immutable borrowed span. The caller keeps
that span live and unchanged until it stops using the view and borrowed names.
Outputs must be disjoint from file bytes. On failure, output values remain intact.

Support ELF32 and ELF64 in both endian encodings, with ordinary table counts.
Explicitly reject extended section/program counts and symbol indexes. This is a
bounded inspector; do not claim full ELF conformance, loader safety, relocation
execution or signature validation. Treat OS/processor-specific fields as reported
data unless the declared contract interprets them.

Bounds are16MiB/file,4096 sections/program headers each,16384 symbols total,
1024 bytes/name, and8MiB aggregate name scanning during validation. Distinguish
unsupported capacity from malformed structure. This prevents a tiny repeatedly
referenced string table from demanding unbounded repeated scanning. Report the
algorithm's actual O(table entries + scanned name bytes) time and constant storage.
Use subtraction/division bounds before multiplication or pointer formation.

`ed_open` first establishes the class/order and header extent, then all table
extents, then referenced string/symbol information, then the program segments.
Only a fully validated view is published. A zero-length or bad-magic input does
not authorize further access. SHT_NOBITS has runtime size without file payload.
PT_LOAD initialized bytes precede a zero-filled suffix; filesz cannot exceed memsz,
the address interval cannot wrap, and the declared alignment must be consistent.
Accessors require a successful unchanged view and reject indexes outside it.

Concrete initial input: ELF64 little-endian, PT_LOAD offset0x1000, address0x400000,
filesz0x30,memsz0x50. Derive file [0x1000,0x1030), initialized memory
[0x400000,0x400030), and zero-fill [0x400030,0x400050). These are model intervals,
not instructions to map arbitrary host addresses. Then inspect your own ET_REL,
ET_EXEC and shared ET_DYN artifacts. Strip only section-header metadata from a
copy of the original executable and establish which program-header facts survive.
Static PIE adds another ET_DYN role; the type alone cannot identify its startup owner.

The host transport accepts repeated four-byte little-endian lengths followed by
the corresponding file bytes on stdin and emits one JSON record per input.
`header`, `sections`, `programs` and `symbols` preserve the field order documented
by their public structs; the header array is bits,order,type,machine,entry,
section-count,program-count,string-table-index. A symbol row begins with table and
entry indexes and ends with a hexadecimal name string. No attacker-controlled
name is used as a format string or terminal control sequence.

```text
cc -std=c17 -Wall -Wextra -Werror -O2 -I. inspect.c elf_depth.c -o inspect
./inspect < original-cases.bin > observed.jsonl
python3 run_core.py --implementation ./elf_depth.c --cc gcc
```

For target compilation, omit the host transport and audit undefined symbols and
the final linked image separately. Struct assignment can introduce an implicit
copy helper despite `-ffreestanding -fno-builtin`. A successful compile is not a
closure result. Original byte helpers are permitted only when explicitly supplied,
implemented and audited under the target profile.

## Symbol ownership, extraction and relocation

Track unresolved and defined symbols after **each** link input. Build main→a→b
using separate objects and archives A/B, where b returns42. Compare orders before
running. Introduce a backward reference from B to another member of A, then justify
the smallest repeat/group policy. An archive groups members and an index; it is
not a completed link. Updating members does not necessarily delete stale ones.

Keep C linkage, weak/common extensions, archive extraction and dynamic exports
distinct. An internal helper accidentally defined twice has an ownership defect;
allowing duplicate definitions is not its repair. A direct strong definition can
win over a weak definition, while an unextracted archive member never reaches that
competition. `-fcommon` is a labeled tool experiment, not permission for conflicting
portable C definitions. Explicit initialized definitions provide the changed case.

Constructor-only registration demonstrates the difference between extraction and
execution. Compare normal extraction, a minimal archive group, one explicit anchor,
and whole-archive. Retain maps, symbol origin and actual included section sizes.
Add an unused conflicting definition: whole-archive can expose it while targeted
extraction remains valid. Later garbage-collection exercises must additionally
distinguish extracted sections, retained relocation edges and the runtime walker.

For a declared PC-relative relocation use S+A−P, where P is the actual relocation
field location, not an assumed next-instruction address. The supplied
S4096,A−4,P4112 gives−20. Compare with an actual relocation-bearing object and
linked bytes. A REL record stores the addend in the field; RELA carries it in the
record. Signed8−20 and236 have the same truncated low byte but different validity.
Derive the admissible interval before encoding. A compiler or linker launch error
is not an overflow rejection; the expected diagnostic needs a known-good control.

## Dynamic images, binding and initialization

For T09.05, keep one source with a local integer7, visible integer5, saved pointer
to the local integer, and `step(n)=n+*saved+visible`. Require step(30)=42 in each
executable. Produce non-PIE, PIE, shared PIC and static PIE with consistent flags.
Classify type, interpreter, load segments and relocation records before execution;
the shared object is inspected rather than launched as an executable. Preserve
object and final disassembly for a visible data address and visible function call.
Repeat with hidden visibility and explain which lookup obligation is removed.

Count relocation destinations and distinct4096-byte writable load pages from the
actual ELF. This is a declared page model, not physical dirty-page measurement.
Pair at least nine measured launches per equivalent executable after two warmups,
alternating order. Report spawn+loader+program+wait separately from an in-process
dlopen/first-call measurement. Preserve every sample; do not infer that relocation
count alone explains timing. Static PIE still needs self-relocation. Identify its
owner in the image and the exact inspected runtime source version.

For T09.06, create `libtoy_plugin.so` calling a separate `libtoy_dependency.so`
operation that returns42. Load by an absolute path and a child-local search path;
remove only the original dependency copy and require a named load error. Restore
it and relocate both files together using an origin-relative policy. Do not modify
system loader configuration. In another original DSO, choice() returns11; a local
override returns22. Preserve binding logs and final GOT/PLT evidence, then hide
the original symbol and predict the changed result. Restrict all environment
changes to the child. Trusted loading here is not a sandbox for unknown plugins.

Construct an unresolved function on an uncalled branch. Compare lazy load without
invocation, eager load, and lazy first invocation in separate bounded children.
Replace it with an unresolved data reference. Preserve the named failure at the
correct stage; a generic nonzero status is insufficient. POSIX dynamic-function
lookup conventions are additional to ISO C's object/function pointer rules.

For T09.07, main and two threads begin with independent `_Thread_local` value10.
Keep all threads alive while recording addresses and values; increment by1,2,3.
Load another original TLS module, initialized to40, **after** the two threads have
started, then repeat. Barriers must publish the function pointer and prevent thread
exit before comparisons. Join threads before closing the module. Never inspect a
TLS object through a pointer after its thread exits. Explain template, module ID,
DTV entry, allocation, copy/zero initialization and lifetime as separate stages.
The inspected glibc2.42 bodies and observed host version are recorded separately.

Use GNU constructor/destructor priorities101 and202 to emit A,B and x,y; main
emits M. Relate observed ordering to the explicit priority contract. Build an
original raw Linux entry image containing the same metadata with no libc. Compare
no walker, init-only walker, and init plus reverse fini walker. Its callback
contract permits only its own bounded state and declared write syscall: no TLS,
allocation, libc, thread creation or inherited constructor machinery. Audit final
undefined symbols. Linker-defined array ranges are a compiler/linker convention,
not an ISO C promise about arbitrary separately declared objects.

## Layout, reachability and representation changes

For T09.08, use only synthetic FLASH[0x10000,0x10100) and RAM[0x20000,0x20080).
Never execute these images. Place four text bytes and four constants at0x10000,
16 initialized bytes at RAM0x20000 with LMA0x10080, and24 BSS bytes immediately
after data. Add a startup table at0x10040 containing three32-bit copy fields and
two32-bit zero fields. Derive all half-open intervals, then inspect symbols,
program headers, sections, maps and the raw image. The model does not describe a
board. Deliberately exceed RAM, shift the LMA by32, and test a96-byte indivisible
section against two separate64-byte banks. A combined capacity sum cannot make
one contiguous input section fit; define an explicit splitting policy.

Declare linker symbols as address-valued byte arrays under the selected GNU
convention. Convert to uintptr_t and validate order/count before constructing a
copy plan; do not read an absolute symbol as if it held an integer object. Validate
every startup-table interval before any destination write, including endpoint,
wrap, unknown-kind and partial-table failures. Use host buffers for this model.
Actual startup execution is the T10/T11 consumer, with its own environment contract.

For T09.09, put two callbacks returning11 and22 in a registration-only archive
member. Include an unrelated function returning99. Compare unextracted archive,
explicit extraction, direct object, omitted KEEP, omitted compiler emission, and
omitted enumeration. Each case must identify the **first** missing stage. Retained
pointer entries should retain both callbacks while section GC removes unrelated
code. Replace pointers with fixed numeric offsets and inspect the lost relocation
edges without calling them. Repeat with LTO and explain why an intermediate slim
object is different evidence from the final image.

For T09.10, independently compute ALIGN32(17)+16=48 and ALIGN32(33)+1=65 against
a64-byte static budget. Reserve the following stack range and reject overlap by
an identifiable ASSERT. Compare32 and128 BSS bytes, distinguishing the intended
budget error from unrelated linking failures. Report ELF bytes, raw load span,
initialized RAM, zero-fill RAM, retained code/constants and reserved stack as
separate quantities. Repeat equivalent constant output with optimization/section
GC and account for every retained or eliminated object; BSS still costs RAM.

For T09.11, use a native-ABI byte request prefix: uint32 version, uint32 advertised
size, int32 value, then optional uint32 flags. The available byte count is separate
from advertised size. Version1 accepts12 bytes with flags0;16 or more bytes may
carry flags bit0, which requests nonnegative input be negated before adding7.
Reject unknown flags, partial tail sizes13–15, undersized prefixes, advertised
extent beyond available bytes and positive addition overflow; leave output intact.
Use memcpy into aligned local scalars before interpreting bytes. Input/output
regions are disjoint and supplied storage must remain readable for the declared
available extent. This native layout is not a portable wire format.

Test a12-byte old caller against the extended operation, then a16-byte caller.
Gate incompatible version/size before later fields. A private teaching mutant
reads the optional field too early; a child-local guard page exposes that read.
No arbitrary-code-execution claim follows from its fault. Require the repaired
operation to reject the same input and preserve output. Inspect the dynamic export
set as well as the size/version behavior.

Build old/new shared libraries with GNU symbol versions TOY_1 and TOY_2, and
separately linked old/new consumers. Test supported pairs, a missing required
version and an implementation retaining TOY_1 while changing11 to22. Then compare
a public layout change with a changed private opaque-handle representation.
Do not execute an incompatible old public layout. Finally change an owned return
to a borrowed return while retaining C types: detect the release-contract violation
through a checked provider release API, without calling free on borrowed storage.

## Image conversion and identity

For T09.12, create sparse bytes0x1000:{01,02},0x1010:{FE,FF}. Derive the18-byte
zero-filled raw span, then use a nonzero fill byte. Decode an address-bearing HEX
output independently, checking length, checksum, EOF and nonoverlap; absent bytes
and materialized fill bytes have different meanings. State the supported record
types explicitly. No image is flashed. Target word-address conventions require
their own named device contract and are not inferred from HEX byte addresses.

Split an original debug executable into stripped image and matching companion.
Record cryptographic file identities, build ID and debuglink CRC; resolve one
symbol/address through the companion. Relink at a different base and reject the
wrong companion before trusting a source correlation. A build ID is a correlation
mechanism, not a security signature; deliberate collision resistance is outside
this experiment. Keep network debuginfod lookup disabled for these child probes.

For an original16-byte image with byte7 reserved, define two separate checksum
contracts: the sum of the other15 bytes modulo256 stored in byte7, or an all-byte
zero-sum using its additive inverse. Preserve the immutable prepatch identity,
create the postpatch file exclusively, and reject both repeated output creation
and a different prepatch identity. Independently verify every byte and both
manifests. Do not alter the debug ELF or reuse the prepatch hash for patched data.

## Assessment and containment

All inputs, binaries, DSOs and layouts are original teaching artifacts. Synthetic
link maps are not executed. Malformed ELF inputs only reach the local bounded
inspector. The attacker controls those bytes and can attempt extent overflow,
inconsistent sizes, invalid indexes, unterminated names or excessive scanning.
A harmless violation is accepting a segment whose file size exceeds its memory
size, or altering the promised output on rejection. A crash/sanitizer report is
separate from a demonstrated property failure and proves no code-execution claim.

Require per-leaf predictions, source/argv identities, decoded values, a causal
explanation and negative controls. Compare valid structures to an independently
implemented decoder and the actual tool metadata. Run both compilers and sanitizers
on boundary and seeded mutation inputs, retaining the seed and every input hash.
Deterministic agreement among C builds is not an independent validity oracle for
every mutated file. The scaffold must fail an ordinary supported input, while a
compiled range-check mutant must expose the named property violation.

Level1 reproduces an ordinary output. Level2 explains every interpreted field and
contract boundary, rejects the stated wrong cause, and supports the result with
independent evidence. Level3 performs the assigned class/order, linkage, extraction
or relocation change independently after the recorded delay. Alternative checked
readers and link layouts are valid when they preserve the same contracts. Hints:
first identify the earliest untrusted field; next draw its bounded interval or
unresolved-symbol set; finally compare the smallest two artifacts that differ.
Reference runs establish author validation only; non-author review and learner
performance are still separate obligations.

Each original AX criterion remains the stable exercise ID and its existing
prerequisites, rubric and delayed ID remain authoritative. Submit D1 prediction,
D2 implementation/artifact, D3 source-bound positive/negative evidence and D4
changed condition for **that criterion**. The reference receipt contains separate
ordinary/changed observations for all36 leaves. A shared experiment counts only
where those observations and the original rubric assess the relevant mechanism.
Require the learner to rerun under changed assumptions independently; the author
reference variant is an assessor package, not evidence of delayed learner success.
