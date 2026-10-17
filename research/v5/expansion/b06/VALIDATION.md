# B06 author validation and remaining scope

Canonical additions comprise eight domains,64 projects,192 objective leaves,
128 authored units,64 independently derived fixtures,128 glossary entries,
64 recall sets with192 variants, eight learner scaffold families and separate
assessor implementations. Accepted baseline records and B01 receipts were not
edited by this batch. New descriptive hours do not cap learner effort.

The audit began from the planned T57–T64 requirements in planning/overhaul_v5.md,
the authoring contract, current family/unit/recall tooling and actual primary
passages. Source records are in research/v5/b06_resources.json. Exact bounded
inspection claims and gaps are retained; bibliographic discovery is not promoted
to inspection. The domain-scoped research gate pins that register and identifies
author inspection, not independent human acceptance.

## Exact observed checks

- `.conda/python.exe tools/check_b06_authoring.py`: passed; eight domain
  structures,192 objective mappings,128 rubric-bearing units, references,
  prerequisites, fixtures, glossary authority/recall and global term uniqueness.
- `.conda/python.exe tools/verify_b06_fixtures.py`: passed64 independent fixture
  calculations. The oracle derives expected values from inputs, separately from
  executing the C implementations.
- `wsl -d Ubuntu --cd /mnt/f/Learning/Curricula/c_mastery python3
  tools/run_b06_campaign.py`: passed675 command/result checks, recorded in
  core_validation.json. Profiles: GCC debug, GCC release, Clang UBSan. Each
  profile executes64 reference anchors and64 semantic mutants. Every mutant
  must compile and fail its intended B06_CHECK assertion; a compilation failure
  or sanitizer crash does not qualify. All64 untouched learner stubs compile
  and fail their corresponding semantic anchor. Builds and compiler versions
  account for the remaining command checks.
- `wsl -d Ubuntu --cd /mnt/f/Learning/Curricula/c_mastery python3
  tools/run_b06_experiments.py`: passed85 checks, recorded in target_validation.json.
  Eight expression trees with32 inputs each agree across an independent Python
  denotation, interpreter, emitted x86 and independently compiled unsigned C.
  The same256 cases execute emitted Thumb functions in QEMU mps2-an386. Final
  Thumb images have no undefined helper symbols. This is the declared expression
  notation only; unsigned suffixes in corresponding C are explicit.
- The target campaign also observes10,000 deterministic-seed semantic fuzz runs,
  5,461 event words through length6,17 journal byte-prefix cut states,68 vector
  lengths, emitted x86 packed arithmetic and a fast-math counterexample. Original
  local ASan targets demonstrate bounds, lifetime and double-release findings;
  the repaired owner passes. A bounded format example changes only its supplied
  sentinel and the repair preserves it. Local epoll/eventfd/timerfd/signalfd,
  fsync and sendfile observations are recorded. io_uring setup is an availability
  probe, never a cancellation or submission success claim.
- Both current campaign input hash maps were checked against the final files:
  no stale paths. The earlier isolated compiler-edit build failure was repaired
  and the focused compiler rerun passed87 checks before the final full campaign.

## Claim limits and required continuation

coverage.json maps source/topic to every objective, project, learner artifact,
assessment criterion and delayed variant. gap_ledger.json records77 required
items:64 delayed transfer assessments, eight domain-specific target/depth
obligations and five cross-domain source/implementation/performance/physical
gaps. remaining_work.json keeps comprehensive_complete false.

RV64/AArch64 context code was cross-assembled and checked for undefined helpers;
it was not executed. The kernel module was neither loaded nor tested in a
disposable matching guest. CBMC is absent, so its authored harness has no proof
result. Required compiler control-flow, local-variable, pointer and call
extensions are specified but lack reference implementations in this batch.
io_uring cancellation, splice, hostile ABI contexts, full reset/trap/translation,
real flash fault patterns, timing/resource comparisons and physical F439/H745
captures remain required. None is displaced into an optional appendix.

These are author executions, not learner acceptance, independent review,
physical validation, measured teaching effectiveness or absolute mastery.
Root integration/publication checks and the final graph rebuild are separate.
