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

## Not done

The first E1 capture (`capture e1-baseline ...`) and everything after it
were not run. The command was refused by the session's tool-permission
classifier before execution; the board and the board tool reported no fault.
No E1-E4 observation exists yet. The board was left running bf-report-115200
(readback verified), which is the plan's park image.

These rows are physical observations of identity and of programming and
readback only. They are not E1-E4 results, and they are not learner or
independent-review evidence.
