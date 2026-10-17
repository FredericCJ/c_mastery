# B05 inspection and depth decisions

Actual primary passages read on 2026-10-17 include SQLite Atomic Commit section2
hardware assumptions and Linux What Is RCU descriptions of reader critical
sections, pre-existing-reader grace periods, publication ordering and update-side
rate pressure. Document-byte hashes are recorded in resources.json. SQLite's
article describes rollback-mode assumptions; the new original prefix-recovery
model is not presented as SQLite WAL. Linux RCU semantics are distinguished from
ordinary serialized C fields. RFC1982 passages additionally support timer-distance
limits; no finite counter can reveal an entirely unobserved wrap.

Substantively inspected existing mechanisms:

- `labs/runtime/include/cvm.h`: typed values, bounded verification/execution and
  collector budgets with callbacks. Retain the accepted CVM. X-T46-001 isolates
  join consistency and limits of structural acceptance; it does not claim the
  earlier runtime lacked types or budgets.
- `labs/storage/include/store.h`: complete-page I/O/error callbacks, possible
  partial persistence, ordered durability after successful flush and explicit
  STORE_INDETERMINATE. Retain these stronger integration contracts; the new
  single-value log does not replace full-page CST2 redo.
- `labs/kernel_smp/tests/contracts.c`: real boot, frame, ELF/archive, handle and
  numeric user-range tests. Range arithmetic alone does not assess page-by-page
  permissions or preflight atomicity; X-T48-001 adds that bounded mechanism.
  These tests alone do not establish a native TLB-shootdown experiment.
- `labs/execution/include/cmx.h`, README and tests/contracts.c: saved-mask/lock
  serialization, timers, synchronization, response-time analysis, SPSC, hazard
  and epoch machinery. Host stress and target guests already exist. Preserve
  this work; new timer/RTA/grace/request exercises add explicit independent
  counterexamples, not claims that the previous mechanisms were absent.
- `labs/os_observatory/SPEC.md` and tests/test_os.c: concrete scheduler traces,
  CLOCK-versus-FIFO discrimination, shared-descriptor/process/thread/I/O probes.
  The spec explicitly asks for an offline optimal-replacement oracle, but the
  inspected CLOCK test does not implement that oracle. This is a concrete next
  extension, distinct from the already executable probes.

The390-leaf structural inventory does not mark unsampled objectives adequate.
The first seven exercises assess16 bounded claims; full native target integration,
independent human assessment, delayed learner transfer and remaining deeper
topic reviews remain open. Accepted baseline identities/hours/receipts are intact.

Physical H745 cached/DMA/restart experiments remain unperformed. The original
noncacheable cold-boot target evidence is not relabeled as completion of them.
This authoring task did not change host security settings, flash hardware or
interact with deployed devices.
