# Private T46.10 assessor analysis

Do not copy this file, oracle or implementation into learner exports. Use the
unchanged criterion and delayed IDs from PR-T46-10. Require all three objective
arguments; a store smoke test is not a capability or reconciliation assessment.

## 10a

CVM_HOST validates integer, budget and callback; vh_host separately validates its
borrowed config, ID range, mask and actual implemented service. Denied budget/tag
never reaches callback. Policy denial may consume the interpreter's host unit but
must not touch database state. Allowing unknown31 still returns failure; shifting
255 would be UB and is avoided by short-circuit range validation. Absent host is
CVM_HOST_ERROR. The two VMs are actual independent Cvm instances configured through
the same helper with different Vh masks, sequentially sharing one opened store.
A forged native pointer remains outside guest isolation. Mutant1 executes the
forbidden assignment and must fail the reader test at a semantic assertion, not
at compilation. Further checks include denied read counters, zero host budget,
boolean operand, active transaction remaining owned, null host and missing service.

Rubric0: changed failure output or unauthorized side effect;1: nominal allowlist
only;2: explains first-refusal order and distinguishes guest policy/native trust;
3: independently derives two-instance rights and demonstrates no forbidden backend
read/write. At seven days issue same actual providers with swapped rights and an
absent enabled service; retain the original delayed task and learner authorship.
Alternative: table of service descriptors containing permission and function,
provided ID validation precedes indexing and lifetime remains explicit.

## 10b

Unsigned encoding is modulo2^32, while decoding high values uses -1-(MAX-u), whose
signed intermediate stays in range. The accepted adapter uses widened int64 for
its equivalent high-half expression; both avoid implementation-defined cast to
int32. Endianness in the byte format is independent of native endianness. A wrong
length is not a short valid value. Versioned reads preserve output on refusal;
allow_legacy permits a read but only explicit vh_migrate rewrites. Repeating valid
version1 migration does not commit. The helper is native privileged, not a guest
service implicitly admitted by the mask. Mutant2 ignores version/length and reads
zero-initialized bounded storage, so its failed assertion is a defined semantic
counterexample rather than an incidental out-of-bounds crash.

Rubric0: unportable bytes or unknown-version acceptance;1: roundtrip one integer;
2: derive all byte/length/refusal cases plus exact actual adapter composition;
3: independently defend migration/rejection policy under an old reader/new writer.
Alternative migration may use a new key, but must explicitly change the contract,
prove atomic compatibility and test the transition. The supplied changed format
requires version1 at the same key; silently using the old four-byte reader fails.
Delayed package asks the learner to perform that actual transition without this
analysis, preserve old input bytes and compare with a rejected unknown version.

## 10c

The test medium charges only writes/flushes, not reads. Events1/2 invalidate a
prior marker,3..34 stage the32 redo images,35flushes them,36writes the commit
marker,37flushes it. Discard mode fails all37 before new publication. Persist-all
failure at36/37 can retain the new marker and images; all other cuts recover old.
Thus74 cuts yield72old and2new, independently derived from the protocol order.
Both results originally report HOST_ERROR; output91 does not reveal persistence.
Assignment reconciliation reads after recovery. Increment reconciliation writes
value and id/delta/result in the same transaction; duplicate returns saved result,
new sequential ID applies once, changed/old/gap/zero/overflow refuse. The protocol
needs one client retaining its logical ID, serial exclusive store access and a
valid recovered store. Do not claim exactly-once external effects: only the
transactionally co-stored synthetic effect is covered.

The private Python oracle enumerates648 histories of three requests, with initial
0/MAX/MIN and fixed ID0/1/2 and delta±1. It models only a mathematical value and
latest accepted tuple, independently of codecs/WAL/provider code.1944 steps compare
status, failure output, persistent value and receipt after reopening/checking the
actual store. Public tests additionally cover full64-bit ID exhaustion, changed
payload, duplicate, older/gap, and all74 crash cuts. Mutant3 repeats a duplicate;
mutant4 suppresses commit error;mutant5 omits the receipt. Each compiles and then
fails the intended visible assertion. All host profiles execute the independent
oracle; target/emulated big-endian runs execute the full public failure-cut set.

Rubric0: retry doubles an effect or acknowledges failed commit;1: correct ordinary
assignment but assumes every error rolled back;2: derives both uncertainty cases
and proves receipt/value atomicity with actual recovery tests;3: independently
implements the increment protocol and identifies bounded-history/client limits.
Alternative: a durable set of id/payload/result records supports longer replay
windows but requires a specified retention/space policy and atomic coupling.
At minimum seven days withhold code/analysis, ask the exact changed increment
variant and a counterexample to unlimited exactly-once claims; author execution
of the package is not that learner assessment.

## Resource and target analysis

The medium uses2*67*4096bytes plus flags/counters; public tests keep two medium
objects for crash replay, totaling about1.1MiB before store/VM state. Hence the
explicit separate emulator SRAM profile. The stack audit reads actual .su data,
charges33 copies of retained insert/refresh/check_node depth-guarded recursive
frames and all other live C frames once, plus2048 assembly allowance; callbacks
are the supplied nonreentrant medium, interrupts masked. This conservative
selected-program bound is neither general call-graph proof nor physical watermark.
The actual CVM uses explicit guest frames. No helper archive may enter final link;
if a compiler emits a helper, add a reviewed original implementation or report
closure failure. Native sanitizers do not establish target stack sufficiency.

Source register: `research/v5/expansion/legacy_vm_services/resources.json` names
actual inspected local provider functions and QEMU mapping. C17 published wording
and physical board claims are not invented. Pass only complete executed campaign,
negative-control guard and isolated public export tests; keep reviewer/learner/
physical dimensions pending until their own evidence exists.
