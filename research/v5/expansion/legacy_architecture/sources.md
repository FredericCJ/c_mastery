# T20 architecture source inspection and remaining limits

The actual T20 module, four project briefs, four original fixture contracts,
four legacy assessor analyses, all thirty ordinary/delayed prompts, and the
thirteen unit step/check bodies were inspected. Unit metadata is retained;
no old unit acceptance evidence is rewritten. The audit found real interface,
adapter, queue-policy, recovery and configuration implementations missing from
the retained generic anchors. `architecture_ports` is a different B06 ISA/ABI
family and was not counted as these T20 mechanisms.

Parnas's original1972 paper was read through Lafayette's digitized reproduction:
module criteria, both KWIC decompositions, comparison, implementation efficiency
and hierarchy. The host warns its digitization may contain errors. This supports
bounded design reasoning, not a claim to have inspected a publisher-certified
facsimile. Dijkstra's institutional EWD249 transcription was read specifically
at "On program families"; it supports shared assumptions and deliberate
variation, not a whole-monograph reading claim.

The selected STM32 GPIO comparison uses actual HAL v1.8.3 source and header,
the GPIO register structure and selected-device branch of CMSIS device2.6.10,
and CMSIS core5.9.0 headers selected by the compiler. GPIO initialization,
write/read/toggle/lock, EXTI callback and clear-mask definitions were inspected.
RM0090 Rev21 pp284–287 and389 were actually read from the already verified
private PDF. Its SHA256 is
`e34807b876642b67a6208e6e4071694f2892b5df088ef89f49cdad5cb254165f`.
No peripheral execution follows from these readings. The frozen F439 receive
driver's SR/DR contract was also inspected before actual composition.

Private source archives were fetched from the exact upstream tags, without
changing host security settings or installing system libraries:

| Archive | SHA256 |
|---|---|
| ST HAL v1.8.3 | `8e8a1a4872ab9617684806a6e7bd5aa82b71b5bfe0f88b6679760ca2933a8c02` |
| ST CMSIS device v2.6.10 | `b0566d2e07f6f38d69f3ba78ffa8bbac6f393c5b0fa31dd9e84f2ab42994fe7b` |
| ARM CMSIS5 5.9.0 | `9ce693dd15c41f02506adda79323c077912b3211303ca0d9b28b8ebe735a7031` |

They live under `/home/frede/cmastery-architecture-tools`. The HAL BSD3 license
was read; no third-party source is copied into the curriculum. Actual consumed
external headers and their hashes are recorded by compiler dependency output.
Archive availability is separate from inspecting every contained file.

POSIX2024 `read`, `pread` and `fsync` pages returned403 during this inspection.
That exact authoritative-edition access remains a reference gap. The actual
Linux man-pages6.19 descriptions, return values, errors and relevant caveats
were read instead: short reads, interruption/backpressure, positional offsets,
the Linux append-mode caveat, synchronization failure and directory persistence.
The source reading exposed two required distinctions: a matching stored record
after a failed synchronization still needs successful synchronization before
acknowledgment; append-mode descriptors do not satisfy this positional journal
contract. Both are exercised by original local tests.

Actual imported code inspected: foundation checked extent/cursor; CDRing
reserve/commit/push/pop; complete SFStream implementation; complete AsRegistry
adapter and underlying ApiRegistry; F439 receive/initialization code; T11 byte
helper provider selected for closure. Imported source is not called independent
review. The separately executed host references remain author evidence.

Exact published ISO C17 text, physical F439 operation and power-loss durability
remain separate gaps. No newly read full ISO/IEEE edition, book title alone,
source census, test passing or generated graph is treated as inspected evidence
or absolute mastery. C17 ownership and effect discipline are contracts checked
by specific assertions and reasoning, not language-enforced linearity.
