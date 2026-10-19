# VT8: an unfamiliar transactional packet audit

Required original G-B06-T61-08-TRANSFER, parent PR-T61-08 and T61.08.a/b/c;
families A/C/D/E/F/G/I/J. Use `curriculum/v5/contracts/b06.md` for the14-field
shared obligations, D1–D6, evidence status and seven-day delayed assessment.
Prerequisites are the learner's bounded proof, fuzz, streaming and shrinker
artifacts from PR-T61-01–07. Do not inspect assessor code or a worked harness
before recording this independent attempt; doing so makes it assisted practice.

## New target and complete specification

Audit `audit_target.c`, an original intentionally flawed hosted C17 program with
synthetic four-key state. The caller owns a live AuditState and nonoverlapping
immutable input bytes; neither may be changed concurrently. `present` uses only
four low bits. Packet header is B6, version1, count0..4. Exactly count records
follow, with no trailing bytes: tag1,length2,key,value sets a key; tag2,length1,key
deletes a key and clears its value byte. Keys are0..3 and may occur at most once
in a packet. All lengths, tags, keys and exact extent must validate before state
is published. Return1 and commit all records on acceptance; return0 and preserve
every state byte on any rejection. Empty valid packets succeed unchanged.
Input lengths may exceed the maximum valid19 bytes and must still be rejected
safely. No heap, parser library or target security setting is involved.

Choose verification methods from mechanisms before implementation. D1 states
pre/postconditions, parser loop invariants, nonwrap bounds, transaction ownership
and O(n) scanning with constant storage; D2 diagnoses the supplied target and
repairs it. D3 supplies your own proof/fuzz/differential harnesses and independent
expected results. D4 demonstrates distinct harmless violations: repeated-key
acceptance, ignored tail or partial state publication after a malformed later
record. A sentinel value77 in this synthetic table is the success condition;
no real credential, external process, execution control or privilege is involved.
D5 minimizes the first violated property without switching failure identity;
D6 explains what each chosen method misses and repeats the argument after a
changed contract. All executable tests must stay in a disposable local process.

## Assessment and hints

T61.08.a assesses method selection tied to exact properties, `.b` the causal
repair and independent adversarial oracles, `.c` transfer and evidence limits.
Level1 reproduces known examples; level2 establishes transactional rejection,
canonical acceptance and distinct negative controls with justified bounds;
level3 completes this original delayed investigation unaided after seven days
with its recorded method choices. VT8-DELAYED permits repeated keys with a stated
last-operation-wins policy: distinguish the removed uniqueness property from
transactional and canonical-extent obligations that remain. This is a policy
change, not a larger random seed. Hints, released only as needed: identify the
first observable write; derive the exact remaining-byte invariant; distinguish
semantic validity from memory safety. Repair must include regressions for all
three defect classes, not simply bypass the supplied target.

The separate assessor files under `assessor/verification_fuzzing/independent`
contain a repair, independent oracle, bounded proof and technical analysis.
Author command `python3 tools/run_b06_independent_audit.py` is deliberately not a
learner acceptance shortcut. `independent_audit_validation.json` records actual
executions and limits. Finite oracle/fuzz samples and a22-byte bounded proof are
not full unbounded correctness or independent learner evidence. Inspected sources
are the already registered R-B06-FOLLOWUP-C11-TYPES, CBMC-UNWIND and CBMC-PROPS;
this exercise claims no additional source inspection.
