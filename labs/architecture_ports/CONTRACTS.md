# B06 common exercise and evidence contract

This contract applies to every PR-T57–PR-T64 project and its LAB unit. Each
project supplies the mechanism-specific inputs, API, invariant, boundary and
changed-condition assessment; these obligations supplement its public tests.

## Prerequisites and permitted dependencies

Use the accepted C17 implementation baseline. C11 draft clauses supply precisely
identified semantic reasoning; a draft is not a substitute claim that published
C17 was inspected. C23 comparison work must identify the actual changed clause
and compiler support; unchanged ABI or OS behavior is not an ISO C guarantee.
Before entry demonstrate checked extent arithmetic, object lifetime, byte
decoding, resource ownership and reproducible build capture with the declared
prerequisite artifacts. Reading a module does not accept an artifact.

The supplied portable cores use only C17 types and explicitly declared host
math operations. Implement the mechanism from these primitives: no allocator,
compiler, container, verification engine or protocol library may replace the
mechanism being taught. Host-side Python/CMake/CTest are verification tools.
Kernel headers, system calls and ISA assembly belong to separately named
adapters. A hosted state model does not demonstrate kernel scheduling, device
ordering, cache coherency or flash persistence. For each freestanding adapter
compile with -ffreestanding, inspect relocation and undefined-symbol tables,
linker maps and disassembly for compiler helpers; report every dependency and
provide or remove it. Merely disabling builtins does not prove runtime closure.

## Execution and containment

Run host tests in a disposable working directory using synthetic fixtures.
Kernel modules run only in a disposable VM with a snapshot, no host shared
directories, no bridged networking and no production data; do not insert them
into the workstation kernel. Firmware work uses the supplied model/emulator
first. Physical transfer is restricted to dedicated NUCLEO-F439ZI or
NUCLEO-H745ZI-Q development boards and requires a separate authorized session;
this authoring task does not flash devices or change security settings.
Security cases target only the original teaching arena/dispatcher. The attacker
controls the explicitly supplied bytes, handles or event schedule, not real
credentials. Harmless success is the named sentinel/property transition.
Crash, undefined behavior and sanitizer detection are different observations;
none alone demonstrates code execution or privilege escalation.

## Deliverables and verification

Retain (D1) a prediction before execution, pre/postconditions, representation,
invariants, ownership transitions, failure atomicity and cost derivation;
(D2) learner source and build identity; (D3) complete commands, raw outputs,
tool versions and independently derived expected results; (D4) a minimized
counterexample, repair and regression; (D5) a changed-condition implementation
and comparison; (D6) an evidence-limit statement. Use supplied fixture data,
then independently construct at least one input that distinguishes your repair
from the named plausible wrong implementation. Public finite checks cannot
establish absence of all defects. Sanitizers observe executions; bounded proof
depends on assumptions/unwinding; measurements depend on workload and machine.

## Assessment rubric and hints

Every objective has its own criterion in the project and authored unit. Score
0 for no assessable artifact or a contradiction of its contract; 1 for a
passing demonstration without a causal explanation; 2 for an original
implementation/analysis with a correct invariant, boundary argument and
reproducible evidence; 3 for an unaided delayed changed-condition solution
that identifies which previous proof or measurements no longer apply.
Require all objectives at level2 before technical completion and level3 on the
specified transfer task after at least seven days before independent-transfer
completion. Record whether the attempt was self-reported or externally
reviewed. No fabricated reviewer independence, mastery certificate or absolute
mastery claim is permitted. Hints are graduated in each brief; consulting the
assessor solution changes the attempt to assisted practice.

## Validation states

Canonical writing means authored; a compiling scaffold means executable; a
source-bound observed run means executed. These do not imply learner acceptance,
external assessment, VM execution or physical validation. Each family records
remaining target evidence and each campaign reports exact commands. A model
pass cannot satisfy a missing mandatory adapter investigation. New descriptive
hours are separate from accepted baseline hours and impose no learner-time cap.


## 1: Stack extent and ABI alignment

Implement po_stack(top,bytes,out): require bytes<=top, subtract and round down to16-byte alignment; publish only on success. Treat integers as modeled addresses, not dereferenceable C pointers. Then build both assembly entry stubs with a private aligned stack and inspect the map.

Prove result<=top-bytes, alignment and nonwrap; account for consumed padding and live stack bounds.

## 2: Callee state and platform registers

Implement po_saved(arch,reg), with arch0 RV64 integer ABI and arch1 AArch64. Report saved integer registers: RV x2,x8,x9,x18..x27; Arm x19..x29 and synthetic register31 for SP. Other fields return0. Independently compare hostile-caller assembly against the declared set; exclude x18 from portable allocation.

Identify which owner saves each register and why the return-address register needs protection in nonleaf callees despite its caller-saved classification.

## 3: Argument register exhaustion

Implement po_arg(index,reg,offset) for the deliberately restricted nine-or-fewer uint64_t scalar interface: index0..7 maps to abstract argument registers0..7 with offset0; index8..15 maps to regUINT_MAX and stack offset8*(index-8); index>=16 rejects transactionally. The assembly adapter translates abstract registers to a0..a7 or x0..x7.

Derive the first stack argument relative to entry SP; distinguish entry-SP offsets from offsets after the callee allocates its frame.

## 4: Owned context images

Implement po_save(dst,arch,registers) copying only po_saved registers into a zero-filled32-slot image; reject arch>1 before mutation. po_restore validates image arch equals requested arch and copies only saved slots into a live register model, preserving others. Destination/source are disjoint. Extend assembly context switches using the same documented offsets.

Prove a save/restore preserves exactly the declared set and leaves nonowned registers untouched. State stack ownership, resume-address validity and exclusion of asynchronous FP/trap state.

## 5: Signed immediates and relocation ranges

Implement po_sign12(raw) for the low12 bits as a mathematical signed value -2048..2047, ignoring higher bits. Implement po_branch(delta,bits,align) accepting only bits2..30, align2 or4 and aligned signed delta in [-2^(bits-1),2^(bits-1)-align]. Use this as a relocation admission oracle, not as a complete instruction encoder.

Derive sign-extension using unsigned extraction and subtraction in a range representable by int. Separate field representability, target instruction alignment and linker relaxation.

## 6: Trap status versus call state

Implement po_trap(status,from_supervisor) and po_return(status): trap records spp, copies sie to spie and clears sie; return restores sie from spie, sets spie1 and clears spp. Inputs are boolean fields; invalid input rejects without mutation. Compare the explicit model to supervisor CSR traces in a dedicated RV guest.

Draw status before entry, after entry and after return. Explain which processor state is automatic, which is software saved and why a call-return instruction cannot replace a trap return.

## 7: Privilege-aware memory access

Implement po_access(user_page,supervisor,sum,mxr,r,w,x,operation), with boolean flags and op0 read,1 write,2 execute. User execution rejects supervisor-only pages. Supervisor execution always rejects user pages; supervisor reads/writes user pages require sum. Read requires r or mxr&&x; write requires w; execute requires x. This models the inspected SUM/MXR obligations, not a complete page walk.

Derive each access decision before dereferencing and identify omitted page-walk obligations: validity, canonical address, superpage alignment, access/dirty updates, translation synchronization and fault cause.

## 8: Link closure and changed-target defense

Implement po_closure(required_mask,provided_mask) returning true exactly when every required helper bit has an approved implementation. Bits represent memcpy, division, atomic fallback and stack protector for the synthetic map. Then derive the real closure from nm/readelf/map output for both ports and record every unresolved or policy-disallowed symbol.

Distinguish declarations, emitted references, linked providers and actually exercised target paths. Explain why hosted test libraries cannot be treated as target dependencies.