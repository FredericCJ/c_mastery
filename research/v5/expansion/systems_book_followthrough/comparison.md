# Bounded reinspection of the retained systems books

This versioned supplement adds a fresh author inspection. The original resource
records and their 2026-10-04 evidence remain unchanged. No book text, page image,
or extracted manuscript is distributed here. The exact library copies match the
retained source hashes; catalog search was only discovery, not passage evidence.

## CSAPP Global third edition

The inspected linking passages separate driver stages, symbol resolution and
relocation, then derive a concrete signed PC-relative displacement. These support
the actual T07 driver-stage and T09 relocation/loader investigations. The current
exercises must retain explicit machine, relocation type, addend, place, range and
endianness checks. The book's conceptual relocation structure is not a portable C
serialization layout, and its historical relocation count is not a current ABI
enumeration. Its section-oriented loading explanation is pedagogical; the actual
ELF loader consumes program headers. Neither `_start` nor a particular libc
startup sequence is an ISO C requirement. Shared objects and executables need
their actual ELF type and program headers checked; a filename suffix is
insufficient. The measured modern toolchain and runtime sources remain the
authority for current behavior. The Global edition pagination was verified;
US-edition page numbers were not substituted.

## TLPI 2010

The inspected passages distinguish return-value and `errno` contracts, explain
mutex cooperation and condition-variable predicate rechecking, and distinguish
private/shared mappings from immediate persistence. These support the current
T23 mapping and T25 lifecycle/waiting successors. The unsynchronized increment
example explains one machine-level lost-update history; it does not bound the
behavior of an ISO C data race. A timed wait is not a service deadline or a bound
on mutex reacquisition. A notification stores no application predicate, and an
unlock-before-signal optimization still requires the condition object to remain
live. The historical `mmap` example terminates on a short write; it is not a
recovering copy loop, empty-file policy or durable-write protocol. Current
Linux/POSIX contracts and the selected C implementation must supply those
obligations. Historical glibc maintainers, versions and paths are not asserted as
current facts.

## Evidence limits

Only the exact registered passages and title/imprint pages were inspected using
private text extraction. Diagram layout was not visually assessed. This closes
the newly-inspected status for these two named assignments only; it does not
assert inspection of the full books or final ISO C17/C23 text. The exact reading
scope audit covers all five T07--T11 modules, their 24 project briefs and 24
supplemental cards. It finds one assigned book resource, CSAPP in T09, with the
exact relocation locator now inspected. TLPI is additionally retained because
the original gap names it; all its registered locators above were read. No other
book assignment occurs in these 53 canonical inputs. Prerequisite readings from
earlier modules remain governed by their own records; an undefined phrase
"other assigned books" does not create a new whole-library assignment.
It adds no execution, independent review, learner or physical
observation claim. Existing mechanism campaigns remain separately source-bound.
