# Iterative compiler assessor analysis

Keep this file, compiler.c and oracle.py out of learner exports. The canonical
original six criteria and six delayed variants remain the assessment identities.

## T46.05.a

The AST scanner owns copied token spellings and indexes into caller-owned fixed
arrays. No NUL sentinel terminates the input scan. Collect complete function
signatures before lowering bodies, so a forward reference is an arity lookup,
not an unresolved dynamic symbol. Install let only after its initializer emits.
An innermost-backward environment search implements shadowing. The original
Nystrom local-variable discussion uses a different initializer policy; do not
copy that language's semantics into this prefix language.

Sequential scopes reuse slots after NIL/STORE clears the outgoing binding.
Escaped pair fields contain copied tagged values, not slot addresses. A future
closure/address operation needs a new representation and lifetime proof.

## T46.05.b

The two x bindings occupy distinct live slots. Loads from the inner scope yield40
or the newly assigned41; the final load after releasing it still yields2. Result42
or43 follows. A copied source-level environment and independently decoded stack
program provide different checks. The SHADOW and INITIALIZER mutants expose
outer-first lookup and premature visibility; neither repair is a test suppression.

## T46.05.c

Explicit parse parent/tail arrays build an AST; explicit emitter continuation
frames replace recursive calls. Both are bounded separately from runtime stack
and fuel. Work/report are scratch, output publication occurs only after both
actual verifiers pass. The code-capacity boundary is visible: N Boolean atoms in
a block require N pushes, N−1 pops and one return, totaling2N bytes.
The canary mutant must fail after a real admission/compilation refusal.

## T46.06.a

JFALSE consumes a Boolean in CVM1, unlike the inspected Lox instruction. A source
expression leaves one value on normal completion. If arms join at h+1; a loop
backedge returns to h, its false exit adds NIL, and break exits join at h+1.
Short-circuit forms must test the evaluated RHS too; otherwise integer truthiness
can slip through a lowering that happens to choose the right branch. The independent
decoder checks all instruction boundaries before reachable data-flow analysis.

## T46.06.b

Break's scratch slot retains its value while temporaries above the loop entry
height are popped and exited locals cleared. Reload the result and clear the
scratch before jumping past the loop's normal NIL. A break in a nested loop
condition targets the nearest enclosing active body. The reserved local capacity
is explicit. Quoted host effects are original counters; no claim of rollback
after a host callback is made. Both logical operands use Boolean tags.

## T46.06.c

The independent bytecode machine charges once before each dispatch, including
RET, branches, POP and root-cleanup stores. Finite source termination and sufficient
dispatch fuel are different properties. Literal folding changes the selected
program's four-dispatch sequence to two; it is equivalent only under stated
adequate-resource assumptions. Overflowing literal arithmetic stays a runtime
range error. No optimization may silently wrap or erase side effects.

## Evidence and residual limits

The Python oracle parses source into a separate tree and uses a separate interpreter,
then independently decodes code and simulates dispatches. Its selected corpus is
finite and has explicit supported operations; it is not a general language proof.
Native compiler call-graph and local frame audits show the authored own-function
graph has no recursion; external libc implementation frames are a separate cost.
The original C compiler/VM and new Vv verifier are linked unchanged. No independent
learner, external assessor or physical target observation is supplied here.
