# T46.02: verification is a stated theorem

This successor implements the three original `AX-PR-T46-02-T46.02.{a,b,c}`
assessments and their unchanged seven-day transfer tasks. Read the original
project and `labs/runtime/SPEC.md` first. Bring your T46.01 raw CVM1 codec,
instruction decoding, finite queues and set reasoning. The reference campaign
imports the separately identified author codec/VM; that is not learner work.

The input is a bounded `struct cvm_program`: up to eight contiguous function regions,
1024 code bytes, eight locals per function and 64 frame-relative operand slots.
Its data representation is independent of whether its instructions are safe to
dispatch. A byte inside an immediate operand is not a branch destination merely
because it resembles an opcode. Decode every function completely, including
instructions with no incoming reachable edge, before following control flow.

Implement the three functions in `learner.c` using `verifier.h`. Do not call
`cvm_verify` to implement `vv_verify`, or use a parser/verifier library. The
lowerer's final compatibility check may use the predecessor `cvm_verify`.
Use one declared rule table for widths, operand requirements, stack effects and
edge kinds. CALL derives its argument count from the checked function descriptor.
No heap, recursion, global mutable verifier state or hidden runtime is permitted.
The caller owns disjoint, live input, output, report and workspace objects;
serialize calls sharing scratch. Report/scratch can change on rejection. Load
and lowering must preserve every output byte on rejection. Do not compare
uninitialized padding as a semantic property: the tests initialize all sentinel
bytes before checking preservation.

For the base verifier, entry height is zero for each function, CALL consumes its
arguments and produces one return value, RET requires exactly one value, and
every reachable join requires equal incoming heights. Unreachable instructions
still need valid widths, local/call operands and branch targets. Their stack
effects need not be reachable-safe. A reachable fallthrough beyond its function
is invalid. Record first-discovery count, processed count, examined edges and
maximum queued nodes. For a valid base graph, every reachable instruction is
processed once; a second edge checks its established height rather than walking
the cycle recursively. With V instruction starts and E edges, decoding plus
analysis takes O(code bytes + V + E), and bounded storage is O(code bytes).

Derive that argument on `NIL; TRUE; JFALSE exit; JMP loop; ...`. Adding an extra
NIL on only the backedge changes the loop's incoming height even when the first
visit was valid. Give a malformed and nearest-valid pair for interior targets,
cross-function edges, local indices, call arity and unequal heights. Predict the
phase and location before running each pair. A verifier that rejects all branches
does not satisfy this contract.

The first changed condition adds opcode 23, SPLIT, width five: pop one Boolean;
read true and false absolute little-endian 16-bit destinations at offsets 1 and
3; no fallthrough. Select this dialect explicitly; an ordinary CVM1 VM must reject
23. Implement `vv_lower`: replace SPLIT with `JFALSE false; JMP true`, relocate
all branches and function boundaries, then validate the ordinary output. Expansion
can exceed 1024 bytes even when every added instruction is unreachable. Reject
atomically. Execute both arms and a later relocated function on your unchanged VM.
This is a compiler lowering contract, not permission to send extended bytes to an
old dispatcher. Raw CVM1 bytes do not encode this out-of-band dialect selector.

The second changed condition tracks **definite initialization**, not possible
values. Parameters are initialized on entry. STORE adds its local; LOAD requires
its local on every incoming abstract path. At a join retain only facts common to
both incoming states. If a later path removes a fact, propagate that change even
through an instruction already processed. Draw a short initialized arm and a long
uninitialized arm joining immediately before LOAD. A one-visit algorithm is now
wrong. Bound the revised algorithm: after its first discovery, a node loses at
most eight bits; queued flags suppress duplicate pending entries. Derive the
processed/edge/queue bounds and measure them, instead of retaining the earlier
one-processing claim.

This analysis conservatively follows both syntactic edges, even for TRUE. It can
reject a program whose uninitialized path is concretely infeasible. Document
that precision limit. Initialization does not establish value tags: `NIL; STORE0;
LOAD0` is initialized. Structural verification alone accepts an uninitialized
LOAD, Boolean arithmetic, a balanced infinite loop, and a caller retaining 63
operands while its callee pushes two. Run these only in the original bounded VM:
expect TYPE, TYPE, FUEL and STACK_LIMIT, respectively, with unchanged result
output. Do not turn guest errors into reads of uninitialized C objects.

Submit the implementation, prediction ledger, rule table, termination and
failure-atomicity arguments, instruction-level traces, maximum-resource account,
independent malformed/valid pairs, and evidence for actual predecessor reuse.
The host check is:

```
python3 labs/runtime_followthrough/verifier/run.py --source YOUR/verifier.c --cvm YOUR/cvm.c --build build/my-verifier
```

The supplied stub compiles and fails semantic assertions. Public tests are a
minimum contract; assessor path-set cases and controls are separate. C17 is the
implementation baseline. No C23-only feature is used; guest typing is defined by
this VM, not by either ISO C edition. Native host sanitizers are host tools.
`target_build.py` requires explicitly named original byte/ABI helper providers,
links direct objects, and reports their imports. M4/PowerPC64 execution is
emulation. Neither emulation, finite tests nor a static stack estimate establishes
physical timing or absolute mastery.

Hints, in order: (1) mark starts before inspecting branch destinations;
(2) separate unknown height from height zero; (3) distinguish first discovery from
changed abstract state; (4) retain a map from original starts and function ends to
new offsets before emitting any branch operand.

The conceptual use of monotone transfer and finite lattices is supported by the
inspected [Clang dataflow introduction](https://clang.llvm.org/docs/DataFlowAnalysisIntro.html#formalization).
Its possible-value example uses union; this exercise's must-initialized facts use
reverse inclusion and intersection. The original CVM specification remains the
authority for guest behavior. See the successor source register for inspected
passages and access limits.

All malformed inputs are synthetic, confined to the original teaching VM or the
learner implementation. No native code execution, privilege escalation or attack
against another implementation is claimed. Preserve runtime checks and bounded
fuel; a guest rejection is the harmless observable result.
