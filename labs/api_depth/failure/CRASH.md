# T21.10: evidence, identity and recovery are different contracts

Implement `crash.h/c/tests` after CONSTRUCTION.md and TRANSACTION.md. Families
A/B/C/D/F/G/I/J cover the original three criteria and changed packets. Reuse the
actual constructor/resource ledger and journal; the author campaign identifies
their assessor provenance. `struct ae_recovery` is stable, noncopied storage because its
private constructor points into its own ledger. This is a bounded original
failure model and selected hosted ELF experiment, not a deployed crash collector.

The compact record is five octets: version1, build1..7, byte address0..255,
state0..3 and sum of the preceding four modulo256. `[1,4,9,2,16]` is valid;
checksum17 on the same payload is invalid. Decode exactly five bytes, validate
before output and preserve output on failure. A checksum detects selected
corruption, not malicious forgery. Build and address are synthetic identifiers in
this compact core; the ELF exercise uses a separate full build-ID/address record.

The changed persistence model has two eight-byte slots: generation1..3, the five
record bytes, sum of those first six bytes modulo256, and commit marker165.
Select the valid greatest nonwrapping generation. Equal valid generations conflict.
To save, invalidate the inactive marker, write generation, payload and slot sum,
then write the commit marker last. Reset cuts1..9 occur before each of these nine
byte writes; cut10 follows commit before acknowledgment. An older committed slot
survives all precommit cuts; cut10 selects the new record even though the caller
received AMBIGUOUS. Generation3 refuses further saves. The model assumes ordered,
atomically persistent byte writes and no destruction of the other slot. A real
flash/filesystem adapter must replace these assumptions with its programming,
flush, erase, torn-write and power-failure contracts; cross-building this C does
not establish them. Exercise single-byte corruption and equal-generation conflict.

For symbolization build two real GNU/Linux PIE artifacts from the supplied
original fault-site specimen, with logical builds4/5 and distinct ELF build IDs.
Retain separate debug files. A selected runtime function-entry address is synthetic
crash evidence; no real fault or arbitrary PC is invented. Check the exact build ID
before running addr2line. Observe the runtime load bias and subtract it for this
ET_DYN/PIE contract; verify against the independent ELF symbol value, then resolve
the source. Relocate the executable to another directory and run it again.
Function-pointer/integer conversion and dladdr here are selected GNU/Linux ABI
assumptions, not portable ISO C. Return addresses, interior PCs, Thumb bits and
ET_EXEC addresses require their own normalization rules; do not blindly apply
the PIE subtraction to them.

A valid record with state3 is explicitly unrecoverable and must not start recovery.
State2 and the matching expected build are prerequisites, not proof of safety.
Recovery cleans any prior private constructor, proves the ledger empty, begins a
fresh private control lifetime, acquires resources0/1/2, submits synthetic effect
(ID1,delta1) to the actual two-entry journal, then marks done. Internal handles
never escape, so quiescent recreation does not reset a public generation behind
escaped references. The effect's completed identity remains retained during this
bounded recovery; other requests do not evict it. Each of five reset cuts is
repeated and resumed. At completion exactly one effect exists and each held
resource has acquisitions=releases+1. Repeated done recovery has no new events.
After eight admitted attempts the model refuses further restart. These are
persistent-state model cuts, not actual OS crashes or physical measurements.

Deliver byte fixtures and rejection cases, slot-state derivation, build/debug
identity manifest and raw symbolization commands, recovery preconditions and
resource/effect traces. Costs are two8-byte slots plus bounded recovery state and
at most nine byte writes per save; measure compiled code separately from modeled
persistent storage. Hints: put the commit marker last; validate identity before
looking up a plausible line; never infer recovery from a successful decode.
Level2 requires all three independent arguments and discriminating tests. Level3
requires the changed slot, load-bias and repeated-reset packet after the original
minimum seven days. Author reference runs do not supply learner or board evidence.
