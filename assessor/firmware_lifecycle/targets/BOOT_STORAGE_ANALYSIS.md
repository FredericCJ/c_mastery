# Assessor: storage identity is part of admission

The coordinator reads the complete signed envelope from the trusted candidate
slot and authenticates the snapshot. It never accepts the journal's digest as
proof that an independently supplied payload has the same bytes. Signature
placement is a fixed trusted slot convention; header length controls only a
bounded payload extent. The source layout object has named nested arrays to
avoid pre-C23 pointer-to-differently-qualified-array conversions.

Each journal candidate is admitted independently. Unreadable or unauthentic
new data can leave the older pair admissible, but only under current trusted
floors/revocations. This is an explicit availability policy, not rollback
prevention. The tests require a revoked newest key plus raised floor to reject
both candidates without leaking an intermediate sequence to output.

The metadata mutant ignores digest binding despite a valid CRC; the partial
publication mutant exposes a sequence before all authentication succeeds.
These are distinct semantic failures. The guest adds actual guarded-load
fault return followed by physical-address reads of original MPS2 RAM slots,
signature checks, record selection and terminal handoff. It does not model
STM32 flash ECC cells, power loss or a second CPU's boot protocol.
