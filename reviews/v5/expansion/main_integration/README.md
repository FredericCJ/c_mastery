# Selective integration evidence

The public source profile passed 72 checks on local main and repeated all 72 in the extracted source archive with the same source identity, `7c771587c2f05bc1d3255980ba9f6d7e4b5119121c2a8f61e1cdfc6a3519f5ed`. Both builds produced identical handbook pages. The learner archive initialized all seven inspection exercises and replayed the supplied layout observations offline. Repeating generation produced identical exercise, edition, inventory and route files. [Public candidate](public-candidate.json) · [Generator hashes](generation.json)

| Evidence | What was observed |
|---|---|
| [Compiler, ELF, GDB, Ghidra and QEMU checks](binary_inspection.json) | The full author run exercised GCC and Clang, Ghidra headless analysis of each stripped binary, a live debugger on both host builds, Cortex-M4 reset on QEMU, and separate replay and native evidence gates. |
| [Learner CLI checks](inspection-cli.json) | Five author-operated checks through the learner CLI selected one CTest each and bound the inspected source and image identities. |
| [Native PMU campaign and historical reanalysis](pmu/README.md) | The Intel campaign recorded 24 paired comparisons. The three AMD dossiers retain 4,038 raw rows. Their original measured executables and the runner version used at capture remain unavailable. |
| [F439 firmware, UART and restoration](physical/README.md) | The device reported startup results over USART3. The final sector-0 read matched both independent pretrial reads. The private rollback image is retained outside the repository. |
| [Defensive exercise contracts](security-contracts.json) | All 78 applicable canonical cards carry a checked defensive contract and evidence boundary. |
| [Strict author comparison](strict-author-comparison.json) | The integrated tree passed all 101 author-contract tests and 29 exercise-workflow tests. The pre-existing POSIX launcher export pin and source-projection negative control remain open. |

The new cards are available in the [learner guide](../../../../curriculum/guides/binary-inspection.md). The [47-commit selection record](../../../../planning/polish_integration.md) identifies which branch changes entered main. Independent assessment, delayed learner transfer and release acceptance remain open. The [route coverage notes](../../../../curriculum/v5/routes/README.md) identify the eight pending encounter bindings.
