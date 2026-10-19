# T28.08 I2C remaining work reconciliation

The original brief intentionally supports one-byte receive. Its changed criterion requires an inspected two-byte procedure; this successor supplies that procedure and an actual F439 I2C1 adapter. Original criteria, IDs, hours and baseline binding remain unchanged. The existing serial phase and firmware recovery references are imported under their own contracts; neither supplied the register-order, interrupt exclusion or multi-master recovery policy by itself.

Required depth now includes read-SR1/read-SR2 side effects, ACK/POS/BTF/STOP ordering, pending CR1 command exclusion, a START completing after timeout, separate NACK/ARLO behavior, spurious-BERR errata, line-level stretching/recovery, and a concrete lost-ack increment counterexample. GPIO recovery requires a physical exclusivity premise and cannot be inferred from timeout. Native target models and strict M4 images remain distinct from board observations.

Before closure, execute independent positive/negative and changed-condition support in all declared profiles, actual provider composition, target object/import audit, learner export and criterion-specific packaging guards. T28.09–.12 and T29 remain required author work. Historical elective labels do not exclude them; final canonical regeneration must retain comprehensive scope.
