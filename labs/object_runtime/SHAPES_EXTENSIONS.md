# Prototype shapes, weak cache guards and mutating accessors

This required later integration is X-T74-013. Complete the original X-T74-003
and X-T74-004 kernels, then X-T74-008's typed pool, before returning to their
prototype/reuse transfers. This staging avoids a reverse prerequisite edge.
`object_runtime_shapes.h` supplies the representation and public API;
`src/object_runtime_shapes.c` is the learner scaffold. `tests/shapes_ext.c` and
`fixtures/object_runtime/shapes_ext.json` supply concrete inputs and controls.
The dependency is the actual ORXHeap implementation, with its generation and
root semantics, not a second simulated object-identity array.

Use C17 and the same hosted compiler profiles. No JavaScript engine, property
dictionary, collector or object-model library may implement the studied work.
The original V8 text is a bounded comparison of descriptor/handler guards,
not a promise that this teaching object model implements JavaScript semantics.
The original ISO/object lifetime and shared exercise contracts still apply.

## World, shape and object identity

An initialized stationary `struct orx_world` owns an ORXHeap, eight object metadata rows
and16 immutable descriptor slots. A world object uses the same slot/generation
handle as its actual heap entry; its copied byte payload is empty, and strong
heap edge0 is its prototype. Metadata generation must match the heap entry.
New objects have one external root and the permanent empty shape0/generation1.
Only `orx_world_new` creates world objects. Call existing `orx_retain/release` on
the world's heap for explicit external roots, but do not bypass the world API
with raw pool creation, edge mutation or collection. `orx_world_collect` calls
the actual collector, drops dead objects' descriptor references and clears
their metadata. Prototype edges therefore retain parents transitively.

A descriptor contains an ordered list of up to four nonzero byte-sized keys
and attribute bits. Identity includes slot and generation. Live descriptors
with the same ordered keys/attributes are shared. Each live object owns one
descriptor reference; a cache owns none. A nonempty descriptor with no object
references becomes free and increments its generation. Generations1..3 are
usable;4 is retired forever. Shape0 remains live with an exact object-reference
count. Reuse of a numeric slot does not revive its old descriptor identity.

## Definition, deletion and prototypes

`orx_define` adds or replaces an own property. Attribute bit1 is READONLY and
bit2 is ACCESSOR; other bits are invalid. An accessor requires a nonnull typed
getter function. Its context is borrowed through every possible call; it must
remain live even if the callback changes its defining property. A data property
uses its int32 value. This small model treats READONLY as forbidding both
replacement and deletion, including attribute changes. It is not JavaScript's
independent writable/configurable policy.

Existing writable data/getter changes that preserve keys/attributes keep the
shape. New keys or changed attributes acquire the replacement descriptor before
publishing fields and dropping the old descriptor. Deletion shifts remaining
fields and ordered descriptors together; absent own keys return MISS. Inherited
properties are never deleted by deleting an own property. Full property storage,
descriptor exhaustion and readonly rejection preserve world state. A full
four-property receiver still permits an existing writable value update.

`orx_prototype` admits null `{0,0}` or another live object. Reject any chain that
would reach the receiver, before changing the actual strong edge. There are at
most eight live objects, so an admitted chain is bounded and acyclic. A lookup
searches the receiver then successive prototypes, stopping at the first own
property. Adding an own key shadows an inherited key; deleting that own key
reveals the inherited value again.

The world has a structural epoch starting at1. New/changed layouts, deletion,
changed prototype edges and world collection advance it. A structural operation
at255 fails unchanged. Updating only the current field value/getter context
does not advance it because cache hits read the current field. Object creation
does not change any existing prototype chain or shape. This conservative global
epoch deliberately invalidates more caches than a per-chain validity scheme.
Derive that cost and a safe changed-condition refinement; never wrap an epoch
while a weak cache can retain its old value.

## Cache admission and accessor effects

Zero-initialize `struct orx_property_cache`. It weakly records world identity, receiver
and holder handles, both descriptor identities, key, offset and epoch. A hit
requires all guards, live receiver/holder generations and matching current
shapes. It reads the current field; it never owns or caches the returned data
value. A different world resets cache counters. Otherwise hits/misses are exact
uint32 counters and the corresponding operation fails before wrap. Missing
lookups may update diagnostic cache state/counters but preserve payload output.

Before invoking an accessor, copy the typed function and context and retain
temporary roots for receiver and, if different, holder. Those roots prevent
collection during an allocating/mutating callback. The callback may define,
delete, change prototypes, create objects or collect through the world API.
Recursive property lookup while a getter runs returns STATE, preventing hidden
recursive dispatch. The callback must not consume roots it does not own or
invalidate its borrowed code/context. Its own external side effects are not
rolled back by a failed return status.

Stage the returned scalar, clear the reentry flag, then release the temporary
roots on every callback exit. Do not reread the old field after callback return.
If the epoch changed, invalidate the cache rather than installing the old
resolution. A successful mutating callback still returns its staged result.
A failed mutating callback preserves output while its completed mutations remain
visible. The next lookup resolves the changed structure. No C type rule enforces
the callback's ownership or reentry contract; diagnostics, runtime guards and
programmer discipline cover different obligations.

## Deliverables, independent checks and delayed assessment

Implement the world, descriptor acquisition/release, prototype admission and
guarded lookup using the supplied types. Derive reference-count conservation,
generation retirement, chain acyclicity and the structural-epoch invariant.
For each failure path, list private state, irreversible callback effects and
published state. Measure descriptor metadata, cache footprint, generic versus
retained-cache lookup under equivalent result/lifetime contracts, branch/code
cost and compile/stack evidence using the13-dimension modern cost contract.

The independent Python model compares3,600 lookups across ordered layouts,
deletion, readonly policy and own-property shadowing. CTests additionally cover
descriptor reuse/exhaustion, prototype retention, world switching, counters,
stale objects and accessors which mutate then succeed or fail. Four compiled
mutants omit readonly/cycle admission, omit the epoch guard or use the wrong
post-callback field value. None relies on a crash as its success criterion.
Run GCC/Clang debug/release/sanitizers and record source-bound results; a finite
model is not all-input proof or independent learner acceptance.

The delayed X-T74-013-DELAYED variant returns failure after installing an own
property that shadows the accessor. Require an unchanged immediate output,
visible completed mutation, released temporary roots and a fresh successful
lookup of the new value. Explain why restoring the old receiver would undo a
committed callback effect and why retaining the old cache would be stale.

Hints: derive the three identities before writing a cache; use an immutable
descriptor candidate to locate the publication boundary; then draw a callback
trace in which the old offset becomes meaningless. Level1 nominal property
lookup is insufficient. Level2 derives and checks the stated invariant and
counterexamples; level3 independently changes cache invalidation granularity or
attribute policy while retaining lifetime and failure arguments. Separate
assessor analysis contains the technical solution. Host execution, external
review, delayed learner retention and physical target validation are distinct.
