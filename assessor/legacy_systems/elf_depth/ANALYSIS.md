# T09 assessor analysis and withheld transfer package

The original AX-PR-T09 criterion IDs, objectives and seven-day delayed IDs remain
unchanged. Use their original tasks/rubrics plus this package; do not award learner
credit for executing the reference producer. `support.json` binds every ordinary
and changed-condition observation to its exact original criterion. Generated
original sources, argv, diagnostics, checks and sample values live in the full
source-bound receipt, including intentionally failing attempts in timestamped runs.

## Byte inspector and independent expectations

The reference reads bytes with bounded shifts. It first checks every outer extent,
then referenced strings/symbols, then segments; publication occurs only after all
checks. `offset<=length` followed by `count<=(length-offset)/entry_size` avoids both
addition and multiplication overflow. ELF32 address wrap must use the32-bit address
bound even on a64-bit host. SHT_NOBITS is not a file extent. Zero symbol names need
no string scan; a nonzero offset must terminate within the actual string section.
Resource caps bound repeated scans, not just individual names. Accept equivalent
checked readers; reject native-struct casts, byte-order guesses and success after
an unsupported extended count. Output bytes must stay unchanged on rejection.

The Python `struct` oracle decodes independently constructed valid records and
tool-produced original files. Specific malformed outcomes are declared separately.
Seed90817 byte mutations compare memory safety and deterministic outcomes among
six compiler profiles; that comparison is not an independent validity proof.
The ED_MUTANT_SEGMENT control accepts a filesz>memsz record, demonstrating the
named property violation without a crash. The unsolved learner rejects a valid
fixture and therefore must not pass the visible contract. Accessor argument and
index failures are checked separately under both sanitizers. The Arm object audit
detects implicit helpers; it is not a board-execution claim.

The stated PT_LOAD gives initialized memory[0x400000,0x400030), then32 zero bytes
through0x400050. Removing section headers leaves the loader's program-header facts.
ET_REL/ET_EXEC/ET_DYN classification must precede interpreting addresses. Static
PIE and a shared object both being ET_DYN refutes suffix/type-only role inference.

## Ownership, archives and relocation

For main,A,B the unresolved set starts{a}, becomes{b}, then empties. In main,B,A,
B is initially unneeded, and A later introduces an unresolved b. Backward demand
for tail in a second A member needs another pass or a group. Group repetition does
not extract a constructor-only member lacking a demand edge. An explicit anchor
extracts one member; whole-archive also exposes an unused conflicting definition.
`ar r` retaining an omitted member is the stale-archive discriminator.

Weak/common and exported ELF visibility are tool/ABI mechanisms, not C storage
classes. `-fcommon` can merge tentative definitions but does not repair conflicting
initialized definitions or authorize a portable multiple-definition contract.
Use internal linkage for distinct helpers; use one authoritative public helper
for a shared operation. Visibility cannot repair incompatible source ownership.

For PC32,4096−4−4112=−20, encoded as EC FF FF FF. Absolute32 encodes00 10 00 00.
Moving only S changes both results; moving S and P together preserves their
difference, not an absolute value. REL stores A in the field; RELA carries it in
the relocation. Signed8 permits−128..127;236 and−20 sharing EC does not authorize
236. The actual original large-address link must fail for relocation overflow,
with a known-good nearby link, rather than because a tool or file is absent.

## Dynamic binding, TLS and startup

Inspect objects and final images: hidden visibility permits local binding and may
eliminate a PLT entry. Interposition must identify the provider in binding logs;
11/22 alone does not establish lookup origin. The missing dependency test isolates
one original DSO, restores it, then relocates the whole package. An origin-relative
RUNPATH does not imply arbitrary recursive dependencies inherit all search rules.

Under the declared loader, lazy function resolution can wait until first call;
data relocation cannot. Separate processes make the fatal lazy-call failure
observable without losing other evidence. Static PIE still invokes self relocation;
the inspected2.42 source call and observed host2.43 symbol are different evidence.
Do not claim identical uninspected loader internals.

The TLS test barriers establish worker existence before dlopen, publish the lookup
function, keep every TLS instance live during address comparisons, and join before
module release.10/11,10/12,10/13 and40/41,40/42,40/43 distinguish independent state.
The tagged source shows DTV capacity/module entries, deferred allocation, template
copy and zero suffix. It does not grant a live pointer after a thread/module ends.

GNU priority hooks yield ABMyx; the custom entry yields M, ABM or ABMyx according
to which consumer exists. Metadata cannot execute itself. Custom callbacks use
only original state and one declared raw Linux write, with no libc/TLS/allocator.
No undefined symbols in the linked image supports that narrow closure claim.

Timing reports retain warmups and raw paired samples. Spawn cost, scheduling,
cache state, relocation type and first-call work confound simple causal claims.
The writable-page count uses a4096-byte model and does not measure physical dirty
pages. Require equivalent42 behavior and separate static/dynamic quantities.

## Layout and retention

Data copy intervals are[0x10080,0x10090)→[0x20000,0x20010); zero-fill is
[0x20010,0x20028). A startup table at0x10040 and gap padding make the raw extent144
bytes despite only16 initialized RAM bytes. The ELF additionally has headers/debug
metadata; RAM residency is40 bytes. Shifting the data LMA by32 changes raw extent
and source records while retaining RAM addresses. A96-byte section cannot be split
implicitly across two64-byte banks; two explicit48-byte sections can be placed.

The linker-symbol array convention is implementation-bound. Converting to integer
addresses and checking bounds makes the chosen plan auditable; it does not create
an ISO C object at a synthetic address. The host model validates all copy/zero
records before committing any destination write. Actual target startup belongs
to its separate T10/T11 execution contract.

KEEP applies only after the linker has an input section. The stage matrix includes
no compiler emission, no archive extraction, no retention, and no enumeration.
A pointer table creates real callback relocation edges; numeric16/32 entries do
not. The numeric table is never invoked. Plain and LTO versions require different
intermediate evidence but the same final behavior and ownership contract.

ALIGN32(17)+16=48; ALIGN32(33)+1=65. Source sizes alone miss padding and stack
reservation. A budget failure must identify the intended assertion. Optimization
may eliminate objects whose values are known; inspect retained sections instead
of charging source declarations as though they all survived.

## ABI evolution and transformed artifacts

The request gate reads only the available8-byte header before interpreting later
fields. Available extent and advertised size are independent. Version1/size12
defaults flags to zero; size16 permits bit0 and retains addition overflow checks.
Result stays unchanged on all rejection paths. Guard-page early-read mutant
failure is an intentionally faulty local specimen, not evidence of exploitability
beyond the demonstrated read. It is distinct from a constraint-compilation fixture.

The version matrix checks old→old11, old→new11, new→new22, two named missing-version
failures, and same-version semantic break22 versus11. Public layout8→16 changes
offsets and is never tried against an unsafe old layout. Opaque allocation/read/
destroy preserves an unchanged client binary while the provider owns allocation
and destruction. Returning borrowed memory behind unchanged C types is still a
contract break: checked release returns44, without freeing an invalid pointer.

The sparse raw span is0x1012−0x1000=18. HEX checksums validate each encoded record;
they are not cryptographic authentication. Gap fill materializes absent bytes.
The debuglink CRC and build ID correlate companion artifacts; a changed base has
a changed address identity. The wrong companion must be rejected for CRC mismatch,
not an absent debugger. The checksum image excludes index7 when computing the
stored sum; the alternative all-byte zero-sum stores its additive inverse. Require
exclusive output creation and matching prepatch hash; reapplying silently fails.

## Marking and delayed release

Use0 for an incorrect semantic claim, unsupported dependency or missing artifact;
1 for matching an ordinary output without its cause;2 for the original exact
criterion with distinct positive/negative evidence;3 only for the original delayed
change independently derived and assessed. Do not substitute one ELF parse for a
loader, TLS, reachability or ABI criterion. The full receipt's72 observations are
author-reference packages, not72 learner results. Release each original changed
condition after at least seven days, collect pre-experiment predictions, withhold
this file/reference implementation, and record assistance separately. Alternatives
are valid if they preserve the original criterion's invariants and discriminators.
