# A reproducible two-translation-unit list tool

This required successor supplies the nine original PR-T01-03 criteria and their
unchanged delayed variants. The assessment card retains its AX IDs and original
tasks. Read it with this concrete contract. Prerequisites are the PR-T01-01 checked
prefix invariant and PR-T01-02 object, initialization and sequencing reasoning;
bring those learner notebooks and your own diagnostic source. Reuse the prefix
proof while deriving this narrower arithmetic domain. No earlier library parser
or accumulator may implement the assessed mechanism for you.

Use hosted ISO C17, dependency profile A, Linux x86-64 System V, GCC and Clang.
The public interface requires int32_t, ASCII decimal encoding, and int with at
least32 value bits including sign. The wrapper can use stdio and the ordinary
CRT; the core uses no allocation, parsing library, locale conversion or POSIX
call. This is not a strict freestanding claim. Record actual linked dependencies.
C23 is not needed and does not make a post-overflow signed check valid. The
inspected language source is specifically the N1570 C11 draft, not published C17.

Implement `learner.c`. `list.h`, `main.c`, `tests.c`, `replay.sh` and `run.py` are
supplied. The unfinished stub compiles and must fail semantically. Run
`python labs/foundation_list/run.py --build build/list-contract`, then use
`replay.sh` with explicit compiler, core source, public header directory and output
directory arguments. The CLI has exactly two application translation units:
your core and main. Headers are not a third application translation unit.

## T01.06.a: decide the grammar before choosing an implementation

Each argv token matches `[+-]?[0-9]+`, with no whitespace or suffix. Accept leading
zeros and negative zero; reject an empty string or lone sign as malformed. The
mathematical value must be in [-32768,32767]. There are at most4 tokens, with a
compile-time changed-capacity option1 through16. Every mathematical prefix must
also lie in that interval. Input `2 -5 9` yields `ok count=3 sum=6`; no input yields
`empty count=0 sum=0`. Both CLI successes exit0. Other cases print only the status
name and exit its enumerator: malformed2, range3, capacity4, prefix5, invalid6,
state7, unfinished8. No rejected request prints a partial numeric result.

`ft_tokens` borrows count live null-terminated strings and their pointer array
for the call; it retains no pointers. A null token is invalid. A null array is
permitted only for count0. Result and diagnostic objects must be live, writable
and mutually disjoint, with no overlap with input. Invalid top-level arguments
or collect values other than0/1 preserve both output objects. For admitted calls,
reset diagnostics first. Traverse token indices in order, assigning at most one
diagnostic per token: malformed/range/invalid takes priority over excess capacity.
Fail-fast stops on the first token error. Any token error suppresses arithmetic.
Only if all tokens pass, validate every prefix; an arithmetic diagnostic uses
index=count as an aggregate sentinel, not an offending token index. Every error
preserves the entire numeric result; successful empty input publishes0/0.

Derive a digit-state machine with untouched, sign-only, digits and malformed
states. Before `m*10+d`, compare m,d to the sign-dependent limit quotient/remainder.
Remember out-of-range without evaluating an overflowing multiply. Continue lexical
inspection so `999999999999999999999x` is malformed rather than range. Many leading
zeros need no growing buffer. For addition derive `sum <= 32767-v` for positive v
and `sum >= -32768-v` for negative v; explain why those comparison operands fit.
The core owns only bounded automatic state and needs linear time in inspected
bytes plus tokens, constant space for the fixed configured capacity.

For changed input implement `ft_stream_init/feed/finish`. Input bytes are borrowed
only during feed. Only space, tab, CR and LF separate tokens; repeated separators
collapse. A chunk boundary is not a token boundary, EOF is. Embedded NUL is a
malformed byte. No token length limit may reject valid arbitrarily many leading
zeros. Feed(NULL,0) is permitted on an initialized active stream. A token error
becomes sticky; no numeric result is published. Finish flushes the last token once,
then applies the same prefix arithmetic. A finished stream rejects further feed
or finish with STATE. Reinitialize to begin another request. Public stream fields
are representation, not permission for callers to forge an impossible state.

## T01.06.b: partition the contract and change output ownership

Predict normal, empty, singleton, exact-capacity, fifth-token, signed extrema,
one-beyond extrema, malformed sign/suffix, huge magnitude and both directions of
prefix failure. Distinguish final-fit/intermediate-fail cancellation from a final
out-of-range result. Include complete output preservation and repeated stream
finalization. Test every two-chunk split of a representative input and single-byte
feeding; provide your own additional partition and explain its mechanism.

For collect-all, `struct ft_errors` is caller-owned fixed storage: retain the first8 token
diagnostics in encounter order and count all errors in `total`. All input tokens
must be inspected; do not reset total after each failure, append beyond8 entries,
or lose the first status as the return value. An error at an excess-capacity token
is still lexical/range first. Arithmetic is assessed only after zero token errors.
This output is intentionally partial diagnostic information with an exact count,
while the numeric result remains failure-atomic. Compare fail-fast byte work and
collect-all linear work; neither mode may allocate storage proportional to input.
The host oracle may use regex and arbitrary-precision integers because it is not
the implementation under assessment. Public tests alone are not a proof.

## T01.04.a: observe the transformations

Predict and retain preprocessed core `.i`, generated assembly `.s`, two relocatable
objects and the linked executable. Run separate `-E`, `-S`, `-c`, link and execution
commands. Explain why a header is expanded during preprocessing and why unresolved
external references can be present in an object. Use readelf/nm/objdump to identify
the actual file kinds and symbols. Compare source, object and executable hashes.
For the delayed change remove a source suffix. Observe default driver treatment,
then use explicit `-x c`; check whether an object was actually produced rather
than accepting a zero driver exit alone. `-###` is a command plan, not execution.

## T01.04.b: connect source objects to an actual ABI

Compile `int mix(int a,int b){int local=a+b;return local*2;}` with caller(3,4).
Annotate integer parameter and return locations, saved state, stack pointer and
return address from actual disassembly under the stated System V ABI. Keep the
precondition that these int operations are representable. Distinguish source
automatic storage duration from a mandatory physical stack slot. Compare O0/O3
with debug data and explicit frame-pointer policy. A compiler can eliminate the
call in the caller or keep local only as dataflow. Inspect the out-of-line function
body as well; a forced frame pointer does not force each source local to memory.
Explain why this ABI observation is neither an ISO C guarantee nor a Windows ABI.
Label the address-space owner in the sketch: these are virtual addresses in the
running process, and the caller/callee share that process image and calling thread's
stack. A call does not create a process or a private address space. Another process
can use the same numerical virtual address for different storage. A thread in the
same process normally has its own stack but shares the process address space;
neither a C pointer type nor a function boundary provides isolation. This is OS
reasoning, not a claim that ISO C specifies virtual memory or thread stack layout.

## T01.04.c: isolate four failure stages

In newly created local teaching files, obtain a syntax diagnostic, a link failure
from omitting the core object, an existing ELF that cannot start because its
explicit interpreter path is absent, and a valid running CLI rejecting `12x`.
Retain the file and command evidence that separates each cause. For transfer,
build your original core as a local shared object, use an executable-relative
library path, run it, move only that owned library, observe loader failure and
restore it. Successful linking does not establish runtime dependency availability.
Do not change host loader/security settings or target other programs.
Here the initial core object is statically incorporated into the executable;
standard CRT/libc may still be dynamically linked. The changed experiment moves
that core into libft.so. Do not confuse component linkage with a fully static
executable or infer a library's availability merely from a symbol at link time.

## T01.05.a: replay from a fresh shell

Write an argv-safe script naming every stage and output. Start with a fresh
environment and explicit minimal PATH; do not rely on an interactive alias or
previous object. Repeat with spaces in source and destination paths. Quote each
path as one argument, keep compiler flags separate, and record command vectors.
Distinguish source identity from execution environment and process observations.

## T01.05.b: demonstrate the defect and repair as revisions

Use an original scratch Git repository, not curriculum history. Commit a controlled
defined final-only prefix check and its repair. In separate clean worktrees build
both with identical tests. Show normal agreement and cancellation disagreement.
For transfer, rename the core API in a separate revision, rebase the repair onto
that interface, and update the typed caller. Explain why the saved CLI regression
remains applicable while the old C caller requires an interface change. Record
all source revisions, source hashes and build products. A conflict-free rebase
does not establish that the resulting program satisfies its contract.

## T01.05.c: make the experiment reviewable

For both revisions retain compiler executable path/hash/version/target, source
and test hashes, exact argv, cwd, relevant environment, input, exit, stdout/stderr
and actual object/executable hashes. State every unsuccessful command's intended
reason. Replay with another compiler/optimization. Prefix algebra is unchanged;
instruction selection, diagnostics, debug locations and loader observations need
fresh evidence. No command plan or cached artifact counts as a fresh execution.

## T01.06.c: diagnose two independent causes

Analyze a signed check-after-add at INT_MAX,1. Identify the undefined addition
before the conditional; give no portable golden output. Separately use the wider
int32 teaching representation to investigate the narrower16-bit arithmetic
contract without executing C overflow. Minimize a violated prefix and a final-fit
cancellation example; say which property each minimality result concerns.
Retain the regression before changing the implementation.

For transfer, repair the source but deliberately relink an old object in an owned
build directory. Capture source/object/executable identities and unchanged faulty
output, then actually recompile and rerun. Explain the missing build dependency;
changing expected output or touching an unrelated file is not a semantic repair.

## Submission, hints and acceptance

Submit the two-TU source, public tests plus new cases, criterion-indexed predictions
and derivations, build script, revision/replay manifest and raw artifacts. For each
criterion distinguish0 wrong/absent,1 repeats an example,2 derives and verifies the
ordinary task with an independent counterexample,3 independently rederives the
changed condition. No one passing CLI input earns the nine criteria together.
The separate assessor analysis names the discriminating observations for each.

Hints in order: (1) mark the earliest state where a contract can fail; (2) keep
numeric publication separate from parsing and diagnostics; (3) for build symptoms,
trace which exact bytes entered the next stage before revising C code. The delayed
package is administered at least7 days later without the earlier solution. Author
execution of its implementation does not constitute an independent learner attempt.
Current executable evidence and its bounds are in the linked successor support;
external review, learner performance and physical observation remain separate.
