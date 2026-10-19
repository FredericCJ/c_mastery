# AP6: bounded Sv39 walk and changed translation regime

Required PR-T58-07 continuation, objectives T58.07.a/b/c, families A/B/C/D/F/G/I/J.
Incorporate `curriculum/v5/contracts/b06.md` and its D1–D6, rubric and containment
rules. Prerequisites are the learner's permission model, trap decoder and both
original privileged guests. Sources are the registered R-B06-FOLLOWUP-RVSUP,
R-B06-FOLLOWUP-RV-WALK, R-B06-FOLLOWUP-AAMMU passages; whole-Arm-manual inspection
remains a named source gap, not evidence supplied by a successful test.

## Problem and exact model

Implement `pw_translate` in `walker.h` from integer masks and bounded array reads.
PWMemory owns eight synthetic4096-byte pages with512 uint64 entries each. Physical
addresses here are integers: never convert them into host pointers. `present`
and `read_blocked` are eight-bit masks. `root` must be4096-aligned and below2^56.
An absent, blocked or out-of-range table read returns PW_ACCESS_FAULT; malformed
API flags, root or masks return PW_INVALID. Input/output are live, disjoint and
unchanged while the function runs; no callback, heap or library walker is allowed.
A failure leaves `*physical` unchanged. Successful output is a translated byte
address, not permission to dereference a C pointer or a host physical address.

The selected regime is Sv39 with Svade-style A/D faults and no optional PTE
extensions. Reject noncanonical virtual addresses; examine at most three levels.
Validate descriptor encoding before descending or using a leaf. A leaf may map
4KiB,2MiB or1GiB; require its corresponding physical alignment and preserve the
proper virtual offset. Apply original U/S/SUM/MXR and R/W/X policies plus accessed
and store-dirty requirements. Non-leaf U/A/D are reserved; software RSW and global
bits do not change this uncached translation. A next-level pointer at the final
level faults. Data physical-address range/PMP/PMA checks, misaligned multi-byte
accesses, optional extensions, hardware A/D writes and translation caches are
explicitly outside this model. Do not invent a host safety guarantee from it.

## Deliverables and verification

D1 derives index, canonicality, permission and superpage invariants, terminating
level descent, and the distinction between page and table-access faults. D2 is
source plus a transition trace. D3 runs `walker_test.c`:18,432 permission states,
3,072 offset cases and named canonicality/reserved/nonleaf/absent-table cases.
Expected addresses are independent base-plus-offset arithmetic, not a paired
decoder. D4 kills omitted-canonicality, omitted-superpage-alignment and SUM-allows-
execute mutants. Retain unsolved-stub rejection and GCC debug/optimized plus
Clang address/undefined sanitizer checks via `tools/run_b06_walker.py`.
D5 bounds runtime to three PTE reads and constant local storage; tables consume
32KiB plus masks independently of each query. D6 compares the actual selected
AArch64 guest:39-bit low virtual region,4KiB granule, three levels rooted at L1,
4KiB terminal mapping, descriptor type/AP/AF/attribute controls and explicit
barrier/TLBI sequence. Both guests remap one VA from17 to29 and reject a write
then an invalid-leaf read, but the descriptor bits and fault records differ.

The comparison must identify unresolved full-manual register details and avoid
claiming equivalent SUM/MXR controls, canonical-address rules, memory types,
permissions or cache-maintenance semantics. `abi_guest_validation.json` supplies
actual selected guest observations; `walker_validation.json` supplies hosted
model evidence. Neither substitutes for the other.

## Assessment, hints and transfer

`.a` separately assesses the fault/offset derivation, `.b` all boundaries and
bounded implementation, `.c` the regime comparison and omitted assumptions.
Level0 confuses host pointers with modeled addresses; level1 matches a single
mapping; level2 satisfies each explicit invariant and negative control; level3
solves AP6-DELAYED unaided after seven days: add serialized software fault repair
for A/D with an explicit retry/revalidation protocol, then show why concurrent
hardware PTE updates require a different atomicity argument. Do not alter the
original immutable-model result to pretend it tested concurrent updates.
Hints: separate table access from leaf permission; derive leaf offset width from
its level; prove that every rejection precedes output publication. Assessor
analysis is `assessor/architecture_ports/targets/WALKER_ANALYSIS.md`. Use only
synthetic tables and disposable guests. No operational memory probing, physical
measurements or learner mastery follows from these finite author experiments.
