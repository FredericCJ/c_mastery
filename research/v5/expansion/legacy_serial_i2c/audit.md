# T38.04/.05 depth and exact original tasks

Both original projects, both complete legacy supplements and fixture records were
read. Six original criteria and their exact seven-day V1 tasks remain unchanged.
The earlier arithmetic/arbitration support cannot establish controller register
ordering, recovery authority or physical timing. The actual retained PiI2c,
PiRecovery, PuRates implementations, contracts, side-effect tests and F439 target
adapter were read. They supply one/two-byte combined transactions, token checks,
pending-command cleanup and bounded open-drain recovery. This package composes
those producers unchanged, rather than crediting their names as implementations.

The new pure calculator provides integer nominal intervals and an explicitly
conservative leakage/tolerance envelope. The new arbitration trace distinguishes
identical streams, first differing released bits and inadmissible START/STOP
combinations. The recovery adapter fixes a specification mismatch at its boundary:
an already-idle original PR38 fixture requires no pulses or fabricated STOP,
whereas calling the retained Pi recovery directly on idle would generate STOP.
Actual provider delegation is retained when either line is not idle.

The original board evidence gap is split, not erased. The actual F439 register
adapter now participates in host and Cortex-M4 RAM-model experiments. Analog
thresholds, rise/fall intervals, route ownership, peer behavior and calibrated
physical captures remain unobserved. A synthetic capture admission experiment
demonstrates why decoding the desired bytes cannot admit an excessive rise time.
The MPS2 experiment executes CPU instructions with a RAM register fixture; it does
not emulate the F439 I2C peripheral or measure line timing.

Named passages actually read: NXP UM10204 Rev7.0 printed pp8–12,19,50–51;
RM0090 Rev21 printed pp849–855,863–865,867–874; ES0206 Rev24 pp25–27.
NXP's undefined simultaneous START/STOP/data condition is a protocol limit, not
an ISO C undefined-behavior claim. The leakage values in this exercise are original
bounded inputs, not values borrowed from the unrelated Ultra Fast-mode table.
RM register rules and the errata qualify the retained controller implementation;
the original source register remains authoritative for its package/board pin rows.

Early compilation and original finite probes precede the final source-bound
campaign. Development attempts are retained separately when superseded. No whole
document inspection, physical validation, external review, delayed learner
attempt, absolute mastery or complete T38 scope is inferred from a passing suite.
The exact published C17/C23 access gaps remain unchanged.
