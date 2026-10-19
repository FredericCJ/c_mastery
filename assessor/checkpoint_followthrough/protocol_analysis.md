# Exact P3 task analysis

## T04.06.b

`codec()` compares all13 fixture bytes at offset1, including unchanged outer
sentinels. `protocol_tests.c` constructs numeric258 independently from a
big-endian byte representation then verifies little-endian wire2,1. That meets
the model branch of the delayed disjunction; do not call it target execution.

## T15.12.b

Baseline lengths remain0..32. `CPXParser` has configurable admission for both
versions; the successor contract corrects the earlier imprecise “unchanged”
wording. New flags/timestamp alter CRC extent. The old parser rejects version2
after two input bytes; the wrong-extent mutant fails actual round-trip checks.

## T15.12.c

The reflected recurrence and known answer precede the host oracle. A public
checksum can be recomputed after a payload change. The authenticated successor
uses the explicitly permitted hosted HMAC primitive and full header binding;
mutant7 omits the header, mutant8 advances replay state before authentication.
Both fail their concrete synthetic property assertions. A volatile ledger reset
still permits an old valid packet and is deliberately demonstrated.

## T18.01.b

Capacity/invalid encoder refusal occurs before output writes. `peek` publishes
only when ready. The in-place variant moves high indices first after complete
admission and restricts its input overlap shape. Ordinary encoder and arbitrary
control/output overlap have no such guarantee.

## T15.13.b

Every frame composition is exercised; full queues retain ready and make feed
consume zero bytes. Prefilling the new admission wrapper makes both1- and2-slot
policies actually block the parser. There is no lossless unlimited-input claim.

## T15.13.c

Reject-sync+1 preserves the nested valid candidate. Whole-candidate skip is a
normal-exit wrong-output fault. Candidate bound B controls both retained bytes
and worst-case shifting/CRC cost; configurable16/64 tests must not be described
as the unchanged original32-byte acceptance contract.

## T21.05.b

`cp_deliver` separates frame validity from nonfailing copied admission. Its
clear-on-full mutant loses a pending message. Later `CPXSink` composes actual
receipt/store code: failure retains held input, reopening resolves uncertain
durability, and an exact retry returns the retained result. Seventy-four finite
working/stable-image cuts test the composed acknowledgement contract without
claiming physical power-failure behavior.

## T32.06.a

The independent whole-buffer oracle uses zlib and a different control flow.
The original deterministic corpus and configured/refusal corpus remain bounded
evidence; every run resets state. Input-dependent refusal schedules are logged
and bounded rather than relying on a live scheduler to reproduce a failure.

## T32.06.c

The retained minimizer preserves the missing-sequence7 predicate and checks
single-byte deletion minimality. New paired replays leave the reduced bytes
unchanged, vary chunk1/7/256 and logical capacity1/2, and prefill distinct900/901
messages. The repaired output equals prefills plus oracle output; faulty output
lacks7 under normal exit. The admission-drop mutant fails a distinct ready-state
assertion. Neither a sanitizer report nor a timeout would satisfy these checks.

The canonical completion packet maps each original ordinary and delayed ID to
its specific retained/new executable evidence. No author result is an external
review or an independently delayed learner attempt.
