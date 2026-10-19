# Withheld technical analysis: PR-T36-08

The exact six original criteria retain their identities and V1 tasks. Use the
learner's completed producers and changed explanations, not this reference as
proof of independent assessment. All tests here are author controls.

## T36.02.a

Three declared Record elements are24 payload bytes; pointer storage, cached
snapshots,32-node ID tree and control metadata are additional. The selected
native owner/bank sizes are emitted, not hardcoded as ISO values. Generation1
becomes2 on insert,3 on remove; a further insert at limit3 rejects before a new
lease. An old token cannot revive. This consumes future update capacity; a
scheme that wraps needs another quiescence/identity premise. Owner address reuse
is explicitly outside the token lifetime contract. Require that distinction.

## T36.02.b

Ring positions after the seven steps have heads0,0,1,1,1,1,2 and counts
1,2,1,2,3,3,2. Slot queue allocates0,1,then reuses0,then2. Final head0/tail2
names values33/44. Retired node1 still contains22 but is not live. Physical
storage is not synonymous with logical membership. Full push preserves the
ring's caller-written count output77; do not invent a zero-count convention.
The free-slot scan total7 is derived from actual current source and replayed
states, not measured cache traffic. Growth of the wrapper uses two banks and
generation admission; queues themselves retain their original fixed policy.

## T36.02.c

Old bank objects stay physically alive in the preallocated pool, but the API
token is logically stale after committed growth. Read-only access does not
prevent the owner changing validity. On failed acquisition the retained bank,
owner bytes and view remain valid. `aw_read` first checks live owner control and
generation/index; all required borrowed payloads must still be physically live.
The cache comparison is not a dangling-pointer detector. Destroy clears pointer
slots and its retained contexts; it never owns/frees caller Record objects.

## T36.10.a

The actual CDTree proves unique-ID membership. Its comparator is integer ID,
not rank; do not claim the wrapper changed that producer's ordering. A projected
actual Record array feeds binary lower bound; the assembly uses rank then ID
and relation branches instead of overflowing subtraction. The x86 direction
test destroys flags, so rank comparison must be repeated before the signed
branch. This is a useful independent diagnostic prompt, not a license to teach
wrong code. Thumb saves r4–r7/LR and is a leaf. Public caller boundary remains
eight-byte aligned; the internal20-byte frame remains word aligned.

Callbacks and finite dispatch are evaluated on the same four fixture forms.
Finite dispatch cannot admit an unknown enum; callbacks can change implementation
but still must implement the declared finite ordering. Inspect generated
comparison and indirect-call code, including callback implementation. Never
compare a dispatch stub's size with an entire inlined operation as equivalent
work. The callback policy checker contributes O(n²) comparisons; clones and
bank copies dominate this deliberately small wrapper's cost.

## T36.10.b

Changing the caller's rank after value insertion leaves copied rank2. The same
live mutation after borrowed insertion is diagnosed MUTATED and output remains
unchanged. Restore the object to continue, or destroy without payload access.
The owner records epoch and direction; a direct change while populated is
COMPARE. Empty context adoption increments generation, then descending rank
produces7:2,9:2,4:1 with ascending ID ties. Function pointer type checks do not
constrain effects. An inconsistent self-comparison and attempted reentry are
separate callback failures. No finite pair sample establishes universal purity.

## T36.10.c

Duplicate lookup precedes capacity rejection and causes no acquisition. A new
key at capacity is FULL. Before any new bank is committed, stage CDTree and
the ordered payload projection. On failed acquire no owner/index/count/revision
or old-bank byte changes. Five update boundaries × two representations each
fail once and then retry, giving six attempts/five releases per full path.
Nine separate pair-callback failures during third insertion precede acquisition.
Destructor cleanup remains possible after a context/live-borrow diagnostic.

The early-index-publication mutant must fail the backing snapshot assertion,
not a later tree operation. The copy mutant retains the external pointer and
must fail the copied-rank assertion after live mutation. The generation mutant
must fail the old-view assertion after a successful move. The precedence mutant
must fail DUPLICATE at full capacity. Target tests select each relevant case
so unrelated earlier effects cannot masquerade as intended mutation evidence.

`oracle.py` independently reconstructs7,560 mathematical lower-bound packets,
all seven physical queue states and exact fixture/backing observations. Its
ordering rule does not call the target comparator or assembly. These finite
relations, six native profiles and two emulated targets support this bounded
reference only. They do not establish all callback behaviors, target timing,
external review, completed delayed learning or absolute mastery.
