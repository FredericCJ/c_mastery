# T37.08 source and depth audit

Read the actual original PR-T37-06 project/fixture, all three ordinary/V1
criterion objects and T37.08 module teaching. Freshly read RM0090 Rev21
pp59–64 and68, and visually inspected both bus figures on60/62; also read
p1010 USART allowed access widths. The changed F405/407 task is therefore
grounded in actual primary passages, not a copied F439 graph or bibliography.

Read the actual frozen Pd header, complete state-machine implementation and
authoritative DMA contract. Root confirmed this provider remains frozen. Its
whole ordinary-memory aperture is intentionally broader than this project's
single-bank allocation policy. The extension checks the stricter bank before
calling actual pd_validate/submit/poll/finalize; it does not duplicate a DMA
driver or reinterpret EN clear as completed ownership return.

Read the actual B06 selected F439 linker recipe and the frozen PR-T37-02 typed
placement source/header. New linker additions include the original recipe and
reuse the original AWRecord objects unchanged. Two actual linked symbol maps
show the CPU-only temporary moving toCCM while both DMA extents remain fixed.
They are audit-only images; no real reset/initialization or silicon identity is
inferred. Host/model execution separately uses live typed storage.

Original missing mechanisms are the F407 difference, stronger whole-bank
admission, actual producer lease composition, unchanged DMA placement under
CPU-local relocation, and burst/wait assumptions behind a timing bound.
First native pilots passed five mechanism cases plus independent graph,
interval, lease, grant and bridge checks. An early new-prefix spelling typo
was diagnosed by strict compilation and corrected before the recorded campaign.
No compiler diagnostics were suppressed and no frozen producer was altered.

Final C17/C23 source access, real board/silicon/clock/errata qualification,
physical measurements and independent reviewer/learner evidence remain separate.
