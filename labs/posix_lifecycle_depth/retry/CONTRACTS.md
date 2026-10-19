# Uncertain replies and bounded durable deduplication

## Identifier and families

Required successor for `AX-PR-T23-02-T23.04.c` and its existing seven-day
`-DELAYED` variant; B/C/D/F/G/J. Original units, IDs, assessment prose and
historical acceptance remain unchanged.

## Objective and inspected sources

Refute "a failed response proves no effect occurred". The actual inspected
course sources are `receipt.h`, the separate reference implementation,
`receipt_faults.c`, and the **Durable effect and receipt transaction** section
of `labs/service_depth/adapters/CONTRACTS.md`. Their source-bound campaigns
provide finite implementation evidence. The original Linux write passage
remains its original source; no unseen distributed-systems theorem or published
standard inspection is claimed here.

## Prerequisites and artifacts

Bring the PR-T23-02 byte/error dossier, completed `store.h` implementation and
the explicit service `receipt.h` implementation. The public `receipt.c` remains
unsolved. Explain transactions, result-cache expiry, signed overflow and
serialized ownership before running the campaign. The original storage
contract remains authoritative for `store_commit` and indeterminate outcomes.

## Environment and dependency contract

Hosted C17 and an owned synthetic in-memory device. Import learner receipt/store
sources explicitly through `run_retry.py`; the runner never searches assessor
directories. Standard C I/O is only host evidence machinery. The imported
service transaction is externally serialized. Six GCC/Clang profiles assess
hosted execution; separately linked Cortex-M and QEMU reports retain their own
precise limits, not physical durability claims.

## Concrete problem and inputs

First increment a local counter by7, discard its reply, then blindly repeat:
the effect becomes14. Implement the existing two-client receipt API. A fresh
store provisions epoch1, three receipts per client and a durable high-water
request ID. Apply client1/ID5/+7, discard the result, reopen and retry; require7.
Then run client2/ID9/+3, client1/ID10/-2, ID20/+1 and ID30/+1: results10,8,9,10.
Old ID5 must be stale after eviction; client2 ID9 still returns its recorded10.
Changed payload for a retained ID conflicts; another epoch is stale.

For the delayed variant, interrupt every write/flush boundary while processing
ID5. The public device model either discards unflushed state or persists a flush
before reporting failure. Reopen the server and retry. Compare with a flawed
effect-then-receipt two-transaction implementation, explaining its missing
atomicity. Do not repair the supplied negative control.

## Necessary distinction

Byte transport retry, application retry and durable effect recovery have
different contracts. A lost reply may follow a committed effect. A bounded
cache alone cannot return every old result forever; eviction requires a stated
policy. This contract rejects evicted IDs rather than executing them again.
The ID is bound to the complete signed32 delta, not merely a hash.

## Invariants, ownership, failure and cost

Use the explicit shared service transaction contract: effect, receipt and
high-water/cursor publish atomically. The sole owner serializes access. On
indeterminate commit, leave output unchanged, reopen and inspect durable
identity before deciding what to do. Successful duplicates return their saved
result. Overflow/conflict/stale rejection preserves output. Fixed two-client
and three-entry bounds make lookup bounded; store I/O costs remain measured
transaction work. The model owns separate stable/staged images and resets only
its own state; successful flush persists all staged pages. It is a declared
device model, not a claim about hardware flush atomicity or adversarial media.

## Deliverables

Submit the blind-retry trace, reference state/receipt table, all cut outcomes,
reopen/retry traces and the minimized split-transaction counterexample. Record
the output sentinel before failed calls, expiry/epoch behavior and exact source
imports. Identify the transaction that must contain both the effect and receipt.

## Verification and pass/fail

Run `python retry/run_retry.py --receipt-source PATH --receipt-include DIR
--store-source PATH --store-include DIR`. A correct implementation prints
`PR_PASS`, with blind effect14 and final repaired effect10. The current reference
operation has37 boundaries and the two public models yield74 cuts; derive and
report actual count if the implementation changes. Every cut must reopen to
0or7 and retry to exactly7. The author campaign separately requires successful
compilation then a `PL_CHECK:` failure for reapply, split receipt, payload-binding
and unsolved receipt controls; crashes/compiler failures do not count.

## Rubric

Level0 violates the effect/ownership contract. Level1 repeats ordinary replies
without uncertainty or expiry reasoning. Level2 needs blind14 versus repaired7,
payload binding, stale eviction and sentinel-preserving errors. Level3 requires
the original independently attempted delayed crash variant, its missing
atomicity argument and evidence; author tests never award learner level3.

## Hints and assessor separation

Track response delivery separately from state commitment. Next place the effect
and receipt in one recovery invariant. Finally ask what an old ID means after
its result has been evicted. Private causal analysis resides under
`assessor/posix_lifecycle_depth/retry`; no completed code is exported by this
supplement.

## Changed conditions and composition

The named delayed variant is executable through every model cut. Existing
service evidence separately explores185 five-mode cuts,18,176 sector patterns
per release profile and152 write/recovery guest executions. Those receipts
are reused only after exact source/hash validation; they are not newly rerun by
this harness. Physical power loss, arbitrary malicious corruption, epoch
migration, distributed concurrent writers and indefinite result retention
require distinct contracts.

## Containment

Only an original teaching service, synthetic counters and owned memory are
used. An adversarial local client may retry an ID or change its payload. The
harmless violation is duplicate counter effect or a conflicting request accepted
under the old ID. Repair is atomic effect/receipt publication, full payload
binding and stale-ID rejection. Regression checks cover all of these; they do
not authenticate clients or establish arbitrary exactly-once delivery. No
external server, real credentials, deployed storage or host settings are touched.

## Validation and evidence limits

Reference execution, finite device models, source-bound existing VM observations,
external review, learner work and physical validation remain separate. The
actual campaign records counts and compiler/source hashes. No passing finite
suite establishes all recovery histories or absolute mastery.
