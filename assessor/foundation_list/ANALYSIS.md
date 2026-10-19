# PR-T01-03 technical assessment

Private solution and experiment driver: list.c, workflow.py, oracle.py. Public
stubs deliberately remain incomplete. Grade the original AX task and delayed ID
individually; source/body labels below are rubric anchors, not automatic credit.
Exact campaign evidence is bound by close_foundation_list.py. Reject unexplained
copied traces even when the numeric result is correct.

## t0104a

The core `.i` includes declarations from list.h and standard headers, `.s` names
ft_tokens, `readelf -h` identifies REL objects, and the linked ELF has INTERP.
The two application TUs are core and main. The compiler driver with -c and an
unknown suffix can exit0 without producing the expected object; explicit -x c
changes input-language selection and produces it. Merely renaming the output does
not change the input language. Accept a different explicit stage toolchain only
with the same actual boundaries and artifacts. Level3 requires this no-suffix
experiment plus an explanation of driver inference, not another normal build.

## t0104b

Under the inspected System V psABI integer a,b use the first two integer argument
register families, RDI/RSI, with the relevant32-bit parts EDI/ESI; integer result
uses EAX. The actual O0 bodies access local stack offsets with the chosen frame
pointer policy. O3 derives twice the sum from registers and can omit the distinct
local slot. Clang may use RDI/RSI address-register spellings inside LEA even though
the C arithmetic result uses EAX; a checker requiring only `%edi` is wrong.
Stack return address, saved RBP and dataflow are different things. Annotate the
actual object, not an idealized stack picture. Accept equivalent ABI-correct
code; reject “every local always lives on the stack” and “debug info forces it”.
The sketch must name process virtual-address ownership: caller/callee inhabit one
image and thread stack, and a call grants no isolation. Equal virtual addresses
in separate processes need not identify the same storage. Threads can share an
address space while using separate stacks. Reject “every function has a private
address space”; keep these Linux/process claims distinct from the C object model.

## t0104c

Syntax error fails compilation; linking main alone fails the ft_tokens reference;
an existing ELF with absent PT_INTERP fails process launch with ENOENT; a valid
program rejects malformed input with exit2. The wrapper process normally spawns
then execs; exec itself replaces an existing process image, not a new PID. The
shared object experiment reaches loader failure after a successful link and run:
DT_NEEDED records libft.so and the owned move makes it unavailable. Restoring the
same bytes restores execution. A crash is none of these four demonstrated causes.
Require actual commands and file evidence; permissions or host configuration
changes are unnecessary and outside this teaching protocol.

## t0105a

The supplied replay script executes each stage in a fresh environment, quotes
compiler/source/output arguments, and succeeds from actual directories containing
spaces. Passing an entire command as one string or unquoted expansion changes argv
boundaries. The criterion is replayability under these conditions; reproducible
source hashes alone do not establish byte-identical linked images on every host.

## t0105b

Two original scratch commits change only default mutant selection from final-only
to each-prefix validation. Clean worktrees yield normal sum6 in both, cancellation
success only in the defect and prefix rejection in the repair. The changed API
revision renames ft_tokens to ft_parse_tokens, then the repair is rebased onto it.
The old typed caller must fail translation against the new header; the updated
caller passes and the same CLI regression still rejects. Copy the old caller away
from its old quoted header before -I selection, or the negative test is vacuous.
Git success alone is level1; level3 requires explaining contract preservation
across the new source interface and demonstrating both caller outcomes.

## t0105c

The campaign retains actual compiler path/version/target, command vectors, working
directories, controlled locale, revision/source/input/tool/output hashes and raw
stdout/stderr/status. `revisions.json` maps commit and compiler to command indices.
The ordinary compiler change is GCC O0 to Clang O3 on both revisions. Prefix
representability and token grammar remain mathematical/C arguments; exact generated
code and diagnostics are empirical. Hashes identify bytes, not trustworthiness.
Reject a manifest describing a different source tree or only a -### command plan.

## t0106a

Each token stores a state, sign, bounded magnitude and sticky range flag. Test
digit against limit%10 when magnitude equals limit/10; once range fails never
evaluate another growing arithmetic expression. A later nondigit wins lexical
precedence. Negative magnitude32768 is converted to int32 before negation, which
fits this profile; do not negate an int16 minimum. A temporary values array and
local total postpone publication. The CLI copies pointer values into an actually
typed const-char-pointer array; casting char** to const char** is not the proof.
The stream carries unfinished token state between calls and flushes exactly once
at EOF. Whitespace-only is empty; lone sign and embeddedNUL are malformed. A fixed
token buffer wrongly rejects arbitrarily long leading zeros. Five mutants isolate
final-only check, missing EOF flush, premature publication, lost error count and
ignored nondigit. Do not infer correct stream behavior from argv tests alone.

## t0106b

The independent host oracle uses whole-string regex and arbitrary-precision
conversion, then direct sums of slices. This differs from C's incremental digit
automaton and checked-add recurrence. It enumerates133 grammar tuples of length0
through2,125 arithmetic triples and4 special lists:262 lists in both modes.
For the11-token collector fixture errors occur at0,1,2,4,5,6,7,8,9,10. Store8, count10,
return malformed, preserve both numeric fields. Token3 is valid and admitted;
excess tokens are checked for lexical failure before capacity. Do not count every
character as another token error or run prefix arithmetic on a partial valid list.
The oracle does not prove arbitrary string lengths or pointer validity; the
invariants and independent cases are still required. Collect-all has separate
diagnostic ownership and work cost, not merely a different return value.

## t0106c

In analysis_only.c, a=INT_MAX,b=1 makes `a+b` undefined before the conditional.
No later comparison repairs that evaluation. A two-element prefix `[32767,1]`
already exceeds the synthetic contract; with a wider accumulator this observation
is defined C, not actual signed overflow. The final-only mutant needs three values
`[32767,1,-1]` to accept a final-fit cancellation; no valid singleton or pair can
fail a prefix then recover. workflow.py minimizes both distinct predicates and
checks deletion minimality. Do not mislabel the three-value result as the smallest
counterexample to signed check-after-add itself.

The stale-build experiment changes list.c, relinks unchanged list.o, and observes
the same defect; hashes prove the object was reused. Recompiling that source changes
the object and executable and restores prefix rejection. A timestamp heuristic or
“clean fixed it” without identifying the omitted compilation is insufficient.
For level3, predict which source/object edges are missing and retain a regression
that checks executable behavior and build provenance independently.

## Limits and alternative solutions

A different lexer, explicit two-pass validator, or owned diagnostic container is
valid only if it implements the declared public ABI, bounded storage, exact status
precedence and lifetime/failure rules. No allocating parser shortcut is accepted.
Public tests, finite host oracle, sanitizers and observed compiler behavior establish
only their recorded bounds. The private compile-only UB specimen is never linked
to a test driver. External review, seven-day learner performance and physical
measurements are not created by these author experiments.
