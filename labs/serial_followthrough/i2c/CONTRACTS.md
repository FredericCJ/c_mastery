# T38.04/.05: electrical intervals, arbitration and transaction ownership

This version1 contract supplies concrete implementation and changed-condition
work for all six original criteria in PR-T38-03 and PR-T38-04. Their stable AX IDs,
seven-day V1 IDs, prerequisites, rubrics and historical acceptance are retained.
Families are A/B/C/D/F/G/H/I/J as mapped by each original criterion. Read this
contract together with the actual `labs/peripheral_depth/i2c/CONTRACTS.md`.
The source register is `research/v5/expansion/legacy_serial_i2c/resources.json`;
only its exact inspected locators support the source claims below.

## Prerequisites and environment

Use the parent T38.04/.05 teaching, caller-owned fixed storage, checked integer
arithmetic, T27 pin/clock ownership and the T28 PiI2c/PiRecovery producer. Submit
the producer source identity and contract recall before composing it. Preserve
the actual Pi API: substituting a success-returning mock does not demonstrate
composition. A fixture models the hardware observations behind that API.

The pure `i2c_depth.h` core uses C17, exact uint8/uint32/uint64 types and no hosted
runtime or allocation. Admitted implementations provide those optional types.
No library may supply the electrical interval or arbitration mechanism. The
reference uses an explicit fixed-step unsigned division; an alternative must
account for its generated helpers and costs. `recovery.h` composes the actual
Pi state machine, Pu clock decoder and their documented dependencies. Native
stdio, Python Fraction, compilers, sanitizer runtimes and QEMU are verification
tools, not target dependencies. The strict M4 image uses explicit original
byte-helper source, reset assembly and linker input, with no default libraries.
The semihosting harness uses compiler assembly extensions; this is not ISO C.

ISO C establishes arithmetic/object access only. MMIO widths, ACK/POS ordering,
clock epochs, register side effects, pin routing and bus timing come from the
specified ABI, producer, RM0090, errata and electrical premises. C11/C17 provide
the unsigned operations used here; C23 changes neither a bus transaction into
an atomic C operation nor a volatile access into a lifetime guarantee. Exact
published-edition source-access gaps are retained, not filled with C11 passages.

## Problem and deliverables

Implement the public stubs and retain these six objective-specific packets.
Controls and input/output objects must be live, aligned and disjoint; callbacks
are nonreentrant and do not retain expired storage. Null required inputs return
invalid where declared. No pointer provenance or physical lease is inferred
from numeric flags. All limits below are API admission rules, not I2C maxima.

| Objective | Required artifacts and explicit expected boundary |
| --- | --- |
| T38.04.a | D1: derive and implement `ia_nominal`; D2: prove and implement `ia_guaranteed` with leakage/tolerance; preserve invalid outputs. The ordinary3.3V/0.4V/3mA/300ns/100pF interval rounds inward to967..3540Ω. At400pF it is empty967..885Ω. |
| T38.04.b | D3: implement `ia_arbitrate`, publish every bus/lost/active entry, justify identical survivors and first released1/observed0 loss. Identify protocol combinations outside the model. |
| T38.04.c | D4: audit original synthetic analog packets with `capture.py`; submit the separate raw-evidence dossier in BOARD_CAPTURE.md. Change capacitance/mode, recompute admission and identify measurements that cannot be reused. |
| T38.05.a | D5: compose actual Pi for0x50/register07 with one-byte5A then two-byte5A,C3. Record A0,07,A1, ACK/POS/BTF/STOP operations and unchanged caller output until terminal success. |
| T38.05.b | D6: distinguish address NACK, data NACK, arbitration loss and clock-wait timeout. Then deliver token1 after token2 begins and after token2 has received data; show no stale register read, state advance or output publication. |
| T38.05.c | D7: implement `ia_clear_begin`, compose actual recovery, enumerate idle, release after1..9 clocks, still-low after9, held SCL, modular-time wrap and absent lease. Explain the changed-condition other-controller counterexample. |

## Electrical representation and invariants

`ia_nominal(vdd_mv,vol_mv,iol_ua,rise_ns,cap_pf,out)` accepts vdd1..6000,
0≤vol<vdd, iol1..20000, rise1..1000000, cap1..10000 in the named units.
Return `IA_INVALID` without publishing for invalid input. For valid inputs
publish inward-rounded integer-ohm limits, `IA_OK` when minimum≤maximum and
`IA_EMPTY` otherwise. Use the original nominal0.8473RC30–70% assumption and
the sink-current constraint. Derive all unit conversions before coding.

The changed `IaEnvelope` admits vmin1..vmax≤6000mV,0≤vol<vmin,
the same iol/rise/cap bounds, source and sink leakage0..20000µA and tolerance
0..250000ppm. Leakage bounds are explicit original worst-case assumptions,
not manufacturer specifications. Supply and component tolerances are independent.
Use source leakage for the low-state sink budget and sink leakage for charging.
If source leakage consumes the whole sink budget, publish an empty interval.

Derive a *sufficient*, potentially conservative charging bound: throughout30–70%
the remaining pull-up voltage is at least0.3Vmin. Require the minimum net current
over that window to charge C by0.4Vmin within the admitted rise time. Apply the
smallest actual resistor to the sink constraint and the largest to charging.
The output is a nominal integer resistor interval satisfying these conservative
inequalities; it is not the maximal feasible interval from an exact leakage RC
solution. Prove products and conversions fit uint64, final bounds fit uint32,
rounding is inward, divisors are nonzero and rejection preserves output.
Both scalar functions are O(1) with bounded iteration and storage under this API.

## Arbitration representation and limits

Input is at most three arrays of at most32 bit symbols, plus positive low/high
hold durations. Symbols0/1 mean pull low/release;2/3 denote START/STOP and return
`IA_UNSUPPORTED`; larger symbols return `IA_INVALID`. Validate before publishing.
The successful `IaTrace` contains observed wired-AND bus bits, newly lost masks,
survivor masks after each bit and final survivors. A lost controller releases SDA
and does not manufacture STOP. Equal streams need not elect a unique winner.

`low=max(low[])` and `high=min(high[])` describe the initial synchronized clock
interval while all declared controllers participate. They do not predict later
clock periods after a losing controller ceases clocking at a byte boundary.
The32-bit data arbitration trace abstracts those clock changes. It does not
model ACK ownership, target-mode switching or analog edges. Mixed repeated
START/STOP/data while arbitration is unresolved is outside the specified I2C
protocol condition, rather than a winner this simulator should invent. Cost is
O(controllers×bits), bounded storage and no allocation.

## Transactions, failure and recovery

Keep the retained Pi ordinary7-bit address admission, one/two-byte count, clock
generation, strictly fresh nonzero request token and under-half-range deadline
requirements. The scalar event clock advances monotonically modulo32bits.
Receiving bytes does not alone end ownership: publication requires the producer's
successful terminal STOP observation. Output, controller state and callbacks stay
live through pending cleanup; timeout does not prove the device ignored a command.
One-byte and two-byte closing differ in ACK/POS and BTF sequencing. Explain why
the final controller-generated NACK is normal, distinct from address/data rejection.

The F439 target uses the actual producer pin/clock adapter and narrowly scoped
PRIMASK save/restore around each call. It does not hold interrupt masking while
waiting for a bus edge. The register fixture checks configured PB8/PB9 AF4,
open-drain, CCR/TRISE, phase-sensitive accesses and prior PRIMASK restoration.
The side-effect fixture independently checks ACK/POS before clearing ADDR and
the prohibition on writing CR1 with START/STOP/PEC pending. These are separate
observations; plain RAM does not reproduce all peripheral side effects.

`ia_clear_begin` requires an already established exclusive recovery lease covering
the entire operation, no other controllers, valid PiLines callbacks, inactive
PiRecovery, budget/half within the retained producer's bounds. Failure of those
admissions occurs before line writes. Observe both released lines: if already
high, return `PI_OK` inactive without manufacturing STOP. Otherwise delegate the
actual bounded Pi recovery. Each raised SCL is observed before counting its high
hold. Stop after at most9 pulses; still-low SDA returns `PI_IO` and requires an
explicit higher-level reset policy. A held SCL times out and leaves both outputs
released. There is no `reset_required` Pi enum or automatic power cycling.

If another controller starts after recovery began, the lease premise was false.
A free-looking bus or elapsed deadline cannot establish exclusive recovery
authority. D7 must supply a concrete conflicting-start trace and a policy that
refuses recovery without an out-of-band lease; adding more pulses is not a repair.
Callbacks cannot conjure or verify that lease merely from an unsigned flag.

## Verification and rubric

Retain unaided predictions before running tests. Compile with both GCC and Clang
at O0/O3 and ASan/UBSan; compare emitted observations to an independent rational
and transition oracle. Test all65536 eight-bit pairs,150 leakage/tolerance
envelopes,10000 capacitances and the specified finite failure/recovery cases.
Compile the target with both compilers at O0/O3, inspect unresolved symbols,
link map, disassembly and emitted stack records. Explain the bound and excluded
interrupt nesting. Public unfinished stubs must compile and fail semantic tests.
Mutate rounding, leakage, loser removal, protocol admission, idle recovery,
ACK/BTF/ARLO, pin setup and token checks; identify the first violated assertion.
Finite coverage and sanitizer silence do not prove unrestricted correctness.

Use each original criterion's0–3 rubric with these mandatory level2 discriminators:
T38.04.a units/inward bounds and all arithmetic limits; .b complete survivor and
unsupported-state reasoning; .c decoded-byte versus timing/evidence distinction;
T38.05.a actual one/two-byte producer sequence; .b distinct cleanup and no stale
publication; .c lease, idle, pulse bound and line release. Level1 merely repeats
successful values or calls a mocked API. Level3 requires an independent seven-day
V1 attempt, a fresh falsifier and a revised resource/evidence argument. Author
reference results cannot fill the learner's level3 evidence.

## Graduated hints and variants

First draw ownership and the event that changes it. For arithmetic, mark units
before deriving a quotient; for bus transitions, identify who is pulling each
line low. Next inspect one failing boundary without reading assessor source:
which direction must an integer endpoint round, which ADDR read releases the
receiver, and which state still owns the caller's output? Finally compare a
minimal two-request or two-controller trace against your invariant.

Keep exact original `AX-PR-T38-03-T38.04.a/b/c-V1` and
`AX-PR-T38-04-T38.05.a/b/c-V1` identities. After at least seven days, require:
leakage plus resistor tolerance; START against data; changed capacitance/mode;
two-byte reception; old-token completion of a newer request; and another
controller during recovery, respectively. These alter mechanisms or premises,
not only constants. The assessor implementation and technical analyses remain
under `assessor/serial_followthrough/i2c`, outside the learner export.

## Security and validation limits

All fault injection targets original synthetic arrays, callbacks, register models
or dedicated teaching hardware. The stale-token mutant demonstrates a local
publication-integrity violation, not arbitrary code execution. No deployed bus,
third-party service, vehicle, host setting, real credential or flash operation is
authorized. A physical experiment requires the isolated setup in BOARD_CAPTURE.md.

Authored and executable evidence is separately recorded in the source-bound
validation and export reports. The actual author campaign, source inspection,
external review, independent learner attempts and physical observations are
distinct dimensions. The physical gate, independent assessment and other T38
projects remain required even when every finite software test passes.
