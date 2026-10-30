# Phase 2 status (NUCLEO-F439ZI, PLAN.md revision 3, commit 7ce0251ba)

Operator run on 2026-10-30 from 16:17 UTC; perf_event_paranoid -1 (recorded
in every run.json); host lock /run/lock/cmastery-f439.lock plus the checkout
lock.

## Done

| PLAN §6 step | Command | Result |
| --- | --- | --- |
| 0 | `test_f439_board.py` | 140 tests OK |
| 0 | `test_f439_board.py --mutants` | 180/180 mutants killed (4 min 42 s) |
| 0 | `f439_board.py build` | all 12 ELF SHA-256 values equal §4 |
| 0 | `--dry-run flash bf-report-115200` | planned, no refusal |
| 0 | `selfcheck-openocd` | config parsed; examine-end and reset-init markers empty |
| 1 | `probe` | identity matches the receipt; see below |
| 2 | `recover` (first park programming, on a known-good board) | completed; readback identical; OPTCR and OPTCR1 unchanged |

Observed in step 1 (`runs/*-probe/run.json`):
- serial 0669FF485753667187124334;
- ST-LINK firmware V2J39M27 per openocd and V2J39S27 per st-info;
- chip ID 0x419, 2048 KiB flash;
- SWD IDCODE 0x20036419, DBGMCU_IDCODE 0x00000419;
- FLASH_OPTCR 0x0FFFAAED, FLASH_OPTCR1 0x0FFF0000, recorded as the
  `probe-baseline.json` for `recover`;
- DBGMCU_CR 0x00000007 and DBGMCU_APB1_FZ 0x00001800, as predicted in P2.0
  (openocd's examine-end writes);
- target voltage 3.246 V.

Observed in step 2 (`runs/*-recover-bf-report-115200/`): connect under reset,
DBGMCU_IDCODE read, bf-report-115200 (ELF 451acb96...8a3a) programmed under
reset. The flash readback of 5892 bytes at 0x08000000 (sha256 d17300ec...7c77)
equals the gated image. After programming, OPTCR is unchanged and OPTCR1
equals the baseline.

## Continuation (2026-10-30 17:39-17:57 UTC, user mandate for board actions)

The first serial capture had been refused by the session's tool-permission
classifier. The user then granted full mandate for actions on the board, and
PLAN section 6 resumed at step 2's first capture.

| Step | Result |
| --- | --- |
| E1 | five UART captures (baseline 115200 8N1, parity even, host 57600, target 57600 against hosts 115200 and 57600), each termios-unchanged; `analyze uart` JSON is in each run's capture |
| E2 / X1 (bf-report) | program reset, debugger reset at 2 s and software reset captured |
| E2 bu-* | refused: see "bu-cold-cross refusal" |
| E3 | fed and starved IWDG captures; `analyze starvation` |
| E4 | core clock and /8 clock captures, both with two DLE Q pairs and `analyze probing` stopped=true; Stop without and with DBG_STOP; DBGMCU read |
| 6 | park, final capture, `probe`: OPTCR 0x0FFFAAED and OPTCR1 0x0FFF0000, equal to the step-1 baseline; DBGMCU_CR 0x3 |

Host load was between 3.6 and 7.5 during these captures (other builds were
running). Host-side timestamps carry that jitter. Board-side counters do
not.

### Observations (derived; raw captures in `runs/`)

- **E3, IWDG starvation.** Each reset of the starving image set
  RCC_CSR.IWDGRSTF (0x24000000), every 4.164-4.172 s across four
  intervals. With the image's prescaler and reload, that implies
  f_LSI of about 31.4-31.5 kHz, inside the datasheet's LSI range. The first
  interval (1.75 s, CSR 0x04000000, no IWDGRSTF) is the scheduled debugger
  reset at 2 s, so it is not a watchdog interval.
- **E4, clock at core rate.** DWT CYCCNT advanced 15,999,999-16,000,001
  cycles per SysTick period of reload 15,999,999. The board's own counters
  agree on a 16 MHz HSI core clock. A host-time regression gives 15.90 MHz,
  but its residuals are large (rms 0.93 s, max 5.98 s), so only the
  board-side agreement is reported as a result.
- **E4, /8 divider.** CYCCNT deltas reach 128,000,000 (= 8 x 16,000,000)
  per tick. The host-time regression from `analyze ticks` is unusable
  (negative slope). This is recorded as a defect of the offline analyzer,
  not of the board; the raw capture is kept for reanalysis.
- **E4, Stop.** In each of 10 Stop cycles:
  - without DBG_STOP, CYCCNT advanced 568 cycles over about 2.48 s of host
    time (clocks stopped in Stop);
  - with DBG_STOP, it advanced 39.7-39.8 million cycles over the same span,
    about 16.01-16.03 MHz against host time (clocks kept running).

  All 20 cycles woke after the wake byte. The bf-stop-nodebug image had
  DBGMCU_CR 0x0 when read afterwards.

### bu-cold-cross refusal

`flash bu-cold-cross` was refused (`runs/*-program-bu-cold-cross`). Openocd
reported "Programming Finished" and "Verified OK", but the post-program
identity attach, made while the image runs, read 0x00000005 at
FLASH_OPTCR, FLASH_OPTCR1 and RCC_CSR alike. Debug-domain and system-memory
reads were correct, and the 1200-byte flash readback from the same attach
equals the programmed image.

Read protection would block a debug read of flash, so the option bytes were
not changed; the 0x5 values are a read artifact. The bring-up image enables
the MPU with MPU_CTRL = 5 (ENABLE | PRIVDEFENA). `recover` then reprogrammed
the park image under reset and read OPTCR 0x0FFFAAED and OPTCR1 0x0FFF0000,
equal to the baseline (`runs/*-recover-*` at 17:45).

The bu-* steps (E2 retained stage, X1 cross-checks, the bu-watchdog-cross
part of E3) are deferred. They need an identity read that is reliable while
such an image runs, which is a reviewed change to tools/f439_board.py.

## Not done

- the bu-* images (see above);
- the power-on observation (step 7, P2.4), which needs a physical USB
  replug;
- a corrected host-time analysis of the E4 tick captures.

These rows are physical observations of this board. They are not learner
or independent-review evidence.
