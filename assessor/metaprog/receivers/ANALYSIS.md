# Assessor: T68.05 generic receivers and volatile command views

R1: the controlling expression is syntactically present in `_Generic` but is
unevaluated. Only the selected function call evaluates the supplied pointer.
All associations contain valid function names, so invalid branch-specific
member expressions are avoided. The no-default table admits exactly the four
pointee qualifications. A synthetic category marker exposes wrong const policy
without invoking a write through const. The duplicate-evaluation mutant uses
a sequenced comma expression and a defined counter, not an undefined test.

R2: a const pointer is an immutable pointer object, not a pointer to const.
The actual N2243 DR481 C17-marked corrigendum specifies conversions as if
performed for selection. Both compiler probes choose the unqualified pointer
association and decay the array. Compatible typedef aliases cannot appear as
distinct associations; function and variably modified types are not admitted
association object types. The explicit qualifier-discard cast compiles, but
that is not permission to access a volatile-defined object through an ordinary
lvalue. Keep that specimen translation-only. No broad language trait or
ownership-system equivalence follows from typed function selection.

R3: each read checks word/output pointers, reads once into a scalar, then
publishes. Observe checks all admission inputs before read/callback/read; a
const view of the mutable synthetic volatile word may see17 then23 through a
legal mutable alias. Callback lifetime/nonreentry premises are explicit.
The independent oracle predicts categories, before/after values, exact stored
command and null/mask status without inspecting C layout. The model is finite;
it is neither a weak-memory checker nor a peripheral simulator.

R4: RW update is valid only for the declared externally serialized ordinary
register. Its mask test precedes the single read and store; set wins overlap.
The W1C wrapper sends only flags, with no read. For current0xa2 and flags4 an
OR-based command0xa6 is wrong even though it type-checks and executes safely
on the original RAM teaching target. A real device's clearing behavior is a
separate premise, not an observation of this campaign. Neither wrapper is
atomic or inserts an ISA barrier. Inspect GCC's actual read_v/read_cv/update/
ack assembly and the Clang host/M4 volatile IR counts. The M4 link has no
runtime/startup contract and was not executed.

Held delayed prompt: expose a mutable volatile word through a const volatile
view, change it in the explicit between callback, then refuse an outside-mask
write before acknowledging one permitted W1C bit. Derive exact counter, old/new
output and command, then identify the missing contract if the peripheral is
changed to read-clear or asynchronously updated. A learner who removes const,
casts away volatile or substitutes a fence without a device contract has not
repaired the abstraction. Existing reference support tests these author-side
mechanisms; the actual unaided delayed learner evidence remains pending.

Mutants1..6 duplicate receiver evaluation, substitute category3 for cv,
reuse the first sample after callback, add an improper read/OR to ack,
ignore clear bits, and store a refused value. Mutants7/8 invoke the retained
MOD9/10 const-policy/extra-evaluation defects. Ordinary semantic assertion
rejection is required; translation and sanitizer failure do not qualify.
