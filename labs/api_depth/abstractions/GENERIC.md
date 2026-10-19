# T19 generic storage and tagged ownership

PR-T19-02 T19.04.a–c and T19.05.a–c retain their original AX identifiers,
rubrics and seven-day changed tasks. Families A/B/C/D/F/G/H/I/J apply as below.
Prerequisites: T18 bounds/lifetime/failure contracts, T06 macro/type reasoning,
and actual typed object storage. Use C17 hosted GCC/Clang O0/O2/ASan+UBSan;
edition probes explicitly add C11/C23 and code-size probes add LTO. Host Python
oracles are not target dependencies. The selected parser interval requires
`int` to represent[-32768,32767]; the implementation must reject an unsupported
representation at translation, not assume all C17 integer representations do.

## Generic syntax does not carry an ownership proof

Implement a four-int concrete buffer, a macro-generated typed buffer and an
erased core with exact copy/destruction hooks. Count0..4 is invariant; a fifth
push returns full without mutation. Evaluate each runtime argument once. Use a
`_Generic` selection of function designators followed by one call, keeping the
controlling expression unevaluated. The API deliberately admits `int`, not all
implicitly convertible arithmetic types. A direct typed C function can accept a
converting double argument; its signature alone is not that stronger diagnostic.

Compile three negative fixtures: wrong generic argument type, an invalid member
access in an unselected generic association, and an omitted enum case after
adding a variant. They must be diagnosed; never invoke diagnostic-invalid code.
Not evaluated does not mean not semantically checked. Compare array arguments,
const pointee pointers, top-level const pointer objects and address-of-array
types in C11/C17/C23 under both actual compilers. Record observed selections and
warnings. The inspected WG14 DR481 resolution supplies the C17 correction for
controlling-expression conversions; N1570 alone predates it. Pointee qualification
and array extent through pointer-to-array remain distinct from top-level qualifier
removal. Exact published-edition access is still recorded separately.

## Erased storage needs real typed objects

`AsErased` owns a live prefix of a caller-owned array of capacity0..4. Its descriptor
contains byte size, alignment, exact copy/drop functions and live callback context.
Caller supplies correctly typed destination, staging and source arrays, mutually
disjoint. Do not substitute an aligned byte array for a declared array of owning
objects. Size/alignment metadata is an additional contract, not a runtime proof
that an arbitrary address has that type. Reject zero/overflowing size, invalid
count/capacity, absent hooks, inadequate stage capacity and incompatible supplied
alignment before calling hooks. Address validity, compatible real element types
and nonoverlap remain caller premises. There is no pointer-sandbox guarantee.

Copy hook success constructs exactly one staged owner; failure retains no staged
ownership and leaves its source unchanged. Drop consumes exactly one live element,
is infallible and does not reenter the component. Elements must additionally be
relocatable by representation copy: no internal self-pointers needing repair and
no address-registration invariant. Commit transfers staged ownership into the
unused destination suffix; staging then contains no live ownership obligations.
Its old pointer bits must not be separately dropped. This relocation rule is a
real restriction; size, alignment and a destructor alone would not justify it.

Append copies into typed staging first. If copy2 fails, destroy only staged copy1,
leave destination length and existing owners unchanged, and return the failure.
Only after all copies succeed may the destination prefix grow. Count zero performs
no copy/drop and succeeds under a valid destination descriptor. Clear destroys the
live prefix in reverse order. Complexity is O(n) hook calls plus O(n*element_size)
relocation bytes; hook work is additional. Temporary bytes are n*element_size,
bounded by the four-element supplied staging array. Owned strings additionally
require separately counted allocations. Compare these costs with typed pushes.

Use actual resource-owning `AsValue` elements. Arrange allocation failure on the
second staged clone. The existing destination integer7 must remain, source owners
must remain, and one staged allocation must be freed. On success, mutate a source
owned string and show the destination deep copy is independent. A shallow pointer
copy would duplicate release obligations. For move-only insertion, fail before
transfer and preserve both operands; on success transfer once, replace the source
with inert integer0 and release only from the destination. Destination must not
already own a resource; constructors/clone/move cannot silently overwrite an owner.

## Alternatives and conversion

`AsValue` has explicit tags INT, TEXT, ERROR, OWNED. Validate the tag before reading
its selected union member. INT carries an int; TEXT borrows0..8 bytes, including
embedded zero; empty null text is valid. ERROR has a negative code and explicit
offset. OWNED contains nonnull allocated data,0..8 length and its allocator. Even
empty owned text has an allocation to keep its release rule uniform. The allocator
context survives the last owned copy; its release consumes and cannot fail.

Construct int7, text{'a',0,'b'}, empty text and error-2. Clone preserves borrowed
text's lifetime obligation but allocates a distinct owned-text copy. Move transfers
the current alternative; drop clears it to inert integer0, releasing only OWNED.
Invalid external tags reject without payload inspection. A missing switch case is
a compiler-diagnostic issue; a malformed runtime tag is an admission issue. Generate
tag declarations and names from one list, then give a semantic counterexample:
complete generated metadata cannot establish that a numeric conversion checks its
range or that a clone preserves ownership.

Decimal input is length-delimited0..8 bytes. Accept an optional single '+' or '-'
and at least one ASCII digit; no whitespace, embedded zero or suffix. Return INT
within[-32768,32767], otherwise ERROR: -1 empty/sign-only at the first missing-digit
offset; -2 invalid byte at its index; -3 range overflow at the first digit whose
prefix exceeds the selected signed limit; -4 invalid extent/null-with-positive
length at offset0. Validate before arithmetic. For negative minimum, do not compute
an unrepresentable positive signed magnitude. Source text remains live throughout
conversion; result owns no reference to it.

The accumulating changed task accepts up to four separate fields and returns one
valid result per field without short-circuiting. Preserve field index and local
error offset: ["x1","32768","-","7"] produces errors at0,4,1 and successful7.
It does not pretend every later character of an already-invalid token has a
meaningful arithmetic value. Generate an independent grammar/range oracle using
unbounded integers, including the full decimal range[-32800,32800], all strings
of lengths0..4 over{'0','+','9','-','x'}, embedded zero and eight-byte extrema.
Retain exact corpus count and identity; compare each tag/value/error offset.

## Cost and assessment

Compile typed/generated/erased int forms at O0/O2 with and without actual LTO for
both compilers. Keep full binaries, section sizes and disassembly. All three
perform1000 rounds of values1..4, reject a fifth, and verify total10000. Add a
runtime heterogeneous INT/TEXT form with equivalent scalar contribution but a
different type/lifetime contract. Report persistent storage, descriptor bytes
already included within it, typed staging bytes, allocations and actual direct/
indirect call sites. Whole-executable call counts include startup/runtime code;
annotate component calls before attributing overhead. A linker pass or disappearance
of one indirect call does not guarantee all implementations devirtualize. This is
code/resource analysis; timed transport comparisons belong to T19.10.

Deliver the three forms, owning-element lifecycle code, staged transaction proof,
variant constructors/converter, independent oracle, negative fixtures, exact
build records and cost interpretation. Run the public tests with learner bodies;
intentional stubs must compile and fail semantic assertions. Selected leak/range
mutants must also compile then fail for the expected property. Sanitizers supplement
the ownership proof; a clean run is not a universal lifetime guarantee.

Score each of six leaves independently:0 incorrect;1 nominal output only;
2 correct ordinary implementation/reasoning plus falsification evidence;
3 independent changed-condition implementation with explicit enforcement/resource
limits. After at least seven days, assess array/qualification behavior, move-only
failure, runtime heterogeneity, owned variants, generated metadata limits and
accumulating errors with assessor solutions withheld. Hints: enumerate live owners;
stage before commit; select the function rather than invalid per-type expressions;
separate tag coverage from semantic correctness.

Source register: `research/v5/expansion/legacy_abstractions/resources.json` names
the actual original tasks, N1570 effective-type/generic/function clauses and DR481.
All current evidence is bounded author validation unless explicitly recorded
otherwise. Original synthetic local programs only; no external targets or
arbitrary-code-execution claim follows from these fault specimens.
