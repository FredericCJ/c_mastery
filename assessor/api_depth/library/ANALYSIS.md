# T22 assessor technical analysis

This is author analysis, not an independent review. Keep it out of learner
packages. The original leaf rubric still determines assessment; do not award a
whole objective from a loosely related successful command.

## Organization, naming and documentation (T22.01–03)

Installed `lc.h` depends only on standard size/integer types. `api_depth.h`, the
generated release/layout header and constructor support header are private to
the package build. Removing access to source/build directories is a stronger
packaging discriminator than counting five directory roles. Static consumers
still need the provider archive because the wrapper actually calls api_append.
A private link dependency is not the same as a public header dependency.

The board and host models use distinct contexts/biases and operation-table
entries. Their arithmetic results differ, and both call counts must remain1.
An unused directory does not establish backend isolation. Native build evidence
does not claim physical board execution.

The original ambiguous adapter `Old.p/n` and explicit `Named.owned_buffer/
capacity_bytes` deliberately preserve units and behavior. The `LCElements` versus
`struct lc_bytes` translation failure is additional enforcement. Creating a wrapper with
the wrong number still passes the type checker; runtime checked multiplication
and available-byte admission address a different defect. A timeout name alone
cannot establish duration units; no timeout API is introduced unnecessarily.

Constructor failure at1 yields acquisition[1], no release. Failure at2 after
required2 yields[1,2,11]; failure at3 gives[1,2,3,12,11]. Optional2 skipped gives
[1,3,11] on failure3 and [1,3,13,11] on successful destruction. Both nested and
linear variants preserve output mask99 on failure. The cleanup mutant skips
release1: nominal construction still succeeds but independent live bits fail.
The helper variant moves predicates into acquire_step. Its locally lower token
metric leaves the same resource states and callback effects. Do not describe the
lexical indicator as a complete cyclomatic analysis.

Doxygen output is inspected for parameter text in both comment positions. A
successful renderer cannot detect the intentionally ignored append status; that
example reports the wrong logical length and returns17. Capacity rejection must
preserve nonempty state, not merely an empty object's length. The callback
rationale is tested by request→poll→ack; the early-terminal mutant violates
LC_BUSY/pending even before any unsafe access. Public API requirements live in
the header; architecture notes explain alternatives without divergent duplicate
normative clauses.

## Traceability and packaging (T22.04–05)

The original fixture leavesR3 uncovered, T9 orphaned andR9 unknown. A validly
linked assertion-freeT3 closes the navigation gap but remains a depth defect.
The split R1a/R1b points to historicR1 rather than replacing its source-bound
evidence. An analysis recordP3 is labeled author-derived, not executable or
externally reviewed. The shared append fixture has separate capacity(snapshot)
and ownership(no release, borrow retained) claims; matching input is not enough
to merge their rubric obligations.

CMake installed targets are relative to their own prefix. The generated header
is absent from the include directory, and deliberate private/vendor includes
fail there. All outside consumer compiler/link output must refer to installed
headers/libraries. Source/build directory renaming affects only copied material
under build/packaging-followthrough. The optional static helper has a real symbol
reference; removing its archive must make consumer configuration fail. A copied
prefix with spaces tests both relocation and argument handling. A missing
developer-local source path is a negative control, not a justified prerequisite.

## Foreign and provenance boundaries (T22.06–07)

The library allocates/reclaims its opaque handles. Foreign callers borrow views
or copy to their own output. A returned pointer must keep its address for paired
release; declaring c_char_p would often hide it in an automatic byte copy. The
ctypes descriptor compares actual pointer/size_t/alignment facts. It never calls
a knowingly mismatched pointer-return signature; integer reconstruction exposes
the truncation question safely. The real32-bit C client establishes only that
ABI, not a32-bit Python runtime.

All descriptor capacity/version checks precede writes. No native struct cast is
used. Release2 accepts a longer output buffer as a compatible prefix and leaves
the tail untouched; an unsupported ABI or short buffer remains rejected. Input
length controls embedded zero. The retained Python callback object survives an
explicit collection until acknowledgment; terminal callback fires once.

The target reuses original repository ApiBuffer source, identified by exact hash,
not copied third-party atom code. The independent oracle is Python byte-sequence
algebra. Hosted runtime malloc/free are visible dependencies, not first-principles
append implementations. Static/shared/LTO symbol observations must be reconciled
with the ledger. No blanket runtime-closure claim follows from a lockfile.

Exact local packaged copyright records were read for CMake, Doxygen and libc.
CMake's top-level BSD label does not cover its differently scoped Apache/GPL+
exception/Zlib components; Doxygen's main GPL notice differs from its generated
JavaScript sources' stated terms. These are host tools, not vendored target
implementations. The repository has no discovered top-level license grant in the
bounded scan; distribution permission remains unresolved. Source-only versus a
linked binary or bundled documentation requires a fresh artifact-specific review.
Documenting these questions completes the author analysis, not legal approval.

## Diagnosis, refactoring and release (T22.08–10)

Two hypotheses explain a missing resource: failed acquisition versus skipped
cleanup. The acquisition log and live mask discriminate them: acquire failure
never transfers that resource, whereas the cleanup mutant leaves resource1 live.
This distinction survives compiler inlining/LTO. The public append path is
lc_append→ApiBuffer validation/overlap admission→byte copy→length publication.
Constructor publication follows callback acquisition; destruction calls the
stored release callback. Callback context is the only external state and stays
live under the declared nonreentrancy contract. The backend table has two real
implementations with distinct context semantics, so a direct-only graph is wrong.

The deterministic scheduler enumerates all six orders of request, poll and ack.
Request before ack is the minimal premature-retirement witness. A poll that wins
first completes successfully; a subsequent cancel cannot retroactively cancel
the already completed work. No thread race or real use-after-free execution is
claimed by this serialized model.

Representation2 inserts a private field while existing consumers retain their
behavior. The append mutant changes failure length, an intentional contract break
that old tests reject. Synchronous/deferred operations demonstrate another break:
identical call syntax would not preserve context lifetime if timing silently
changed. Release notes require an explicit new operation/lifetime migration.

Bisection uses the real earlier producer with its intentional mutation, not a
revision-number oracle. The wrapper returns1 only for the exact rejected-append
length marker,125 for a translation failure,200 for harness/other defects. One
history has a unique culprit; the second has adjacent untestable/defective
revisions and must report both candidates. At capacity1 with prior length0,
input length2 is minimal: length1 succeeds and does not expose the mutation.
Every Git mutation is confined to a newly created local teaching repository.

Release1/release2 extend requested capacity8→16 and descriptor admission without
changing ABI1. Same-compiler byte comparisons normalize debug paths but retain
actual hashes; changed-compiler hashes may differ despite passing behavior.
Separate debug information must have the binary's actual build identifier.
Fresh outside processes test old→replacement→old loader selection; this is not
hot replacement in a running process. Version1 persisted data cannot represent
a nine-byte version2 record. Refusing downgrade preserves both old and new files
and states a rollback limit; inventing a successful lossy downgrade fails.

Hints escalate from inspecting first mutation/lookup, to independent snapshots,
to minimizing one acquisition or revision. Withheld learner variants should
change the exact assumptions retained in the canonical criteria. Finite author
tests and the same-author model do not establish external review or mastery.
