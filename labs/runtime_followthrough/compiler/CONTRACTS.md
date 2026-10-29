# Bounded iterative compiler and control-flow successor

Version1; required PR-T46-05/T46-05.a–c and PR-T46-06/T46.06.a–c ordinary and
original `-DELAYED` tasks. Families A/B/C/D/F/G/H/I/J apply: derive lexical and
control semantics, construct the compiler, exhaust bounds, diagnose mutations,
verify independently, preserve failures, measure resources and transfer.
This adds concrete implementations to existing identities and hours; it does
not rewrite historical acceptance or administer a learner's delayed assessment.

## Prerequisites, environment and dependencies

Bring the original CVM1 program/value/codec/VM artifacts and the actual
`struct vv_work`/`vv_verify` definite-initialization verifier from
`labs/runtime_followthrough/verifier`. Read `labs/runtime/SPEC.md` and the
original PR-T46-05/06 contracts. The reference campaign links their actual
implementations; identical names or an always-successful verifier are insufficient.
The retained recursive `cvm_compile` is a comparison provider, never the new
compiler's implementation. Learner exports contain unfinished sources.

The compiler is hosted profile A, ISO C17, eight-bit bytes and provided fixed-width
integer types. It may use the declared C byte/string primitives; no parser,
compiler, AST or scope library may implement the taught mechanism. `struct vc_work` is
caller-owned and contains no allocation. Host Python tools are verification tools.
The VM's separate strict target evidence does not turn this hosted compiler into
a freestanding target. Record GCC/Clang/libc, optimization, NDEBUG and sanitizer
profiles; source inspections of drafts are not exact published C17/C23 access.

## Representation, operations and problem

Implement `vc_compile` in `learner.c` from the declared primitive interfaces.
Source is an explicit live span of at most4096 bytes, not a NUL-terminated input.
Source, options, workspace, report and output are live, correctly typed, aligned
and disjoint; calls are serialized. The source stays immutable for the call.
No callback occurs during compilation. ISO C does not enforce these preconditions.

Tokens are ASCII identifiers/operators, decimal signed-int32 literals and
parentheses. Only space/tab/CR/LF are whitespace. Reject embedded NUL, non-ASCII,
names longer than31 bytes, malformed tokens/parentheses, out-of-range literals,
duplicate function/parameter names and unbound identifiers. Functions have form
`(fn name (parameters...) expression)`. Collect all signatures before lowering;
first function must be `main` with zero parameters. At most8 functions,8 named
locals/parameters per live scope,1024 code bytes. Names follow the original
letter/underscore initial and letter/digit/underscore continuation grammar;
`true`, `false`, `nil` cannot be declarations.

The retained forms are integer/Boolean/NIL, `+ - * / == <`, `pair first second`,
`let`, `set`, `if`, `while`, binary `do`, `call`, `host` and zero-argument `gc`.
`let name initializer body` resolves its initializer in the outer environment,
then introduces a new innermost binding for its body. Clear and release the
local slot after the body while preserving its result. `set` resolves the
innermost live binding, assigns and returns the assigned value. Function
parameters are copied values; forward call arity is checked during compilation.

Changed forms are `(block expression...)`, `(and left right)`, `(or left right)`
and `(break value)`. A nonempty block sequences expressions, discarding all but
the last result. Each nested `let` remains lexical; sequential completed scopes
reuse slots. No address-of, closure or reference-to-local operation exists in
this language. A pair contains copied `struct cvm_value`s, not addresses into reused
local storage. This argument depends on the actual VM representation and is
not a general guarantee about all C implementations or future closure features.

`if`/`while` and both evaluated logical operands require Boolean tags; integer
truthiness is a TYPE error. Logical RHS is evaluated only when necessary and
logical results are Boolean. A normal while exit returns NIL and discards each
body result. `break value` exits the nearest enclosing **loop body**, returns
that value as the loop expression and discards operand temporaries introduced
inside that loop. A break inside a nested loop's condition still belongs to
an enclosing active loop body. Break outside such a body is a syntax refusal.
It clears locals exited by the jump, preserving enclosing locals and copied
escaped values. A program containing break reserves slot7 for a transient
value, reducing its named-local/parameter limit to7; the scratch slot is cleared
after loading the result. This explicit changed capacity is not hidden overhead.

`struct vc_options={64,0}` is the retained depth/no-folding profile. Expression depth
counts every expression including atoms, independently of grammar parentheses.
The iterative variant permits any positive limit up to256. Parser parent/tail
arrays have260 levels; the explicit AST has4096 nodes; emitter frames have256
entries. Derive why a4096-byte source within the expression limit cannot silently
exceed these grammar capacities. The C implementation must not recurse in
parsing, signature collection or emission. Workspace/report may change on
failure; the complete output program remains byte-for-byte unchanged. Null
controls, invalid options or any syntax/resource refusal return `CVM_SYNTAX`.

Lower to ordinary CVM1 absolute little-endian branch operands. Every target is
an instruction boundary within its function. Every reachable join has compatible
operand height; locals must be definitely initialized on every incoming path.
Even unreachable instruction bytes must have valid widths and operands. Validate
the staged program with actual `vv_verify(...,0,1,...)` and `cvm_verify` before
one publication. Output verification is a distinct safeguard, not a substitute
for deriving the compiler's invariants.

Folding mode1 may replace a binary integer-literal `+`, `-` or `*` with one INT
only when its mathematical result fits int32. It preserves values/type/range
behavior under sufficient resource budgets, but changes byte size and dispatch
fuel. Do not fold overflow into a wrapped value or claim budget compatibility.
Never remove observable host effects, eager error behavior or root cleanup.
Mode0 preserves the retained lowering on its supported syntax.

## Concrete inputs and necessary reasoning

Predict outer `x=2` plus inner `x=40` as42; assign41 inside that inner scope and
derive43. An inner initializer `(+ x 1)` observes the outer2. Construct a pair
with11 in a scoped local, reuse that slot, and show the escaped pair still holds11.
Twenty sequential lets need one local slot. Explain why adding a closure or
address-of primitive would invalidate the copied-value argument.

Compile63 nested `first` forms over NIL at depth64, reject64 under that profile,
accept255 under depth256 and reject256. These are compile/depth cases, not claims
that applying first to NIL succeeds at runtime. A block of512 Boolean atoms
produces exactly1024 bytes;513 exhausts code capacity. Admit8 parameters without
break and refuse9. Use4096/4097-byte spans and an existing canary program for
every refusal. Separate source length, parse workspace, emitter depth, guest
operand/frame limits and guest dispatch fuel.

Trace nested branches, patch points and operand heights before running. Mutate a
target to an immediate byte and require verifier refusal. Only one selected host
branch increments the synthetic counter. `and false (/ 1 0)` skips the division;
`and true 3` reaches a TYPE error. For
`(+ 100 (while true (+ 10 (let x 4 (break x)))))`, account for discarded10,
cleared inner local and retained100 to derive104.

The counter loop from0 through4 has56 dispatches in the retained lowering.
Budgets55/56/57 respectively fail unchanged, succeed with0 and succeed with1
remaining. A balanced infinite loop exhausts any admitted finite fuel without
publishing a result. `(+ 2 3)` needs4 dispatches without folding and2 with it.
These are instruction counts under this VM, not CPU instructions or time bounds.

## Deliverables and verification

D1: exact grammar, lexical binding table, AST/continuation representation,
scope/operand invariants and failure publication argument. D2: original C
implementation, code listings/maps and explicit producer dependencies. D3:
predicted source results plus independently decoded bytecode widths, targets,
stack/init joins, dispatch counts and source-level evaluation. D4: all public
selectors0–5, boundaries and a fresh wrong-explanation counterexample. D5:
source/compiler/libc/commands/raw outputs, sanitizer controls, own-function call
graph and stack/workspace accounting; identify external library calls omitted
from any local stack bound. D6: original criterion-specific delayed task after
at least seven days, without consulting assessor solutions.

Pass requires each exact original criterion and variant, correct state/results
and preserved canaries, actual imported verifier/VM, rejected compiling learner
stub and mechanism-specific mutants. Test the failed proof, not merely a changed
return code. Compare the no-folding ordinary subset with the retained compiler,
then use a separately written tree evaluator and bytecode interpreter to avoid
counting two copies of the same lowering as independence. Deep parser bounds are
checked in C; a recursive host oracle is not permission for a recursive compiler.

The original objective-specific rubric applies: level0 wrong contract or result;
level1 reproduces nominal output; level2 D1–D5 with sound invariants and independent
falsification; level3 successful D6 with changed assumptions. Author test execution
does not administer level3, prove universal equivalence or establish absolute
mastery. Preserve all original minimum-seven-day IDs and tasks.

## Hints and variants

First write expression stack equations and scope entry/exit effects. Then separate
signature collection from resolution. Next represent each unfinished recursive
emission action as a bounded continuation frame. For break, identify the exact
loop operand base and local frontier before emitting a jump. Only after deriving
those joins consult the public verifier diagnostic. Assessor implementation,
independent oracles and technical analysis are published separately.

Original changed tasks are mandatory: lexical blocks/slot reuse; inner assignment;
iterative parsing; short-circuit operators; break with value; reduced-opcode fuel
compatibility. Additional transfer: compare a closure-capturing language with this
value-only grammar, change branch encoding to relative offsets, or separate
compile-time resource errors while preserving output. These alter the argument,
not just constants. Source-level semantic equivalence must state fuel/heap/host
budget assumptions and error/effect ordering explicitly.

## Security and validation limits

Inputs target original local teaching compiler/VM only, with synthetic integers
and bounded original host counters. Adversary controls source spans and budgets,
not C pointers or private owner state. Harmless failure witnesses are wrong
binding/value, accidental host counter increments, corrupted canary or excess
work. Repair bounds/resolution/joins and retain regression/mutation evidence.
A parser error or sanitizer finding is not arbitrary code execution. No external
service, board, credential, host security change or production target is involved.

Authored status and actual observations are separate. Current source-bound author
execution belongs to `research/v5/expansion/legacy_vm_compiler/validation.json`.
External review, independent learner/delayed assessment and physical observations
remain separate; no such result follows from native author checks.
