# PR-T02-03: values, owners and qualified views

This successor implements the six retained T02.09–T02.10 criteria and their six
changed conditions. Keep the original IDs, project fixture, historical hour record
and assessment rubric. Prerequisites are T01 arrays/bounds/lifetimes and PR-T02-01
types, compatible interfaces and opaque ownership. Bring your own four-element
record implementation and a field-value/lifetime notebook; the learner template
is intentionally unfinished.

The component is **C17, profile C**: no source calls to an external C library,
allocator, OS, HAL, compiler-runtime archive or atomic runtime. `stddef.h` supplies
types/null constants. Implement all mechanisms from scalar operations and loops.
Compilers may generate memory helpers for aggregate operations; supply and audit
original helpers explicitly. A successful link against the host's libc does not
satisfy this boundary. The hosted printf test harness and sanitizers are verification
tools. They are not target dependencies.

Run `python labs/qualified_views/run.py --runtime path/to/your/original_runtime.c
--build build/my-views`. The reference campaign explicitly supplies its separately
packaged original helper provider; the learner export never substitutes it. The
target builder additionally takes an explicit source, runtime and driver, cross
compiles four objects, and links directly with `arm-none-eabi-ld`. AN386 emulator
observations are not physical board measurements.

## Representation, values and lifetime

`FvRecord` owns four ints and contains a borrowed `int *link`. It does not own or
free the linked object. `fv_initialize` copies a designated initializer with
indices1=7,3=9 into caller storage, initializes link to a null pointer value and
returns a four-element const view of the caller's array. NULL owner returns an
empty NULL view. `fv_reset` sets all four values to zero and link to null, returning
FV_OK; NULL returns FV_INVALID. `fv_update` writes one array element, preserving
the link and other elements; NULL is INVALID, index>=4 is RANGE with no writes.

An explicit initializer supplies values when the declaration is reached; assignment
later copies a value into an existing object. An automatic uninitialized record
does not inherit the initialization of some other object. Do not read its members
before storing suitable values. Static implicit initialization recursively supplies
zero arithmetic values and null pointer values. In the inspected N1570 rule,
implicit static aggregate initialization also initializes padding to zero bits;
do not generalize that observation to every later aggregate/member store. Padding
after stores can take unspecified values. Compare fields, not raw struct bytes,
to establish semantic equality.

For a pointer member, a null pointer value is not a universal all-zero object
representation. Aggregate `{0}` initialization and assigning NULL express value
semantics; blindly clearing bytes with a memory routine does not establish portable
null-pointer initialization. Inspect representation only as character bytes and
under an explicitly named target; do not turn a native observation into an ISO rule.

A block compound literal lives through its enclosing block, including an immediate
function call in that block. A file-scope compound literal has static storage
duration. A non-const literal remains modifiable, and another pointer to it sees
the modification. A function returning a pointer into a local literal leaves an
expired owner. Wrapping that pointer in a structure returned by value copies the
pointer, not its pointee or lifetime. The pointer value becomes indeterminate when
the owner ends. Classify the escape without invoking the defective function.
Repair by constructing in caller-owned storage while the local literal is alive.

## Const is an access contract

| Declaration | What this expression can change |
|---|---|
| `const int *p` | p can designate another object; `*p` cannot be assigned through p. |
| `int *const p` | p cannot be reassigned; the non-const object `*p` can be changed. |
| `const int *const p` | Neither p nor `*p` can be assigned through this declaration. |

These declarations do not establish unique ownership, immutable backing storage
or synchronization. A mutable owner alias can change a non-const object observed
through a const view. Conversely, casting away const and modifying an object
actually defined const is undefined. `int **` cannot be implicitly converted to
`const int **`: reason about the pointer object that a callee could overwrite,
not just the eventual int access. Preserve the negative translation and provide
a compatible positive interface instead of adding a cast to silence the diagnosis.

`fv_view` returns `{owner->values,4}` or `{NULL,0}`. It borrows the owner and never
extends its lifetime. It is a **live view**: reset/update through the owner changes
subsequent observations. A detached snapshot uses a distinct caller array and
`fv_copy`. A view does not become a snapshot merely because its pointer is const.

The separate opaque `FvOwner` handle denotes one static demonstration owner;
`fv_owner_view` exposes a live const view, while `fv_owner_mutable` returns a
mutable alias to the same four ints. NULL gives empty/NULL results. The handle's
layout is hidden from consumers, but this explicitly granted mutable capability
precludes a global immutability claim. Do not cast a fabricated pointer into an
owner. The static owner lives to program termination; caller-owned records have
the lifetimes of their actual caller objects.

## Copy and changed overlap assumption

`fv_copy(destination,source)` validates nonzero source length/non-NULL source and
nonzero capacity/non-NULL destination before checking capacity. INVALID and FULL
leave every destination value unchanged. Empty NULL source and zero-capacity NULL
destination are valid. Success copies source.length ints and leaves the destination
tail unchanged. Caller-provided pointer extents must be true, source values
initialized, storage live/aligned and accessed ranges disjoint. The restricted
local pointers express that aliasing promise; the callee cannot infer arbitrary
object extents or provenance from the lengths. Do not execute overlapping calls
to this restricted operation as expected-output tests.

The changed interface `fv_move(base,extent,destination,source,count)` allows
overlap **within one caller array**. It first rejects nonzero extent with NULL
base as INVALID; then rejects either index>extent or either remaining extent<count
as RANGE, without writes. Derive these checks with subtraction after index
admission, avoiding addition wraparound. NULL/zero extent and count0 is valid;
one-past positions at zero count are valid and never dereferenced. Success gives
the same values as if the source slice had first been saved independently.

Derive forward and backward copying from which writes could destroy a later read.
Do not order or subtract arbitrary unrelated pointers; the shared base and integer
offset contract makes this algorithm's comparison meaningful. Identical positions
and both overlap directions must work. Both copy operations use O(count) time and
O(1) extra storage; the host oracle's saved snapshot is not a component dependency.

## Volatile model and enforcement boundary

`fv_device_twice` accepts a valid pointer to a volatile unsigned RAM object. It
performs two accesses in separate full expressions and returns their XOR. Stable
ordinary RAM gives zero. This is a teaching device model, not an invented hardware
register map. The compiler's actual optimized x86-64 and Thumb code is inspected
for two loads; a cached-read specimen must be distinguished by its one load even
though a stable-RAM output test alone cannot distinguish them.

What constitutes volatile access is implementation-defined. Neither volatile nor
restrict provides an interthread happens-before edge, atomicity, cache maintenance,
bus barriers or a board-specific access-width guarantee. Do not execute a data race
to infer otherwise. A real MMIO extension would require the selected hardware's
authoritative register/access protocol and target observation; this package claims
neither. Here the caller owns the object and no concurrent writer is admitted.

## Deliverables and acceptance

Produce source plus the exact criterion-indexed reasoning dossier: initialization
versus assignment; pointer-value versus byte representation; both compound-literal
durations; every escaping alias; qualification diagnostics; live/snapshot and opaque
owner behavior; restrict preconditions; overlap proof; volatile and synchronization
limits. Include independent counterexamples and unchanged-output tests at zero,
short, exact, excess and extreme capacities/indices. Never use an expected value
for undefined behavior. Keep diagnostic specimens separate from executing tests.

Inspect actual object undefined symbols and generated instructions. For the target,
retain compiler identity, every explicit object, linker command/map, final undefined
symbols, disassembly and stack-usage records. Explain every compiler-emitted helper
and where its original implementation comes from. Do not silently add libc,
libgcc/compiler-rt/libatomic or a HAL. Target driver assertions are a smaller scope
than the full hosted oracle. The IRQ-masked, nonrecursive driver has a conservative
static stack calculation; it is not a physical high-water measurement.

Score each retained criterion separately: 1 is reproduced output; 2 additionally
requires a correct ownership/semantic argument and discriminating case; 3 requires
independent changed-condition transfer with revised evidence limits. The six
delayed tasks are administered from the separate assessor packet after at least
seven days. Hints: identify the storage owner before its views; distinguish pointer
qualification levels; save the original source slice mentally before deriving
copy direction; inspect access count when an ordinary value oracle is vacuous.

Named actual source passages: WG14 **N1570 C11 draft**, 5.1.2.4p25 data races;
6.2.4p2–6 lifetimes;6.2.6.1p6 padding after stores;6.3.2.3p2–4 qualification/null
pointer conversion;6.5.2.5p5–10 compound literals;6.5.16.1p1,6 qualification
assignment;6.7.3p6–8 qualifiers;6.7.3.1p1–8 restrict;6.7.9p10–21 initialization.
The source register identifies actual bytes and inspection limits. It is not the
published C17 text. C17 remains the implementation baseline. Finite author tests
do not establish external review, learner performance or physical validation.
