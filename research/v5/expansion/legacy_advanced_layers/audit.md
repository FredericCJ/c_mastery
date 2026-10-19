# T28.11 depth audit and successor

The retained PR-T28-06 has three genuine criterion-specific T28.11 tasks and
seven-day variants. Earlier firmware_io anchors did not implement a service
framing/cancellation protocol, posted-write mock or actual equivalent UART HAL
comparison. The prior DMA successor closes T28.10 only. This increment reuses
actual UART, serial and MMIO providers instead of counting their unrelated tests
as evidence for the layer tasks.

11.a adds an eight-octet unambiguous frame, polynomial oracle, borrow-preserving
service, real PuDriver adapter and SRAM-restricted F439 target adapter. Every
0..8 cancellation cut has a peer-parser observation; transmitted prefix and peer
commit are distinct. The local synthetic replay/CRC-forgery examples demonstrate
missing protocol properties without claiming memory corruption or exploitation
of third-party software.

11.b extends PmModel with a posted FIFO, delayed readiness, W1C/read-consume effects
and a partial-flush quarantine. The explicit visibility boundary explains why an
immediate RAM mock cannot establish the posted contract. A failed flush can leave
a committed prefix; retry does not erase it.

11.c uses actual HAL1.8.3 UART code in its ARM environment, separately from the
original first-principles target. The selected equal-contract RAM guest tests
TXE/write/TC, timeout boundaries and wrap. Separate guests cover callback restart
and frozen ticks. The documented F439/G474 map difference supplies a transfer
variant; full G474 initialization remains precisely source-dependent, not silently
treated as a working board port. Actual imported objects, linked helpers and
fresh-context code/stack costs are recorded; no physical timing is inferred.

An initial native HAL compile exposed ARM-only CMSIS primitives and pointer-width
assumptions. The supported ARM guest was implemented instead of defining fake
exclusive instructions or suppressing implicit declarations. Original code still
runs in native sanitizer profiles and big-endian service tests. All pilot and
final outcomes remain distinguishable from external/learner/board evidence.

No taxonomy leaf is added: these mechanisms fulfill existing required T28.11
leaves. The original task text, minimum delay, baseline metadata and historical
receipt identities are retained. T28.12 and T29 remain required work, not optional
exclusions. The source register records named passages actually read.

The second campaign retained a harness expectation defect: the stale-G4-map
mutant cannot be rejected by an F4-only guest because that changed branch is not
executed. The successor records its expected survival in F4 and requires semantic
rejection in the G474 guest and native two-map tests. This is not counted as a
mutant kill in F4. The four failed expected-result cells remain in attempt02.

Final contract review found an error-translation defect: the adapter mapped unknown,
FULL and UNIMPLEMENTED provider outcomes to terminal IO, which could release a
service borrow without a quiescence promise. The repaired adapter maps only actual
PU_IO to terminal IO and retains ownership on unknown statuses. Eleven injected
provider outcomes and an old-default mutant now discriminate this boundary; the
actual PuDriver integration remains separately exercised. Attempt03 records the
prior495-check scope; attempt04 reruns the expanded519-check scope.
