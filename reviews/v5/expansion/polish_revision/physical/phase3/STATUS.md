# Phase 3 status (NUCLEO-F439ZI, PLAN.md revision 4, commit 647feced2)

Runs on 2026-10-30 from 19:43 to 19:52 UTC (the board host's clock;
2026-10-31 local). The user granted full mandate for board actions.
perf_event_paranoid was -1 in every run.json. 28 run directories are in
`runs/`.

## Order and outcome

| Step | Commands | Outcome |
| --- | --- | --- |
| 0 | tests, mutation harness, build, dry run, openocd self-check | 171 tests OK; 235/235 mutants killed; 12 ELF hashes equal PLAN section 4 |
| 1 | `probe` | identity equal to the baseline; DBGMCU_APB1_FZ 0x1800 (precondition met); DBGMCU_CR 0x3 |
| 8 (D1) | flash bu-cold-cross, `read --also-running`, park, control read of bf-report | all completed; see D1 |
| 3 (deferred bu-*) | bu-cold-cross, bu-software-reset-cross, bu-guard-cross, each with flash and halted read | all completed |
| 4 (bu-watchdog-cross) | two `read --also-running` attempts, then one halted-only read | the two running attempts were refused (recover after each); the halted read completed |
| 6 | park, final capture, probe | OPTCR 0x0FFFAAED, OPTCR1 0x0FFF0000, equal to the step-1 baseline; DBGMCU_CR 0x3; APB1_FZ 0x1800 |

Every flash and recover read back equal to its gated image. FLASH_OPTCR and
FLASH_OPTCR1 never changed.

## D1: debugger reads while a bu-* image runs (PLAN section 5)

Raw values come from `runs/*-read-bu-cold-cross` (19:43:46) and
`runs/*-read-bf-report-115200` (19:44:11).

- **P8.1 holds.** `flash bu-cold-cross` completed. Its post-program identity
  came from the halted window and was equal to the baseline.
- **P8.2 holds.** Halted reads in the window:
  - `bu_stage` 5, `bu_data` 0x12345678, `bu_bank` 0x87654321;
  - vectors 0x20010000 0x080001ad;
  - both copies of FLASH_OPTCR, FLASH_OPTCR1 and RCC_CSR were equal;
  - RCC_CR 0x00048a83 and PWR_CR 0x0000c000, equal to the control.
- **P8.3 shows the H1 pattern, with one deviation.** The running DHCSR read
  0x00050001: S_SLEEP set, the core asleep in WFI. Running reads:
  - FLASH_OPTCR, FLASH_OPTCR1, RCC_CSR, RCC_CR, PWR_CR, `bu_stage` and
    `bu_data` all read 0x00000000. That is one value, but not phase 2's
    0x5.
  - `bu_bank` (CCM) read correctly, and so did DBGMCU_CR. The flash
    vectors read correctly before the halt.

  The deviation: after `resume`, the flash vectors also read 0.
- **P8.4 holds.** On the control image (bf-report, which does not use WFI
  or the MPU), running reads equal halted reads. That covers FLASH_OPTCR,
  FLASH_OPTCR1, RCC_CR, PWR_CR, `data_canary` 0x5AA5C33C and the vectors.
  Its running DHCSR read was 0x01010001 (awake), so H3 (the probe path
  alone) is excluded.
- **P8.5 is not met.** For bu-watchdog-cross (MPU on, no WFI), the first
  running read printed DHCSR 0x01010001, so the core was awake. openocd then
  exited 1 after 0.43 s, on the next running read (SRAM `bu_stage`), with no
  further output. The same happened twice (`runs/*-read-bu-watchdog-cross`
  at 19:47:08 and 19:49:17). Under H1, running reads would have equalled
  halted ones. They failed instead, with the core awake. So the effect is
  not explained by sleep alone. Since this image has the MPU enabled, the
  MPU or debug-access privilege hypothesis (H2) gains weight. That is a
  derived conclusion, not an established mechanism.

Derived conclusion: while these bring-up images run (MPU enabled, sleeping
or not), debugger reads of SRAM and peripherals made with the core running
are not reliable. They read zero or stale values, or the access fails.
Reads inside the halt window are reliable: every halted value checked
equalled the image's own data and the control. The halt-bracket identity of
revision 4 is therefore the correct design for these images. The mechanism
inside the core, bus matrix or ST-LINK is not established *[doc-check
Armv7-M, RM0090]*.

## E2 retained-stage results (bu-* cross images, halted reads)

| Image | bu_stage | bu_reset_flags | bu_retained | Other |
| --- | --- | --- | --- | --- |
| bu-cold-cross | 5 | 0x04000000 (PINRSTF) | 0x0040DA02 (not the marker) | SCB_VTOR 0x08000000; 107 vectors equal the ELF; RCC_CSR 0x04000000 |
| bu-software-reset-cross | 5 | 0x14000000 (SFTRSTF, PINRSTF) | 0x42553237 ("BU27" marker) | RCC_CSR 0x14000000: the software reset ran, and the retained word survived it |
| bu-guard-cross | 3 | n/a | n/a | SCB_CFSR 0x00000082 (MMARVALID, DACCVIOL); SCB_MMFAR 0x2000F7FC; SCB_SHCSR 0x00010001 (MEMFAULTENA, MEMFAULTACT); SCB_HFSR 0 |

The guard image's MPU no-access region below the stack caught the
deliberate write at `_stack_low - 4` as a MemManage data-access violation,
not a HardFault. Its handler was active when read.

## E3 bu-watchdog-cross: did not behave as designed (open)

Halted read 20 s after programming (`runs/*-read-bu-watchdog-cross` at
19:51:30):
- `bu_stage` 3;
- `bu_reset_flags` 0x04000000 (no IWDGRSTF);
- `bu_retained` 0x0040DA02 (no "WD27" marker);
- RCC_CSR 0x04000003 (LSION, LSIRDY, PINRSTF);
- IWDG_SR 0x00000003 (PVU and RVU still set);
- DBGMCU_APB1_FZ 0x1800, DBGMCU_CR 0x3.

The image is meant to arm the IWDG, starve it, and reach stage 4 after the
watchdog reset. Twenty seconds after programming it had not armed the
watchdog. The IWDG prescaler and reload update was still pending (the image
waits for SR to clear), and no watchdog reset had happened.

A hypothesis, not established: the post-program identity halts the core
with DBG_IWDG_STOP set (APB1_FZ 0x1800) while the update is in progress,
and the update does not complete after that. A test would read IWDG_SR
over time with no attach after programming, or program with DBGMCU_APB1_FZ
clear (only possible after a power-on reset). That needs a reviewed
decision. The phase-2 bf-iwdg-fed and bf-iwdg-starve results, which were
programmed the same way, did arm the IWDG. So the difference is specific to
this image's arming sequence or timing.

## Not done

- the power-on observation (step 7), which needs a physical USB replug;
- a mechanism for D1;
- the E3 bu-watchdog-cross cross-check.

These are physical observations of this board, with the derivations marked.
They are not learner or independent-review evidence.
