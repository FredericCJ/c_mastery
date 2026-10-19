# PR-T35-04 actual hardening successor, version 1

This required additive contract retains AX-PR-T35-04-T35.07.a/b/c and each
original `-DELAYED` identity. Read the parent project, its three exact criteria,
the legacy assessment shared contract and this contract together. Historical
acceptance and hours are unchanged. Seven days between study and independent
transfer is a learner gate; author execution does not satisfy it.

## Prerequisites and environment

Bring the parent's ELF/linkage, process-map, pointer/extent and ownership
reasoning, an unfinished audit implementation and a prediction of the supplied
`fixtures/embedded/PR-T35-04.json`. The hosted target is original C17 code on
Linux x86-64, GCC and Clang, with the installed glibc and a separately declared
musl1.2.5 comparison. Profile A permits the compiler/libc instrumentation being
studied; it does not turn that provider into an ISO C guarantee. Python,
readelf, objdump, nm and size are host verification tools. Implement the
declared-field classifier and the two repairs yourself. No ELF library is
needed for that classifier; actual tool output is independently retained.

The changed no-runtime target is Cortex-M4 Thumb, C17 freestanding, own
`target.ld`, no startup, loader, libc, allocation, formatted logging or implicit
compiler helper beyond the explicitly supplied guard/check-failure interface.
`target_entry.c` is a link entry for inspection, **not board startup**. This
image is never flashed. A host build tests the finite diagnostic action;
disassembly checks the target handler and terminal wait. GNU inline `wfi`,
ELF, Linux mappings and the compiler guard ABI are named extensions/contracts.

## Concrete implementation and reasoning

Implement `sh_copy(src,count,out)`: accessible nonoverlapping input/output,
count0..8 admitted, null input allowed only for zero count, nonnull output.
Stage an eight-byte zero-filled local value, copy the admitted prefix, publish
the sum of all eight bytes. Reject other count/null cases with -1 and unchanged
output. Success returns0. Sum fits unsigned on every conforming C17 target.
No arithmetic-overflow or source-extent inference from a nonnull pointer is
allowed. `sh_consume` in a separate translation unit keeps the stack object
observable in the inspected builds; do not add LTO silently.

Implement `sh_authorize(claimed_role,trusted,counter)`: only trusted==1 grants
one increment of a live nonnull unsigned counter below UINT_MAX. The wire role
confers no authority. Denial/invalid/full leaves the counter unchanged. The
trusted fact is supplied by this laboratory boundary, not authenticated by
this function. The original flawed authority function accepts role1 with
trusted0 and moves only a synthetic counter7→8.

Implement `classify(value)` in `audit_stub.py` for the exact supplied schema.
Return the nine expected fields. Separate DYN from explicit PIE marking,
RELRO presence from eager binding, declared non-executable stack from runtime
maps, symbol dependency from eligible sites, and changed bases from a universal
randomness guarantee. Declared headers are inputs, not observed machine facts.
Run `audit_tests.py candidate.py fixture.json`; its changed predicates must
falsify overstatements independently of the expected fixture object.

For the changed guard interface, `sh_record_failure` performs exactly one
volatile uint32 write of 0x53484746. `__stack_chk_fail` calls it and never
returns: it waits indefinitely with `wfi` on the selected target. There is no
allocation, lock, logging callback or retry. The diagnostic action is bounded;
the terminal state deliberately is not a finite-time recovery operation.
Compile this handler without its own stack protector to avoid recursive
failure. The fixed teaching guard 0x63c291a7 is inspectable and supplies no
entropy/secrecy guarantee. No return to compromised control flow is permitted.
Implement `target_stub.c`; audit every final undefined symbol and helper call.

## Experiments and deliverables

D1: Before execution, derive valid sums0,1,3,…,36, rejection preservation at9,
the trusted/claimed-role truth table and the synthetic ELF predicates. Explain
why bounds correctness, memory-layout mitigation and authority are different
properties. Include plausible wrong explanations and a discriminating case.

D2: Build plain and hardened original Linux programs. Preserve commands,
versions, exact binaries, `readelf -h/-l/-d/-s`, disassembly, link driver trace,
preprocessor fortify macros and size. Plain explicitly omits PIE, RELRO,
eager binding, stack protector and fortification; it retains NX stack and is
executed only with defined input. Hardened uses supported PIE, RELRO/NOW,
NX stack, strong guard and fortify3. GCC aggregate is a separate experiment.
The installed Ubuntu GCC default already defines fortify3: `-fhardened` emits
an explicit-override warning. Keep that diagnostic, use only
`-Wno-error=hardened` for this diagnosed aggregate experiment, and verify the
actual macro/sites. Other warnings remain errors. Do not report this as a
clean aggregate request or weaken any semantic test. Clang's unsupported
aggregate flag is a negative compile result; its individually supported
flags are tested separately.

D3: Read `/proc/sys/kernel/randomize_va_space` without writing it. Save four
actual process maps per build, locate image bases, stack permissions and each
page-covered RELRO prefix. Derive load bias; do not mistake a writable data
page outside RELRO for a failed guard. The supplied campaign selects the
observed 4096-byte mapping granularity; changing page size requires a revised
rounding argument and audit. Preserve missing/conditional observations.
Locate both check calls in `sh_flawed_copy`, then inspect
`sh_optimized_away`: its source array need not survive optimization. An
imported symbol cannot prove that this latter function has a check.

D4: In separate bounded processes, run only the supplied nine-byte original
copy defect under fortification or AddressSanitizer. Require the intended
diagnostic and termination, then require the repaired copy to reject the
same count without altering output. No crash alone earns a result. Compare
the valid data-only policy defect and repair in every build, including the
aggregate: no code injection or pointer corruption is involved.

The separate `guard_control.c` changes the compiler-declared global guard
through an ordinary object write while a protected original probe is active.
With the explicit global-guard ABI option, require its named hosted handler
and exit42. This tests actual compiler guard checking without an invalid
memory access. It is not the hosted TLS policy or the freestanding handler;
do not merge those environments into one enforcement claim.

D5: Compare31 raw monotonic-clock samples of200,000 valid copies per binary.
Require checksum223,200,000 and retained calls with an identical valid-copy
contract. Report distributions, code/data size, relevant stack frames,
compiler helper/libc/loader/startup dependencies and measurement conditions.
Do not infer universal speedup or overhead from noisy runs. Compiler/libc
changed-condition work uses actual musl1.2.5 headers/runtime: an accepted
`_FORTIFY_SOURCE=3` macro does not add glibc's checked memcpy wrapper there.
The static musl variant has a different startup/link topology; explain it,
and compare valid semantics before interpreting cost.

D6: Compile the Arm probe with strong protection and retain its unresolved
guard/check-failure references. Show the no-runtime link fails without them,
then provide the original handler, produce a map/disassembly, and require an
empty final undefined-symbol list and no loader/dynamic section. Contrast
hosted TLS guard and libc failure/fortify paths with this explicit target ABI.
The native record test and link audit do not demonstrate target startup,
exception integrity, asynchronous faults or physical timing.

## Verification and objective rubric

For .07.a, passing requires D1–D4, actual distinct binaries/maps, the isolated
diagnostic plus repaired control, and a surviving data-only counterexample.
Level1 merely lists flags; level2 explains eligibility and exact violations;
level3 independently transfers the authority counterexample under changed
mitigations, preserving the original delayed identity.

For .07.b, passing requires D2/D3/D5: effective aggregate and unsupported-flag
results, per-site post-optimization reasoning and the actual libc change.
Level1 trusts the requested macro; level2 explains actual wrappers/sites and
runtime mappings; level3 repeats under a new supported compiler/libc and
derives which prior observations must be recollected.

For .07.c, passing requires D5/D6 and an explicit dependency ledger. Level1
copies hosted flags; level2 accounts for loader/startup/libc/guard references
and rejects the incomplete link; level3 implements/audits the bounded
failure action and terminal handler without introducing hidden dependencies.
No finite tests establish all executions or independent mastery.

## Hints and changed conditions

Hint1: separate requests, static markers, selected machine instructions and
runtime observations into four columns. Hint2: inspect the complete function
containing the array and the final object, not only the symbol table.
Hint3: compare `memcpy` preprocessing in both libc headers and trace every
undefined symbol in the freestanding object before adding a provider.

Keep exact variants AX-PR-T35-04-T35.07.a-DELAYED (data-only authority),
AX-PR-T35-04-T35.07.b-DELAYED (compiler/libc changes), and
AX-PR-T35-04-T35.07.c-DELAYED (original guard handler). The supplied runs are
author controls; independent learners receive changed tool versions, target
assumptions or policy inputs after the required delay, not a copied report.

## Security and evidence limits

The targets are only these original programs, fixed synthetic bytes/counter,
and own compile outputs. Attack capability is the bounded count or claimed
role admitted by the harness. The defect is an eight-byte destination extent
violation or unauthorized synthetic increment. Observable harmless success
is the intended local detector result or counter7→8, never an arbitrary-code
execution/privilege-escalation claim. The repair checks extent before copying
and derives permission solely from trusted state; regressions exercise valid,
zero, boundary, rejection and changed-policy cases. Mitigations remain useful
but incomplete. No external services, real secrets, host-policy changes or
hardware flashing are authorized here. Separate children contain detector
termination; time is bounded by the host runner. ASan is a verification
dependency, not a target runtime requirement for the freestanding image.

Current exact execution evidence is the source-bound author report
`research/v5/expansion/legacy_security_hardening/validation.json`; it is
separate from original acceptance, independent review/learner evidence and
physical observations. Exact final published C17/C23 source access remains
open. Assessor implementations/analysis are withheld from learner packaging.
