# T25.06–T25.09 implementation inventory

This is an in-progress author work record, not a completed disposition.
Inspected the actual PR-T25-03 brief and all12 original criterion/changed-task
objects on2026-10-17. The parent is profile C: target code retains a freestanding
closure, while host orchestration/thread/sanitizer tools remain separate.
The exact original IDs are `AX-PR-T25-03-T25.{06,07,08,09}.{a,b,c}` and their
unchanged `-DELAYED` variants. No learner interval or independent review is
implied by executing an assessor reference.

Required distinct mechanisms, in dependency order:

1. Publication/read-from graphs, initial-read nonpublication, conflict and
   adjacent-bit-field classification, actual race-free compound-state failure,
   and a bounded snapshot/reuse protocol. An execution graph is not merely a
   schedule permutation; relaxed store-buffering outcomes need separate rules.
2. Real atomic initialization/operation/aggregate experiments, per-object
   lock-free queries, valid alignment/ABI changes, foreign-target emitted
   instructions and undefined-helper closure. No host query stands for a
   foreign target result; no undeclared helper library is added to the target.
3. Relaxed/release-acquire/SC and mixed-order bounded graph enumeration,
   reusable single-slot forward and reverse handoff, two-reader retirement,
   explicit fence communication and dependency versus control-branch analysis.
4. Every success/failure CAS-order pair, scripted weak spurious/mismatch/success
   traces, strong one-attempt comparison, and heap-node ownership/lifetime under
   failed publication. A newly observed pointer is not permission to dereference
   retired storage.

Primary passages actually read so far:

* N1570 C11 committee draft:3.14 memory-location definition/example;
  5.1.2.4 paragraphs1–22 as exposed by the passage reads;7.17.2.2 initialization;
  7.17.3 paragraphs1–10;7.17.3.1;7.17.4 paragraphs1–4 and7.17.4.1/.2;
  7.17.5 and7.17.5.1;7.17.7.1/.2/.3/.4 including weak failure and expected update.
  These are draft clauses, not newly inspected final C17 text.
* N3096 April2023 working draft:7.17.2.2 paragraphs2–6;
  7.17.3 paragraphs1–11;7.17.3.1 and7.17.4 through7.17.4.2.
  Its cover and status are retained as draft evidence, not published C23.
* GCC current `__atomic` builtins manual: memory-order descriptions, consume
  implementation as acquire, target lowering/external routine fallback,
  runtime-order handling, load/store/exchange operation sections. This compiler
  behavior does not define ISO consume semantics for other implementations.

Exact final C17 and published C23 access remains an explicit partial for
T25.08.c's source-comparison obligation; executable/compiler and draft-based
analysis can proceed independently. Do not erase that part or substitute the
name of a final edition for an inspected draft. Existing T25.03 snapshot and
predicate packages may be imported only through their actual frozen contracts
and source-bound evidence, with the changed bounded-retry requirement assessed
separately. No current implementation/validation closure is claimed here.
