# Assessor analysis: G474 initializer

The register model deliberately knows neither addresses nor a complete G474
peripheral implementation. It records reads/writes and controls HSIRDY/TEACK
transitions. Its212 cases are160 baud/readiness pairs,42 missing/delayed flags,
8 admission failures and2 successful retries after partial initialization. Three additional tests import the unchanged generated
G474 byte adapter. This separates algorithmic sequencing from the independently
compiled primary-header layout assertions and the actual board-image audit.

At115200, ideal divisor is138+8/9; nearest139 gives16000000/139 baud and
relative error -12800/16012800. Integer multiplication can independently verify
the midpoint selection. `baud_oracle.py` minimizes exact distance over all65520
allowed divisors rather than reusing the C division formula. Nearest divisor
does not minimize every possible system-level communication error measure;
the contract explicitly selects divisor rounding. At921600 the nominal error
already consumes a substantial budget. A successful initializer need not imply
compatibility with a particular receiver.

Four compiling semantic controls select the wrong clock, wrong AF, enabled FIFO,
or skipped TEACK. Each must reach a failing assertion under all native profiles.
The unfinished public stub must also compile and fail a positive admission test;
a reject-all implementation cannot pass by satisfying negative tests alone.
The supplied PA8 MCO adapter additionally has four admitted/rejected model cases.
Its HSI16/16 output is measured against an external calibrated timebase, so the
clock check is not circularly derived from UART bits. The final two capture words
and version2 tag distinguish this configuration from the earlier author-only
version1 capture design. Actual wire/clock observations remain absent.

The F439 status-offset static assertion is an intentionally negative translation
fixture. It is not counted as a killed runtime mutant.

On clock failure there are no GPIO/USART clock operations. On transmitter-enable
failure the driver turns off UE/TE and reports stage4. Keeping the shared HSI
clock enabled is intentional. An alternative implementation may acquire/release
clock ownership through a separately proved manager; simply clearing HSION is
not equivalent cleanup. The result must not be modified on invalid admission.
After a failed TEACK, an owner may reset/retry; it must not pretend to preserve
an earlier in-flight transfer, excluded by the contract.

Check the independent sequence ledger, not just final registers. Configuring
BRR with UE enabled, dropping clock readbacks, or resetting another USART may
produce the right final model state yet violate the hardware/software contract.
The reference's source masks preserve neighboring GPIO/RCC fields; this is safe
only with serialized ownership. `volatile` is neither a lock nor an atomic
read-modify-write. The hardware target uses the original initialized clock/pin
provider and actual `pl_generated`/`pl_byte`, with an explicit TCCF write before
each byte. A preexisting TC bit cannot stand in for the new byte's completion.

The source comparison is deliberately layered: pinned ST LL/device code defines
selected initialization fields/order; DS12288 identifies the pin AF and nominal
clock characteristics; UM2505 identifies conditional physical routing; ES0430
identifies selected errata exclusions. None establishes that this particular
board was wired, measured or tested. RM0440 remains inaccessible and must not be
listed as an inspected source. The board gate prevents iteration counts from
being mistaken for oscillator-stabilization time. It is a debugger/instrument
protocol, not an autonomous deadline implementation.

Rubric for G-PL-G474-FULL-PORT-IMPLEMENTATION:

| Level | Observable evidence |
| --- | --- |
| 0 | Wrong mode/address/lifetime, unbounded wait under admitted failure, private solution leaked, or false hardware/source claim. |
| 1 | Ordinary nominal registers match; cannot derive masks, sequencing, timeout residue or the byte-completion boundary. |
| 2 | Original implementation, independent divisor derivation, admission/order/preserved-state checks, meaningful semantic rejections, actual image closure and correctly limited source/target dossier. |
| 3 | Independent changed-condition solution after at least seven days, with revised invariants, byte-provider composition and a falsification control; actual physical evidence separately assessed when available. |

Accept different decompositions and stronger error-reporting interfaces if they
preserve the exact admitted behavior and explicitly adapt consumers. Do not
reward a copied numeric oracle without the independent reasoning. A wider
system-clock architecture or HAL implementation is useful comparison material,
but cannot replace the required original strict implementation.
