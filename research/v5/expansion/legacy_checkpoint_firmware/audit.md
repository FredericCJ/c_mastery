# Exact P4 depth audit and successor

The retained firmware provider already implements SPSC admission, P3 framing,
polling/DMA ownership, quiescent recovery and split-clock snapshots. Existing
half-duplex tests and actual MPS2 reset/PendSV runs are substantive and retained.
They did not provide a complete physical capture implementation/protocol, an
additional higher-priority workload, independent finite-capacity loss comparison,
or executable two-producer counterexample/correction.

The successor adds those mechanisms under disjoint public/private directories.
Its full source-bound campaign runs six native profiles, retained semantic
mutants, new delayed variants, independent timing models, capture/decoder
rejections and twelve valid F439 cross-images plus an invalid CCM placement.
An actual corrupted vector image is rejected. The existing reset producer runs
again into a unique successor receipt. Neither MPS2 nor a cross-build is F439
physical evidence. Board capture, non-author review and learner attempts remain
separate dimensions of the original IDs.

Source review discovered the older F439 example's reserved USART_SR write
(`~0x40`). The new entry uses0x3bf, retaining the original file and receipt.
The old renamed demonstration entry is not called. A1024-state host register
model and two compiled mutant runs discriminate this correction; the actual
successor cross-image is audited. Undefined/reserved hardware behavior is not
assigned a portable golden outcome.

The source set is bounded: actual named RM0090 Rev21, Arm DDI0403E.e, N1570
passages and DS9484 Rev14 table/footnote. The datasheet screenshot failed and a
direct download timed out; actual browser PDF text was read. Table10 footnote7
forbids assuming PA0 is an ordinary input in the named regulator/package mode.
The board protocol requires exact package, regulator and schematic qualification.
No board, calibrated instrument or dedicated transceiver is available in this
execution environment. Those block physical observations, not the completed
source, cross-build, host-model and assessor work.

Pilots remain evidence of their actual revisions. `build/p4-complete-pilot.json`
passed184 commands before the reserved-bit correction. Final current validation
is192 commands after the correction, with source identity checked at completion.
Earlier manual attempts had a linker-script assignment syntax error and a wrong
parser filename; they were corrected before successful builds, never counted as
semantic negative tests. Source caches are inspection evidence, not target
dependencies. Taught timing predictions are synthetic and retain their stated
rates, integer schedule, capacity and fairness assumptions.
