# Assessor: signed image and paired publication

Monocypher check returns0 on success, OpenSSL EVP verify returns1, and the
curriculum admission predicate expects an explicit boolean1. Translating
between them is part of the boundary; using generic truthiness reverses the
target provider's meaning. Key selection and complete envelope bounds precede
verification; current revocation/floors remain necessary after signature success.
The compiled provider is a declared external primitive, not a claimed original
Ed25519 implementation. The RFC empty-message vector is an independent expected
result; it is not proof of all provider edge cases or side-channel behavior.

base+512 is safe only after extent>=514 and extent<=limit-base are established.
Use subtraction for the entry extent, preserve output until final publication,
and keep an immutable verified snapshot. A valid vector is not a proof that
every instruction target or peripheral operation in a signed image is safe.
FBImage is forgeable by arbitrary trusted C; test callers cannot turn it into
an untrusted serialized authority record. The trusted coordinator, write
protection and actual slot-readback obligation prevent replacement between
verification and execution under the stated model.

Paired recovery reuses the actual FH and FM implementations. Both images'
generation and metadata match the journal, and core1 failure after core0 health
must keep the old record. The real host fault model performs32-byte-word
programming and selected erase cuts; this is stronger than a boolean mock but
does not model analog cell effects or H745 bus faults on torn ECC reads.

The handoff writes VTOR and switches CONTROL/MSP in terminal assembly. A C
epilogue after setting MSP would consume a foreign stack. CPU MPU state and
actual SRAM-execute/code-write fault observations are expected in separate
original MPS2 images. MPU PRIVDEFENA leaves privileged background access;
this is not isolation from malicious privileged firmware or DMA. Prior
interrupt, peripheral, FP and other-core ownership cannot be cleared by naming
it a reset. Require the platform shutdown argument before a board transfer.

The original stub rejection and three different mutants distinguish the policy
and semantic contracts. Full independent assessment still requires changed
conditions without copied analysis; a passed author campaign does not supply
that evidence. Exact physical and ECC-containment gaps remain in the ledger.
