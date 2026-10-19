# CE-M1â€“M5 assessor analysis

A good solution separates parse/type rejection, CFG proof, runtime bounds/fuel
and target ABI. The supplied implementation is an original bounded reference,
not a suggested shortcut for learner construction. Its teaching notation has
unsigned32 literals and a combined comparison level; it is not ISO C syntax.
Unsigned modulo arithmetic is defined in the source language explicitly. A C
comparison uses separately written unsigned expressions, and pointer C oracles
are run only when their indices are inside the array or one past for formation.

The parser publishes one temporary program after verification. Literal overflow
is tested before multiply-add. Every lexical failure retains the whole malformed
token extent. Pointer values use local frame identity implicitly, preventing
escape only because calls and returns are scalar. This is a representation rule
of the compiler, not a general theorem about integer pointer encodings in C.
The verifier checks bounded function ranges, earlier-only calls, local indices,
branch destinations and reachable exact type stacks. Equal height alone is not
sufficient: the deliberately injected pointer/u32 join passes a height-only
check and later violates a typed return. Unreachable instructions receive
structural checks but no type reachability claim; the emitter skips them.

Each target uses fixed operand slots in a fixed frame. x86 saves RBP then reserves
512 bytes, preserving16-byte pre-call alignment; Thumb saves20 bytes then
reserves508, totaling528 and preserving8-byte pre-call alignment. The saved frame
pointer, original argument and live operand slots survive nested calls. No
external helper is needed in emitted code. Fuel and status pointer slots are
separate from locals, arrays and operands. Every call shares the same fuel word,
so recursively resetting a private budget would be an incorrect repair.

Reference mutation1 reverses subtraction; mutation2 tolerates unequal join
types; mutation3 admits a one-past read and returns a harmless synthetic value.
The last mutant avoids executing host undefined behavior while still violating
the source contract. All must fail named semantic assertions, never merely fail
to compile. The native/Thumb harness must also test failure status and actual
callee-state preservation before expanding ABI claims beyond nested-call output.

Changed-condition assessment: with two arrays, a pointer needs allocation identity
and index. Merely allowing indices0..15 merges distinct C objects incorrectly.
With side-effecting calls, the current left-to-right traversal becomes a stated
language sequencing rule; ISO C's unspecified operand order is not an equivalent
promise. Signed32 needs mathematical overflow admission and explicit conversion
rules; casting a wrapping unsigned result afterward does not establish them.
Serialized IR must be decoded into owned objects with canonical fields before
verification; dumping CEProgram bytes would leak padding and host endianness.
A Thumb-1 backend must replace movw/movt and unsupported addressing forms and
execute on an appropriate CPU; assembly acceptance by a Cortex-M4 assembler is
not that evidence.

Rubric guidance: require objective-specific explanations and original negative
inputs from the learner brief. Level1 is a passing fixture without causal
arguments. Level2 requires correct contracts, implementation and counterexample
regressions; level3 is the delayed independent changed condition. Author reports
are assisted implementation evidence only. No external reviewer is represented.

Signed continuation uses explicit i32/u32 types and exact equal-rank mixed
conversion. A checked unsigned-to-signed cast is a language policy, not an ISO C
guarantee. The interpreter converts stored signed bit patterns to int64 without
an out-of-range signed cast. Each signed operation admits its mathematical result
before converting back to stored bits. x86 uses overflow flags; Thumb-2 multiply
checks high product against sign extension; Thumb-1 uses bounded magnitudes and
prechecked addition/doubling. The INT_MIN times negative2 case specifically
requires rejecting a magnitude already larger than a positive limit before
subtracting it from that limit; otherwise unsigned subtraction can hide overflow.

Canonical IR decoding rejects reserved bytes, unknown version, exact-extent
mismatch, noncanonical names/unused types and semantically invalid CFG. No native
layout or padding is serialized. The text and byte entrances share validation,
but their parsers are independently fuzzed. Retargeting uses local literal islands
so long branches need no guessed short reach. The hostile Thumb-1 caller uses only
supported low-register pushes plus explicit saved high registers. Current fixtures
include actual signed failure status on every backend; they never evaluate an
undefined C overflow oracle.

CE-M9 exposes the specified left-to-right traversal through a main-owned effect
counter. `bump(x)-bump(x)+bump(x)` evaluates x, x+1, x+2 and yields x+1 modulo2^32.
The counter pointer occupies its own preserved frame slot. On Thumb, save the
incoming hidden fourth argument before using r3 to clear local storage; otherwise
nested effects use a null counter even when effect-free call tests pass. Only
main installs and zeroes its own counter; all nested calls borrow that storage.
The separately compiled C oracle sequences its effects in statements rather than
assuming C operand order. A learner who reproduces the result without explaining
this distinction has not demonstrated sequencing transfer.
