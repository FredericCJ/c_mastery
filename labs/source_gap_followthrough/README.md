# Named source boundaries and optimized queue inspection

The two legacy source-gap records describe a particular inspection wave. They
do not erase later, bounded source inspections. For each current assignment,
`research/v5/expansion/source_gap_followthrough/assigned_inspection.json` binds
the exact locator to the inspected registry record, its original date and its
limits. This is a source audit, not an assertion that every book was read or
every implementation was proved.

The renewed API passages distinguish three obligations:

* Modern C 10.1 puts consumer obligations in the interface and explanatory
  choices in implementation documentation. Its C23 attribute examples do not
  change this curriculum's C17 baseline.
* Hanson's Atom interface owns an immutable interned copy for the client
  execution; pointer equality works only within that identity discipline.
  Its implementation uses a bucket table and a length-qualified byte search.
  Arena 6.1 describes group deallocation, distinct creation/allocation errors
  and retained control lifetime. Historical assertions, signed sizes and
  alignment claims still need the modern contract review.
* K&R 5.11 explains callback dispatch, but its cast between incompatible
  callback signatures supplies no C17 call guarantee. Use an exactly typed
  wrapper; copying a representation or casting a function pointer is not
  adaptation.

[Parnas's two KWIC decompositions](https://www.cs.lafayette.edu/~gexia/cs301/resources/parnas.html)
can use the same algorithms while assigning different responsibility for
representation and timing decisions. A proposed interface should identify a
concrete change that remains private. The digitization's accuracy warning
remains in force. [Dijkstra's program-family passage](https://www.cs.utexas.edu/~EWD/transcriptions/EWD02xx/EWD249/EWD249.html)
connects that exercise to shared refinements and isolated variation; it does
not justify making every interface arbitrarily general.

[Doxygen's precondition command](https://www.doxygen.nl/manual/commands.html#cmdpre)
formats text. Neither successful rendering nor an annotation checks that a
caller satisfies the condition.

The renewed [Multiboot2 specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html)
inspection covers 3.3 and 3.6.1/3.6.2/3.6.6/3.6.8. The image creates its own
stack, GDT and IDT; entry registers are only defined where the contract says
so. Tag size excludes padding; traversal respects eight-byte alignment and
the terminating tag. The memory map can describe kernel, module and boot
information ranges as available RAM: the image must reserve those live ranges.
The captured memory-map prose contains a stray `size` sentence that disagrees
with its illustrated entry; retain the declared entry-size/stride contract
and reject malformed input rather than guessing a different wire layout.

For the queue experiment, run from the repository's supported Linux runner:

```text
python3 tools/run_genmc_ir_validation.py --report NEW_REPORT.json
```

The campaign includes the unchanged original queue and finite harness. It
uses RC11 exhaustive verification with race detection enabled, capacity two
(one spare slot), one producer and one consumer, three attempted operations
each and a final drain. Both `-O0` and **`-O1 -fno-builtin`** enumerate 20
complete executions. Early publication, relaxed publication and relaxed reuse
controls each produce a non-atomic race in both profiles. LLVM verifies each
input and transformed IR file.

`-fno-builtin` is a declared compiler optimization choice. It avoids this
tool's failing promotion path; it does not change a memory order, remove an
assertion, rewrite the queue or disable race detection. Default `-O1` still
terminates by signal 4 during GenMC memory-intrinsic promotion. That receipt
is retained and receives no verification credit. The result does not cover
arbitrary optimized IR, a whole service/frame translation unit, target timing,
liveness, all C executions or an independent learner's work.

The integrator's separately inspected successor records now bind Effective C's
named leak passage and actual POSIX Issue7/Issue8 close and read-lock passages.
See `labs/source_readings/realloc_ownership.md` and
`labs/posix_edition/CONTRACTS.md`. The former distinguishes destructive cleanup
from recoverable resize; the latter keeps ownership retirement separate from
diagnostics and distinguishes scheduling premises by edition. Their named
source locators and original inspection dates are retained in this audit.
The historical access failures remain historical. A pure contract classifier
does not establish a real OS's behavior. Exact published ISO C17/C23 access
remains distinct from the inspected drafts and these teaching passages.
