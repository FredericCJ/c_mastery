# Bounded I2C retry and byte-to-transaction composition

Version 1. Required successor for X-T38-001-V1 (stretch), X-T38-001-V2 (transaction) and X-T38-004-V1 (retry). Original objectives, rubrics, prerequisites, profile and seven-day independent assessment remain required. Families A/B/C/D/F/G/H/I/J apply; no historical acceptance changes.

## Prerequisites and environment

Bring actual learner B045 SI2c, serial-depth SDI2c, T30 ct_elapsed and peripheral-depth pi_retry_policy/pi_device implementations. Read labs/b045_systems/CONTRACTS.md, labs/serial_depth/CONTRACTS.md, labs/constrained_depth/timing/CONTRACTS.md and labs/peripheral_depth/i2c/CONTRACTS.md. A canned I2C library or unrelated timer does not satisfy composition.

C17 baseline, eight-bit bytes; hosted GCC/Clang and GCC Cortex-M4 Thumb model profiles. C11 draft arithmetic premises apply to unchanged operations; no exact unavailable C17/C23 text inspection is claimed. target_build.py takes explicit producer inputs. Inspect final map, symbols, assembly and stack. The unchanged B045 whole source is compiled at O0 in GCC builds because its unrelated reducer triggers a strict optimized diagnostic; adapter/tests retain declared optimization. Clang M4 rejects the unchanged T30 whole source for unrelated64-bit atomics. Preserve that refusal. GCC final images must have no unresolved or retained atomic-helper dependencies. These are separate profile results.

## Problem and contracts

Implement all retry.h APIs in learner.c. Fresh live state is caller-owned, single-owner and nonreentrant. Inputs, outputs and controls are distinct; callers do not alter admitted fields. INVALID, AMBIGUOUS and STALE preserve state/output. Timeout and successful partial progress intentionally modify state. Fixed storage, constant work per event; no allocation, recursion or real device calls.

SrTimer accepts bits8 or16, a low tick in range, and budget strictly between zero and half range. Import ct_elapsed with a truthful less-than-one-wrap premise, reject observed gaps at least half range and decrement one remaining budget. Expiry wins over coincident success. Retries never renew it. The caller establishes true consecutive gaps below half range: residues cannot detect missed full wraps, reboot or clock changes. Construct two distinct elapsed histories with equal residues.

SrByte accepts byte0..255 and a positive stretch limit below half range. Every SCL/SDA transition arrives in order with its timestamp and sampling-edge flag. No data bit advances while SCL is low. Stretch starts at first observed low; the prior level is valid between events. Its exact expiry wins even when that event raises SCL. High resets a shorter stretch. Actual s_i2c_sample classifies ACK/NACK/arbitration. Terminal outcomes are sticky. Timeout does not authorize forcing STOP, clearing a bus or releasing a hardware lease.

SrTxn takes nonzero token, syntactic seven-bit address0..127 and register byte. Reserved addresses are synthetic coverage, not permission to contact devices. Use actual s_i2c_drive/sample for address-write, register and address-read bits/ACK, then sd_i2c_event for actions START_WRITE, WRITE_REGISTER, RESTART_READ, READ_NACK_STOP and final STOP. Final read accepts an already assembled byte; it models NACK/STOP, not an electrical receiver waveform. Arbitration loss requests RELEASE without STOP; NACK requests STOP. One total budget spans all bytes. Stale token preserves state/output. Objects are freshly initialized; an outer owner must fence generations across reincarnation. A numeric token is not a global capability.

SrRetry commands0x10/0x11 mean synthetic read/increment. Maximum attempts1..16, bus_free positive below half range, backoff0..half-1, and token headroom for all attempts. sr_retry_poll(now,scl,sda,idle) requires a trusted protocol monitor: idle means initial idle permission or completed STOP without a later START, not just two high wires during another transfer. Every transition/monitor change is delivered. Admission requires continuously high idle wires for bus_free and elapsed backoff since failure. Low/nonidle resets the continuous interval. READY consumes an attempt, advances token and enters ACTIVE; the caller performs exactly one operation for that ticket. Completion must match it. Actual pi_retry_policy uses command and truthful may_have_applied. A lost ACK after increment ends UNCERTAIN without replay. Read can retry; attempts/budget exhaustion is terminal. The scheduler generates no STOP/clear/reset. Real hardware composition additionally needs quiescence and compatible bus ownership.

Concrete traces wrap250→2, break idle qualification at254 and admit at2. Read loses ACK then succeeds on attempt2. Synthetic value9 becomes10 before a lost increment ACK and remains one attempt. Address50 hexadecimal/register80 emits A0,80,A1 and receives5A. Test every256 eight-bit start ×127 budgets one tick before and at expiry, plus twelve address/register traces across65535.

## Necessity and reasoning

Explain why retry eligibility differs from scheduled admission, two high wires differ from protocol idle, effects can precede ACK, and timeout differs from quiescence. Derive modular premises. Invariants: remaining never increases; attempts/token advance only on READY; at most one ACTIVE ticket; no progress while stretched; no STOP after lost arbitration; publish reply only on final success; uncertain effects are not duplicated. Separate ISO C arithmetic, timer contract, protocol and board behavior.

## Deliverables and verification

D1 implement all APIs with representation, lifetime, invariants and failure arguments. D2 run tests.c selectors0..2 and retain machine-readable outputs, compiler/link/target records. D3 independently derive residues,27 transmitted clocks and action order; construct fresh counterexamples violating each external premise. D4 reject seven local defects: budget renewal, unlimited stretch, missing R/W bit, broken idle interval, replayed uncertain increment, unbounded attempts and false idle. D5 inspect retained ct_elapsed/s_i2c_sample/sd_i2c_event/pi_retry_policy, final helper closure, stack and code size. D6 independently repeat after at least seven days with another timer width and partner that commits before failure. Explain limits of every observation.

Pass requires exact outputs, sanitizer-clean admitted paths, intended mutant/incomplete-stub rejection, refusal preservation, actual imported producers and dependency audit. Diagnostic refusal is a profile limit, never successful execution. Physical acceptance uses parent board capture requirements and remains separate.

## Rubric, hints and transfer

Level0 missing/incorrect; level1 happy paths with unstated premises; level2 correct D1–D5, bounded evidence and preserved uncertainty/ownership; level3 independent D6 implementation and derivation under changed assumptions. Stretch assesses timer composition/nonprogress; transaction assesses bit-to-byte ownership/action; retry assesses bounded admission/side-effect containment. Original rubric remains required.

Hint1 separate observation, admission and completion. Hint2 separate overall budget, continuous low and continuous idle counters. Hint3 draw commit-before-lost-ACK histories. Separate assessor analysis/implementation resides under assessor/serial_retry and must not enter learner exports.

## Security and validation

Only original synthetic devices and local native/emulated fixtures. No external bus, flashing or host configuration changes. Harmless witnesses are an extra synthetic increment, early READY or wrong action. Repair invariants and regress the witness; authority and physical quiescence remain external. Current author status is research/v5/expansion/legacy_serial_retry/validation.json with source pins. External review, delayed learner assessment and physical observation are not performed by that campaign. Finite passing traces do not establish absolute mastery.
