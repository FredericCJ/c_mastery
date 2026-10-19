# T59 reset, trust and generation continuations

These required B/C/E/F/G/I/J continuations retain PR-T59-01–08 and the B06 shared
contract. Prerequisites are the learner flash bit-transition model, boot/trial
state machine, checksum distinction, image extent parser and ownership ledger.
Use C17 hosted testing; the component itself allocates nothing. Host verification
is separate from a strict freestanding target's runtime dependencies.

## Concrete representation and operations

Implement `lifecycle_extension.h` from the supplied stubs. A24-byte journal record
contains LE32 sequence, value, complemented sequence, complemented value, CRC32
over the first16bytes, and marker bytes A5 5A 3C C3. CRC uses reflected polynomial
0xEDB88320, initial/final all-one XOR. Sequence0 is invalid. Recovery accepts
only full marker/check consistency, chooses the higher valid sequence, and
rejects equal-sequence conflicting values. Preserve outputs on rejection.
The old committed record remains intact while the new erased record is programmed
in ascending four-byte units; the final unit is the commit marker. The test
enumerates every completed-unit boundary, each subset of next-unit bytes, and
every bit prefix within the next unit. Arbitrary analog flash faults are outside
this digital model. Derive which early bit-prefix completions can already equal
the desired marker; an operation count is not a durability predicate.

Admission input is an immutable owned byte sequence: LE32 version, key ID1/2,
and at least one payload byte. The trusted key mask and revoked mask are trusted
configuration, not image claims. Reject unknown/revoked keys or versions below
the floor before calling the verifier. The callback receives the exact key ID,
full header+payload extent and signature bytes; only return1 accepts. A changed
header must require re-verification. The supplied callback witness checks binding
only; it is intentionally not cryptography. Never replace the callback with a
boolean carried in untrusted metadata. A real target needs a vetted signature
implementation and trust-anchor/key-storage policy as a separate dependency.

Joint health uses a finite generation, two acknowledgment bits and two outstanding
operation counts. Reset increments without wrap, clears both acknowledgments and
counts; acknowledgments from another generation reject. Confirmation requires
both current acknowledgments and zero outstanding operations. At the configured
maximum, reset fails unchanged; recovery needs a separately trusted new epoch.
This serialized model does not provide H745 cache/DMA ownership automatically.
Fault logging publishes a first committed four-word record; nested begin attempts
set a secondary flag and leave the first payload intact. No nested stack growth
or allocation is allowed. Real interrupt/NMI atomicity must be re-derived.

## Deliverables, verification and rubric

D1: code, byte-format derivation and recovery invariant; D2: all294 cut outcomes
and counterexamples to 'checksum authenticates'; D3: trust boundary/key-rotation
diagram and rejected revocation/stale-generation cases; D4: exhaustion and
nested-fault cleanup evidence. Run `python3 tools/run_b06_firmware_extensions.py`.
Require GCC debug/release and Clang ASan/UBSan reference passes, three distinct
mutants rejected at named assertions, and learner stubs rejected semantically.
No changed-state rejection may be hidden by resetting the fixture afterward.

FW1 maps T59.01.a/b/c and T59.05.a/b/c to D1/D2; FW2 maps T59.02.a/b/c and
T59.03.a/b/c to D3; FW3 maps T59.04.a/b/c,T59.06.a/b/c,T59.07.a/b/c to D3/D4;
FW4 maps T59.08.a/b/c to D4. Level0 confuses CRC/signature or wrap/recovery;
level1 ordinary success only; level2 supplies each mapped invariant, boundary
witness and tested repair; level3 independently adapts the assumptions after at
least seven days. Hints: isolate the commit predicate; make verifier ownership
explicit; then enumerate epoch transitions before writing the state machine.
Assessor analysis is `assessor/firmware_lifecycle/targets/ANALYSIS.md`.

Changed conditions require a different program-unit size, a separately vetted
signature backend, a key-revocation persistence failure, and a two-core reset
during outstanding DMA. The last case requires actual target-specific cache and
notification rules and remains physical/emulator-adapter work, not a host-model
pass. All data/images are synthetic and only dedicated teaching firmware is in
scope. No host settings, flashing, deployed bootloader or external credentials.
Author execution is recorded in `firmware_extension_validation.json`; independent
learner and physical acceptance remain unclaimed.

Inspected reference bindings: R-B06-FOLLOWUP-MCUBOOT.
Exact passages and inspection limits are in
`research/v5/expansion/b06/followup_resources.json`; source titles alone are not
inspection evidence. Retain those locator bounds when changing assumptions.
