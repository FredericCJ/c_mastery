# Physical reset capture and changed-target protocol

No board has been identified or flashed by this authoring campaign. This packet
is ready for a separately authorized dedicated development board session; it
must not be used on deployed equipment. Use synthetic records only. Preserve
all failed attempts and do not infer a board revision from the order code.

Before writing an image, fill `board_capture.json`: board/assembly and device
marking photographs, silicon revision and applicability of ES0206, exact tool
versions, image/ELF/linker hashes, power configuration, boot strap/option readback,
clock source, target voltage and manual revisions. The F439 conservative image
uses its internal HSI and no UART. Do not silently substitute HSE driven by
ST-LINK, or assume a debugger configured clocks/RAM on your behalf. Do not alter
option bytes as part of this exercise. Any desired alternate boot provisioning
requires an explicitly reviewed target-specific procedure.

Connect an appropriate high-impedance voltage probe to the documented VDD test
point and a reset probe to the verified NRST test point, with instrument ground
connected to board ground. Record probe model, bandwidth, attenuation and sample
rate; choose resolution sufficient to distinguish supply ramp/reset release,
and include their uncertainty. Do not assume an unverified connector pin. If
only SWD is available, label that observation debugger-assisted; it cannot
establish the independent supply trace. Use the retained stage word from the
ELF symbol table as the early marker. Read it after completion without issuing
a reset, and record the tool's attach policy; verify attach itself did not reset
the board. Retained SRAM is not guaranteed through power loss.

Perform separate runs: debugger reset, AIRCR software reset, watchdog reset,
reset-pin assertion and full power removal/reapplication. The currently supplied
image supports cold startup and software-reset paths. The separate F439
`watchdog-cross` image captures RCC_CSR before clearing flags, waits for LSI and
IWDG update acknowledgments with a100000-poll bound, configures divide256 and
reload511, then intentionally stops feeding. On the resulting IWDG reset it
requires both cause bit29 and its retained marker before reaching stage5, then
feeds continuously for capture. It assumes software-started watchdog options;
reject a hardware-started option profile. Neither the poll count nor the
nominal32kHz timeout is a physical elapsed-time guarantee. Record actual LSI
range and debugger watchdog-freeze setting (RM21§21.3.3). No board run has been
performed. Capture initial RCC_CSR before
clearing RMVF, then explicitly clear the sticky flags before the next controlled
reset. RM21 p203 identifies SFTRSTF28, IWDGRSTF29, WWDGRSTF30, PORRSTF27,
PINRSTF26 and BORRSTF25; these are not mutually exclusive one-hot outcomes.
Record raw bits and explain coexisting causes and retained backup-domain power.
Debugger reset strategy determines which reset is actually applied.

Accept an individual run only if identity and source hashes match, the expected
boot route was observed, vectors/stack/copy/zero checks passed, stage reached5,
no unexplained fault occurred and the raw observation supports the stated reset
category. Repeat with the offset bootloader image and RAM-vector ownership
variant; do not reuse a direct-boot receipt. The available emulator executes
cold reset, software reset, offset entry and RAM vectors separately. Watchdog,
pin, brownout and true power-cycle observations remain physical obligations.

For H745, use the existing `firmware_lifecycle/targets/SELECTED_STARTUP.md`
contract and record both core images, BCM4/BCM7 and boot-address readbacks,
held-peer state, bank clocks/ECC initialization, caches and shared-line owner.
RM0399 Rev4 pp110–112 distinguishes MDMA access through M7 AHBS from ordinary
DMA1/2; pp143–145 specify TCM/boot domains, pp362–363 the hold sequence, p477
the one-way software-set BOOT_C2 bit. Setting it back to0 is not a software stop
protocol. A cacheable shared handoff must clean producer dirty lines, order
publication and invalidate the consumer's stale lines under the selected cache
contract; reject overlapping live owners. A byte-atomic retained model is not
automatically valid under H745 ECC partial-word behavior described on p143.

For independent delayed assessment, retain the original minimum seven-day
interval and supply a fresh bank/linker layout or inherited-state record. The
assessor checks derivation, rejection controls and source applicability before
any target action. An unavailable instrument leaves only its measurement
dimension pending; image/core/assessor work remains executable.
