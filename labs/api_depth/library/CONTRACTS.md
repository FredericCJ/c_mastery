# T22 library-engineering successor contract

Version1 supplements PR-T22-01..04 and their original30 leaf criteria and30
seven-day variants. Their stable `AX-PR-T22-*` identifiers, problem statements,
rubrics and delay rules remain authoritative. This contract supplies concrete
inputs, operations, fixtures and executable author support; it does not replace
learner performance with author tests. All these exercises are required.

## Prerequisites and environment

Use the learner's earlier `ApiBuffer`/`api_append` artifact from
`labs/api_depth/include/api_depth.h`, together with its stated failure and overlap
contract. The author implementation imports the existing producer unchanged.
Know pointer/length validity, allocator pairing, callback lifetime, C17 types,
compilation/linking and T18/T21 contracts. Keep the original project dossier and
failure predictions. A working artifact is prerequisite to packaging it; a
source listing alone is insufficient.

Profile A is hosted C17 on the recorded Linux implementation. The component may
allocate through its own C allocator; CMake, Python, Doxygen, Git, compiler tools
and sanitizers are host verification dependencies. They do not implement append.
The original target provider and adapter are separate compilation inputs. No
strict-freestanding claim is made for an allocating hosted library. Compiler
visibility attributes, ELF symbols, GNU debug links and Linux loader behavior
are explicitly non-ISO layers. C17 is the interface baseline, not an assertion
that live tool documentation is an exact C17 standard text.

The separate teaching-board backend is a small original hosted register model.
It takes a context, records calls and applies its selected policy. The hosted
build requires no board toolchain. A real board port and physical timing remain
separate evidence; model success does not establish either.

## Problem, inputs and outputs

Implement the opaque operations in `include/lc.h`, replacing `learner/lc.c`.
Reuse the earlier append primitive, then add the missing ABI/ownership adapter.
The header specifies exact capacities, descriptor layout, units, failure and
callback transitions. Public C tests supply empty, full, embedded-zero, mixed
unit, allocation-failure and reordered-cancellation inputs. Examples have both
a status-aware consumer and an intentionally flawed unchecked-result consumer.
The latter must be rejected for its incorrect promised result, without calling
undefined behavior a successful exploit.

Implement the constructor and adapters in `support.h`. Resource IDs1,2,3 stand
for distinct live resources; acquire returns nonzero on ownership transfer and
zero on failure with no ownership. Release is infallible, cannot reenter, and
must run exactly once in descending acquisition order. Resource2 is optional.
The output is unchanged on any failed acquisition. Context and all callbacks
stay live through destruction. Successful output must be destroyed before reuse.
Compare nested, staged and helper-based representations using the same trace.
Backend integers are restricted so selected addition/subtraction fits `int`;
the teaching inputs10/bias3 or7 satisfy that precondition. Counters must not wrap.

Build/install with `CMakeLists.txt`. Supply `LC_PROVIDER_ROOT` containing the actual
earlier producer `.c` and header; the default implementation is the unsolved
learner folder. `LC_IMPLEMENTATION_DIR` is an explicit alternate implementation
path for a learner's own source. The assessor campaign stages its private
implementation separately; no assessor source belongs in a learner export.
Build and install interfaces must differ. Only `lc.h` is installed as a public
header; generated `lc_private.h` belongs to the build directory.

`LC_BACKEND=HOST`, `MODEL` or `BOTH` selects explicit auxiliary teaching-backend
source sets; `NONE` leaves that optional investigation out of the minimal byte
library. Inspect the actual compilation database for each selection. Both-model
runtime comparison uses the same public support contract and separate contexts.

Use an outside project with `find_package(Course 1 REQUIRED CONFIG)` and
`Course::course`. Build static/shared versions, move the prefix into a path with
spaces, and make source/build copies unavailable within the disposable workspace.
Install and inspect the private dependency archive needed by static consumers.
Removing it must yield a meaningful consumer diagnostic. No real curriculum
directory may be moved, deleted or reset by this task.

## Mechanisms and reasoning

Before running tests, state why a function name, clean directory tree, generated
manual, complete trace table or identical binary is insufficient evidence of its
corresponding semantic claim. Distinguish an opaque ABI from a public struct,
pointer width from length width, source from semantic compatibility, and a
successful cancel request from callback quiescence. A callback retaining its
context after the synchronous API returns would break lifetime even with an
unchanged function signature.

Append must preserve bytes and length on rejection. Admission subtracts only
after validating the buffer invariant. Explain why checking capacity after a
write cannot be repaired by changing the returned status. The producer's overlap
policy forbids input aliasing the object. Callers must provide valid spans;
neither ISO C nor this adapter validates arbitrary pointers.

Constructor outputs are published once, after all selected acquisitions. Derive
the acquired-prefix state set, reverse cleanup trace and optional-resource case.
Name hidden callback effects in the call graph. Lexical branch counts are a
review prompt, not a proof; the helper variant relocates obligations rather than
eliminating them. Inline/LTO experiments require the same source-level argument
even when generated call boundaries disappear.

For foreign calls, declare every `argtypes` and `restype`, use `size_t` accurately,
preserve the original owned pointer, and retain callback wrappers through the
terminal transition. Analyze the default-int pointer return as an integer-width
counterexample without dereferencing a truncated pointer. Run a supported
32-bit C consumer and record its actual widths; do not infer 32-bit Python from
that test. The descriptor is bytes, so no private struct layout is guessed.

One buffer costs its opaque object plus a bounded byte representation; allocation
occurs at creation, never append. Copy is O(length); the inherited producer's
overlap admission has its own bounded nested comparison cost. Borrowing avoids
copying but couples lifetime. Each operation allocates one state record, and the
constructor uses constant staging space. Packaging modes change deployment and
link closure, not these correctness contracts. Report actual code sizes and
dependencies if making resource comparisons; do not infer a speedup from size.

## Deliverables and verification

- D1: Predictions, public/private role inventory, selected backend/object/include
  lists and clean commands with exact tools/configuration. Detect deliberately
  leaked generated/vendor headers and a developer-local dependency path.
- D2: Correct implementation, before/after constructor traces, naming adapter,
  mixed byte/element diagnostic, callback ownership map and compiled examples.
- D3: Doxygen HTML/XML with recognized before/after parameter documentation, plus
  the separately maintained architecture decision narrative and working links.
- D4: Stable requirement/test IDs, source-bound historical result records,
  supersession links, explicit shared-fixture objective clauses, and structural
  orphan versus assertion-depth reports. A proof artifact may satisfy an
  analysis-only requirement, with author versus external review stated honestly.
- D5: Installed/static/shared manifests, fresh outside C and ctypes consumers,
  negative version/length/dependency controls, actual 32-bit report, ownership
  ledger, direct/transitive/generated/runtime/host-tool provenance and exact
  inspected notices. Unresolved distribution questions must remain visible.
- D6: Competing defect hypotheses, concrete failure traces, call/data graph,
  actual disassembly, retained characterization and semantic mutants. Preserve
  diagnostic/failure output and resource order, not just success values.
- D7: A disposable original Git history with good, bad and unbuildable revisions;
  a same-failure predicate, explicit125 skip, exact/ambiguous bisection results
  and minimized input. Never operate on the curriculum repository's history.
- D8: Two real releases, matching debug identifiers, source/config/tool hashes,
  repeat-build comparison, changed-compiler checks, executable migration and
  replacement/rollback evidence on synthetic files. Retain the old artifact.

Run each listed C test against the implementation and applicable deliberately
flawed variant. Six GCC/Clang profiles cover O0, O3/NDEBUG and ASan/UBSan. Test all
constructor failure prefixes with/without resource2, descriptor sizes0/15/16/24,
ABI0/1/2, capacities0..8 and incoming lengths0..9. Independently model all450
valid prior-length/capacity/incoming combinations in the foreign harness. The
original producer is a dependency, not an independent oracle. Python's byte
concatenation model is a separate formulation, not a formal proof of either.

Release checks require real file hashes and build identifiers. The two small
persistent formats and rollback admission are defined in `RELEASES.md`. A byte
copy migration is not a durability test. The current author campaign is
`tools/run_packaging_followthrough.py`; its source-bound report identifies every
command and outcome. It does not administer delayed learner assessment.

## Rubric and graduated hints

Use each original leaf's objective-specific0..3 rubric. Level0 fails the stated
property or evidence identity. Level1 gets nominal output but cannot explain
ownership, admission or the failed counterexample. Level2 supplies the mechanism,
independent expected relation, repaired implementation and rejecting negative
control. Level3 solves that leaf's exact changed condition after the required
delay, defends which guarantees changed, and limits the claim to the evidence.
Author execution alone never receives learner level3.

Hint1: inspect the first changed state or lookup path, before the final result.
Hint2: snapshot the object before rejection; record acquired/live resources and
callback terminal state independently. For packaging, remove accidental access
to source-tree headers before blaming link flags.
Hint3: reduce to one empty/full buffer, one failed acquisition, one canceled
operation, one install-only consumer or the shortest bad revision input. Keep
the minimized discriminator after repair. Assessor implementations and technical
analysis remain under `assessor/api_depth/library/`.

## Variants, containment and evidence limits

Every original `AX-PR-T22-*-DELAYED` task remains required, including the optional
resource, moved prefix, static private dependency, larger descriptor prefix,
caller-owned output, LTO audit, independent oracle, changed distribution, indirect
backend, deterministic event ordering, ambiguous bisection and deferred callback
migration. The exact pairing is recorded in the successor's bindings; no variant
is satisfied merely by relabeling an ordinary test.

All executable defect demonstrations target this original library, its explicit
mutants and synthetic files in verified disposable directories. Attacker-like
inputs are byte lengths, descriptor versions or event order within this target.
Success is a harmless violated snapshot/lifetime-state/trace property; it is not
arbitrary code execution. Repairs restore admission-before-publication, precise
release pairing or dependency closure. Invalid raw pointers and callbacks after
real free are analyzed rather than executed as expected-output controls.

Author code, actual execution, external review, independent learner performance
and physical observation are different dimensions. Published ISO C17, current
distribution permission, arbitrary concurrency and physical board behavior are
not established here. Private Doxygen provisioning changes no host configuration.
Passing finite tests cannot prove universal safety, compatibility or mastery.
