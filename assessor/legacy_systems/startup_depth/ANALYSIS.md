# T10 assessor analysis and transfer package

Withhold this directory. The original criterion IDs and minimum7-day variants
remain the assessment keys. `core.py`, `entry.py` and `runtime.py` contain separate
observable claims for all thirty ordinary and thirty changed requirements.
They are author reference executions, not independent learner evidence.

## T10.01–.03: what starts, and what is preserved

The auxiliary-vector application entry and first executed PC are distinct in
interpreter-backed images. Require actual PC/map evidence, not a diagram inferred
solely from e_entry. Static PIE is an especially useful counterexample: no
interpreter does not imply no relocations. Its own startup can reach
`_dl_relocate_static_pie`; the custom counterpart without that consumer must not
dereference retained unrelocated pointers. Assigning one original pointer slot is
a valid bounded repair but fails a claim to support arbitrary dynamic relocations.

The96-byte fixture has argv offset8, envp32, auxv48. Under the32-bit word model
the same words occupy48 bytes, with corresponding4,16,24 offsets. Synthetic labels
need a separate mapping; real pointers cannot be recovered by assuming adjacent
strings. Every truncation of the constructed table should fail before publication.
Reject implementations that use native unaligned integer loads or leave output
half-written. The original process pointer survives stack realignment; a helper
call adds a return address below the aligned call stack. The over-aligned by-value
aggregate is an ABI exercise, not evidence that arbitrary C objects need32 alignment.

The installed CRT sequence is selected by the driver and the selected library.
Require hashes and symbols: a file named start.S can implement a completely different
contract. Modern glibc can run init arrays without a DT_INIT fragment, so “remove
crti means link error” is an invalid universal explanation. The discriminating
original .init fragment sets a marker; omitting crti loses its callable entry and
the marker check fails97 after a successful link. Explain why init-array tests
alone could miss that responsibility. Either diagnose a real first missing
responsibility or prove this particular removed responsibility was unused.

## T10.04–.06: prerequisites, termination and register translation

Trace the actual inspected bodies, distinguishing calls from full-body inspection.
Musl's environment/auxv work precedes TLS/guard and initialization/main stages.
Glibc's static path relocates before normal data/function usage, then sets up TLS
and other runtime state. The proposed errno provider contains an fs-relative TLS
access. A resolved archive member does not prove fs points to a valid thread object.
The rejected proposal is not executed. Caller-owned `sd_result` has a narrower
contract with no global errno, allocator, TLS or cancellation service.

The two raw-descriptor handler markers are B then A, followed by the buffered X
on the observed normal exit. The static buffer outlives main: a local buffer would
introduce an unrelated lifetime defect at return. `_Exit` skips registered handlers;
its no-flush result is a named implementation observation. The worker gate remains
closed for exit_group and is released for raw thread exit. The worker then exits
the whole original process. Do not claim scheduler ordering beyond the observed
survival/property. ISO main return does not authorize ret at machine entry.

At `sd_raw6` entry, C puts number,a,b,c,d,e in rdi,rsi,rdx,rcx,r8,r9 and f at
rsp+8. Move number to rax before overwriting rdi, shift first arguments and put d
in r10. The kernel's rcx/r11 clobbers are caller-saved at this assembly-function
boundary. Callee-saved registers and the red-zone/stack contract remain ordinary
ABI obligations. Memory effects are opaque to separately compiled callers; a
different inline-assembly solution needs a correct memory clobber and operands.
Mmap's real anonymous flags and a file-backed nonzero offset discriminate missing
fourth/sixth arguments. Do not count assembly text matching as execution on Arm64.
Linux64 errors occupy unsigned words corresponding to -4095..-1; -4096 is not an
errno word. Whether another operation can return a given address is a separate
contract. Check classification without dereferencing made-up addresses.

## T10.07–.09: progress, closure and bounded cost

Expected scripted submitted offsets are0,2,2,5. Keep errors and progress separate;
the resend mutant must compile and fail the prefix/suffix assertion. EAGAIN owns
no new bytes and preserves the pending borrower; cancellation cannot roll back
bytes already visible to a pipe peer. Zero read/write differ. The real frame run
forces a4096-byte pipe, fills it, checks blocked offset0, drains the old fill,
then resumes the same12292-byte frame through partial transfers. Verify received
bytes independently, including its four-byte big-endian length, not only offset.
A different correct implementation may offer caller-selected budgets/deadlines
provided it meets the contract and proves a finite stop.

Parser multiplication must be checked before evaluation; signed magnitude handles
2147483648 without overflowing negation. Public cases and two compiler oracle
bindings cover65536 values plus extrema, invalid spans, short capacities and signed
limits. Finite enumeration is not a proof of all inputs. Require the invariant
and bound as well. Powers-of-ten subtraction performs at most90 subtractions for
ten digits, with fixed table storage; division emits target-dependent instructions
or helpers. An image-size difference alone is not a latency result. Cost artifacts
separate parser, formatter and alternative increments at O0/O2 and retain symbols,
section sizes and stack-usage records; no speed ordering is required.

`nm -u` is only the unresolved set. The original static archive supplies
sd_disallowed_service, so final unresolved output is empty while its code and map
origin remain visible. Repeat after LTO, inspecting the final executable: slim
LTO object disassembly cannot stand in for generated machine code. A conforming
closure audit must reject the provider for its declared origin/call, not celebrate
the successful link. Arm core objects have no helper calls in the measured build;
that is object evidence, not proof of a complete firmware runtime.

## T10.10: independent diagnosis

Wrong saved pointer yields71 before table reads; bad alignment yields72 before
assuming the C bridge contract. The specimen records stack values in assembly
and the simple helper checks them before other work; this deliberately flawed
bridge is not a portable C executable. For static PIE the unrelocated pointer
check yields86; adding alignment failure yields72, then fixing alignment reveals86.
The ROM-copy model fails74 with omitted setup and passes with the copy. Its label
must stay “hosted initialization model,” not a board observation or a claim that
Linux forgot to initialize ordinary ELF data. Retain the actual original binaries
before reduction, their exact linked entry, symbols/maps and GDB trace. A crash
or an unrelated missing symbol does not satisfy these discriminators.

For every original delayed variant give only its problem statement, allowed
primitives, original header, target requirements and fresh synthetic inputs. Keep
this analysis and reference code withheld. Require causal predictions before the
assessor reveals outputs; vary the library/ABI/lifetime/initialization assumptions
specified in the existing cards rather than merely changing constants. Record
actual attempt dates, prerequisites and assistance. Today's author runs close
implementation/author-validation dimensions only.
