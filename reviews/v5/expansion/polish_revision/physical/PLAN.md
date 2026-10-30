# NUCLEO-F439ZI physical observations: phase-2 review packet (B6)

Status: **phase 1 only (design, implementation, offline verification),
revision 3.** No board action was taken: no openocd, st-flash, st-info or gdb
invocation against the board, and `/dev/ttyACM0` was never opened. Nothing in
this file is an observation. Every expected value is a prediction written
before any run; phase 2 must not edit a prediction after data exists (add a
dated note instead).

Revision 2 answers the first safety review ("do not execute as written"),
which confirmed that no image or reviewed command writes option bytes, OTP or
system memory, or mass-erases. Changes: openocd search path and binaries
pinned (M1), command servers disabled (M2), `recover` reworked (M3),
multi-line `mdw` parsing (M4), no attach can meet a core asleep with the debug
clock off in the normal sequence (M5), and should-fixes 1-10 (§3). It also
covers the scientific-validity notes (§5) and a second review is required
before any hardware step.

Revision 3 (2026-10-31, still before any data) addresses the six should-fix
items of the second safety review: a host-wide lock shared by every
checkout, worktree and clone (§3.3); tests that fail when the per-tool
preflight, the per-tool sandbox check, the TIOCGEXCL confirmation or the
termios refusal is deleted, and a mutation harness on disk in place of the
earlier unbacked mutant count (§3, offline verification); a second,
idempotent DLE `Q` pair in each clock capture and an `analyze probing` gate
before the next flash (§4, §6 step 5; capture lengths 75 → 90 s and 150 →
165 s, P4.4 extended); `recover` as the first park programming while the
board is known-good (§6 step 2); a FLASH_OPTCR1 baseline for `probe` and
`recover`, and a readback compared with the bytes the gate hashed (§3); and
same-user tampering recorded as an accepted residual risk (§8). The
revision-3 changes have not been independently reviewed.

This packet does not claim learner, physical or independent-review evidence
and closes no obligation.

## 1. Scope and fixed limits

- Board: NUCLEO-F439ZI, ST-LINK serial `0669FF485753667187124334`, target
  DBGMCU_IDCODE `0x20036419` (DEV_ID 0x419, REV_ID 0x2003), UID
  `004c0025 3133510a 36393739`, 2048 KiB flash, FLASH_OPTCR `0x0FFFAAED`
  (RDP level 0, no WRP), all from the read-only receipt
  `identity/identity.json` and its raw files.
- Allowed: original repository images only; no option-byte, RDP, WRP or BOR
  writes; no ST-LINK firmware update; bounded runs; one holder at a time
  (CLAUDE.md, Hardware). `perf_event_paranoid` was `-1` in phase 1; every
  phase-2 `run.json` records the live value and nothing changes it.
- No external wiring, instruments or peers. Phase 2 can observe: bytes
  delivered by the ST-LINK virtual COM port (USB CDC ACM) with the host
  `CLOCK_MONOTONIC` time at which `read()` returned them; SWD memory reads
  through openocd without halting; target-internal counts printed by the
  images (DWT CYCCNT, SysTick).
- Not observable (stays open, never estimated): line waveforms and on-wire
  baud error, voltages and supply ramps, reset pulse widths, current or energy
  (T30.09.c), microsecond-scale wake or interrupt latency.

## 2. Findings the reviewer should weigh

1. **The identity receipt's "writes: none" statement is inaccurate.**
   `/usr/share/openocd/scripts/target/stm32f4x.cfg` (openocd 0.12.0) defines
   an `examine-end` handler (`mmw 0xE0042004 0x00000007 0`, DBGMCU_CR;
   `mmw 0xE0042008 0x00001800 0`, DBGMCU_APB1_FZ) and a `reset-init` handler
   (RCC_PLLCFGR, FLASH_ACR, RCC_CR, RCC_CFGR). The identity probe used it, so
   it wrote both DBGMCU registers (debug registers, persistent until power-on
   reset *[doc-check]*). The receipt is under `reviews/` and was not edited.
2. **An attach is not read-only.** Every openocd call here empties both
   handlers before `init`, so this layer issues no RAM or peripheral write.
   The attach itself still programs Cortex-M debug registers (DHCSR, possibly
   DEMCR; openocd/hla source not inspected); `reset run` writes DHCSR/DEMCR,
   drives NRST and sends the ST-LINK system-reset command; st-info in its
   default mode likely halts the core and writes DHCSR (reviewer note), so
   st-info runs only as `--probe --hot-plug` and only in the `probe` action.
3. **openocd script search order** (openocd.info, "Configuration Basics"):
   current directory, `-s` directories, `OPENOCD_SCRIPTS`,
   `$XDG_CONFIG_HOME/openocd` or `~/.config/openocd`, `~/.openocd`,
   `$pkgdatadir/site`, `$pkgdatadir/scripts`. An untracked `target/*.cfg`
   earlier in that list would run arbitrary Tcl. Fixed by M1 (§3).
4. **ST-LINK firmware string differs by tool**: openocd `STLINK V2J39M27`,
   st-info `V2J39S27`. Recorded, not gated.
5. **`verify_image` runs a CRC algorithm in target RAM** (work area
   `0x20000000`, 32 KiB, no backup); it is denylisted outside `program`.
   `program` (text embedded in `/usr/bin/openocd`) runs `init`, `reset init`,
   `flash write_image erase`, `verify_image`, `reset run`, `shutdown`. Every
   image loads into flash sector 0 only. Because `program` ends with
   `reset run`, the `reset-init` override is not observable through register
   readback (the review's point); the `examine-end` override is checked by
   P4.10.
6. **The T27 bring-up images keep `bu_reset_flags` (0x20000014),
   `bu_retained` (0x20000018) and `bu_stage` (0x2000001C) inside the work
   area**; programming clobbers them, which fixes the run order. The new
   harness keeps its retained record at 0x2001F000.
7. **record-physical routes.** `cmastery record-physical` requires
   `source_sha256` to equal the learner workspace snapshot. Format dry runs
   (synthetic files, receipts deleted) were accepted for PR-T38-01 /
   U-T38-01-03 (`interface-bench`, `["UART"]`) and PR-CAP-MCU-02/03/04 /
   U-CAP-MCU-0x-03 (`stm32f439`); **refused** for PR-T30-05. PR-P4-01 and
   PR-T27..T31 have no physical target either: recording against those IDs
   needs an orchestrator catalog decision.
8. **Untracked sources.** The gate refuses images whose rebuild dependencies,
   or `tools/f439_board.py` itself, are untracked or differ from HEAD. They
   must be committed before phase 2.
9. **Primary STM32/Arm documents were unavailable** (no local RM0090,
   DS9484, UM1974, PM0214, ES0206 or Armv7-M ARM; network failed). Every fact
   taken from them is unconfirmed and tagged *[doc-check]* (§9). The safety
   reviewer could not confirm them either.
10. **ModemManager is active, has no ignore rule for 0483:374B, and re-probed
    the port without a replug** (reviewer, journal 22:06:58 JST). TIOCEXCL
    does not exclude a CAP_SYS_ADMIN process. Commands need a DLE (0x10)
    prefix; captures refuse a port another visible process holds, confirm
    exclusive mode, compare termios after the capture and record the
    ModemManager journal window. ModemManager is not stopped and no udev rule
    is added: that is the user's decision.

## 3. Safety layer: `tools/f439_board.py`

Enforcement, in order:

1. **Offline gate** (before any lock or tool): the manifest names the image;
   the ELF digest matches the manifest; `check_elf` passes; the safety layer
   itself (`tools/f439_board.py`) is tracked and identical to HEAD; a fresh
   rebuild is byte-identical, its compiler dependency set (`-MMD`) equals the
   manifest's, and every one of those files is inside the repository, tracked
   and identical to HEAD (should-fix 3). The gate reads the ELF once; the
   digest, `check_elf` and the image later compared with the flash readback
   all use those bytes, so a file swapped after hashing cannot cause a false
   readback alarm (the session copy is still re-read and re-hashed, so such
   a swap is refused).
2. **`check_elf`** (own ELF32 parser): every loadable byte in main flash
   `0x08000000-0x081FFFFF`; allocated sections only in flash, SRAM
   `0x20000000-0x2002FFFF` or CCM `0x10000000-0x1000FFFF`, NOBITS only in RAM;
   option bytes, OTP and system memory named in refusals; ARM ELF32 LE, no
   wrap, entry and Thumb reset vector in flash, 8-byte-aligned initial SP in
   RAM. The build repeats the address rules from `readelf -lSW` and requires
   `nm -u` to be empty.
3. **Two locks** (`fcntl.flock`, non-blocking, both independent of
   `--state`), taken host lock first:
   - host-wide `/run/lock/cmastery-f439.lock`, shared by every checkout,
     worktree and clone on the host. `/run/lock` is the system lock
     directory (hier(7) `/var/lock`; `/usr/lib/tmpfiles.d/debian.conf`
     `d /run/lock 1777 root root`, `legacy.conf` links `/var/lock` to it).
     If `/run/lock` does not exist, `/run/user/<uid>` (the logind runtime
     directory that `$XDG_RUNTIME_DIR` names; derived from the uid, because
     pam_systemd(8) leaves the variable unset for a user who is not the
     session's original user and removes the directory at the final
     logout). The first existing candidate is used; one that exists but is
     not a real directory owned by root or the user, or is writable by
     others without being root-owned and sticky, is refused rather than
     skipped, so two processes cannot settle on different files;
   - `build/f439/board.lock` in this checkout (kept: a sandbox may give
     `/run` a private mount, the checkout is still shared).

   Each lock file is opened with `O_NOFOLLOW` and must be a single-link
   regular file of the user, closed to group and other writes, and still the
   file at that path once locked; it records pid, action, checkout and time,
   and a refusal names the holder.
4. **Preflight before the session and before every tool call:** the pinned
   files match (M1: the five scripts in the `source` closure of
   `board/st_nucleo_f4.cfg` — `board/st_nucleo_f4.cfg`,
   `interface/stlink.cfg`, `target/stm32f4x.cfg`, `target/swj-dp.tcl`,
   `mem_helper.tcl` — plus `/usr/bin/openocd` and `/usr/bin/st-info`, with
   no shadowing copy in `/usr/share/openocd/site`), and no other process
   whose program name is openocd, st-info, st-flash, st-util, st-trace or
   STM32_Programmer_CLI is running (should-fix 1).
5. **Execution environment** (M1): tools run by absolute path, with the
   working directory set to an empty per-session sandbox, the environment
   reduced to `PATH=/usr/bin:/bin`, `LC_ALL=C` and `HOME=XDG_CONFIG_HOME=`
   the sandbox, and no `OPENOCD_SCRIPTS`. Every openocd argv begins
   `/usr/bin/openocd -s /usr/share/openocd/scripts -f
   /usr/share/openocd/scripts/board/st_nucleo_f4.cfg`, then `adapter serial
   <serial>`, `gdb_port disabled`, `telnet_port disabled`, `tcl_port
   disabled` (M2) and both handler overrides, all before anything else.
6. **Command policy** on every argv: allowlist (`st-info --probe
   --hot-plug`; openocd with the fixed prefix and then only `init`,
   `shutdown`, `reset run`, the connect-under-reset `reset_config`, the
   selfcheck echoes, reads, a flash-only `dump_image` into the session
   directory, or `program {<session copy>} verify reset exit`) plus an
   independent denylist (option, lock, unlock, protect, RDP, WRP, BOR, optcr,
   OTP, mass/sector erase, memory writes, write/load_image, reg, halt, resume,
   step, verify_image, `reset halt|init`, flash, gdb other than `gdb_port
   disabled`, breakpoints, bindto, search_dir, upgrade, DFU, st-flash,
   CubeProgrammer, java, option-byte addresses). Reads (should-fix 8): main
   flash, the identity reads, the `READ_REGISTERS` allowlist (USART3 and
   SysTick CSR removed: read-sensitive), or ELF symbols in SRAM/CCM. The ST-LINK
   serial must match `^[0-9A-F]{24}$` before it reaches any argv; capture
   labels must match `[A-Za-z0-9][A-Za-z0-9._-]{0,63}` (should-fix 9).
7. **Identity** (openocd only): DBGMCU_IDCODE, UID, flash size, FLASH_OPTCR
   (must equal `0x0FFFAAED`), and FLASH_OPTCR1, DBGMCU_CR, DBGMCU_APB1_FZ and
   RCC_CSR recorded. `probe` adds `st-info --probe --hot-plug` (one programmer,
   serial, chipid, flash size). The identity receipt predates the
   FLASH_OPTCR1 read, so the first `probe` records the observed value in
   `<state>/probe-baseline.json` (schema `f439-probe-baseline/v1`, serial,
   OPTCR1, OPTCR, time, run directory; never overwritten). Every later
   `probe` refuses if FLASH_OPTCR1 differs from it, and `probe` refuses if
   FLASH_OPTCR1 was not read.
8. **Flash** (should-fixes 2 and 10): identity; the ELF is copied into the
   locked session directory, re-hashed and re-checked; the copy is
   programmed; then one attach repeats the identity and dumps the load range
   of flash (`dump_image`), which is compared byte for byte in Python with the
   ELF segments, independently of openocd's verify; FLASH_OPTCR1 must equal
   its value before programming.
9. **Recover** (M3): refused before any tool unless the probe baseline
   exists, is parsable and names this ST-LINK serial; then connect under
   reset, read only DBGMCU_IDCODE (debug domain) with the pinned adapter
   serial, program the park image's session copy under reset, then the full
   identity and readback; FLASH_OPTCR1 after programming must equal the
   baseline (as FLASH_OPTCR must equal the receipt).
10. **Bounds and children** (should-fix 7): subprocess timeouts 20-180 s;
    every child runs in its own process group and is terminated (SIGTERM,
    then SIGKILL) before the call returns, at session exit and before the lock
    is released; SIGTERM/SIGHUP unwind the CLI like Ctrl-C. Captures are
    bounded in seconds (<= 600) and bytes (<= 4 MiB); an in-capture reset
    leaves >= 5 s.
11. **Capture guards** (should-fix 6): refuse if another visible process has
    the port open; TIOCEXCL must be confirmed with TIOCGEXCL (a refusal
    releases exclusive mode with TIOCNXCL); termios is re-read after the
    capture and a change fails the capture (data kept, `run.json` status
    `termios-changed` with the capture summary, as `reset-failed` for a
    failed in-capture reset); the ModemManager journal window
    (`journalctl -u ModemManager.service`) is stored with each capture.
12. **Logs**: each step `<n>-<label>.{json,stdout,stderr}` with argv, cwd,
    environment keys and SHA-256; each session `run.json` (action, status,
    identity, both lock paths, sandbox leftovers, HEAD,
    perf_event_paranoid) and `SHA256SUMS`. A flash session's directory is
    tagged `program-<image>` so the denylisted word never appears in a
    program path.

Offline verification (author checks of phase 1; exit codes in the hand-back
report): `python3 tools/test_f439_board.py` (140 deterministic tests:
synthesized ELFs, a fake executor that records argv, working directory and
environment, pseudo-terminals, child processes, a temporary Git repository;
no hardware, and the test process refuses to start any board-tool program).
`python3 tools/test_f439_board.py --mutants` applies each entry of `MUTANTS`
at the end of that file (142 single-rule weakenings of
`tools/f439_board.py` plus the removal of each of the 38 denylist patterns
that have a probe of their own) to an in-memory copy and requires the suite
to fail; the unmutated source must pass through the same path first. Rules
deliberately not mutated are listed, with reasons, above `MUTANTS`. Result
on 2026-10-31 for `tools/f439_board.py` SHA-256
`276cc18503fc6c20ed3aa8d5dc279a965ffbc28c29b269622e901c7fe4b199aa`: 180/180
killed (the capture time bound by the 120 s timeout, the others by a
failing test). Revision 2's "53 mutants" had no harness on disk and is
withdrawn. The four controls named by the second review (preflight and
empty-sandbox check before every tool, TIOCGEXCL confirmation, termios
refusal in `Board.capture`) were also deleted in place one at a time: the
unchanged suite failed (exit 1) in exactly the test aimed at that control
each time, and the file was restored byte-identical.

## 4. Images

Build: `python3 tools/f439_board.py build` (→ `build/f439/images/<name>/`:
`image.elf`, `image.map`, `manifest.json`, compiler/readelf/nm/objdump logs).
arm-none-eabi-gcc 14.2.1 20241119, GNU ld 2.45.50.20251209; flags
`-std=c17 -O2 -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -ffreestanding
-fno-builtin -fno-stack-protector -ffunction-sections -fdata-sections -Wall
-Wextra -Werror`, plus `-Wconversion -Wsign-conversion -Wpedantic` for the new
harness C; `ld --gc-sections -z noexecstack`. Two builds in different
directories were byte-identical; all images use flash sector 0 only; none
contains `bkpt`.

| Image | Sources | Exp. | ELF SHA-256 |
|---|---|---|---|
| bu-cold-cross | existing T27 `assessor/bringup_depth/target/{reset.S,guest.c}`, `labs/bringup_depth/target/image.ld.in` (CODE 0x08000000, BANK 0x10000000), `-DBU_F439=1` | E2 | a9e0129a816122c4db352f87f77b71c8cdbe5a5d055ba40d60e10b0795ffe2dc |
| bu-software-reset-cross | same, `+BU_SOFTWARE_RESET=1` | E2 | 38929e2c3d216e45862016bfa777f03c6ee1ee3f59417a61258bd6ad3a8061f9 |
| bu-watchdog-cross | same, `+BU_WATCHDOG=1` (BOARD_CAPTURE.md watchdog-cross) | E3 | a328070f0b9a70f7bd69af744348df182542ecf65d54bf6fee4e1f3e22d249f4 |
| bu-guard-cross | same, `+BU_GUARD_FAULT=1` | X1 | a1812c62d0616d5ad6224823da4337054a594140e7b0b5c14f6135543c60293c |
| bf-report-115200 | harness `BF_MODE=1 BF_BAUD=115200` (park image) | E1, E2 | 451acb9685a7ac7c45a050979c87ecc1abfeaf89a4cab28e023e2e0a903c8a3a |
| bf-report-57600 | `BF_MODE=1 BF_BAUD=57600` | E1 | fdc4d440ab282f6f39788d0b876a273fdbeb930f61634af8f71f9245a32076f0 |
| bf-iwdg-fed | `BF_MODE=2 BF_STARVE_AFTER=0` | E3 | 937f64d80925c43370b5fd2f372e0c5f1300e397c54b038f6b6783d57ebc6b8a |
| bf-iwdg-starve | `BF_MODE=2 BF_STARVE_AFTER=10` | E3 | f2842f090088e8f3c02af2a21bb75a96f5690b0bedd970c493af98503fb8ab93 |
| bf-clock-core | `BF_MODE=3 BF_SYSTICK_DIV8=0` | E4 | 3396e76311fd9e2d8022721364a4420944ecd7bcae20946a9a2812b4aa2e61bc |
| bf-clock-div8 | `BF_MODE=3 BF_SYSTICK_DIV8=1` | E4 | af393799b1510a90a3a4c5d22222abf24443c7bcab746a2556836562b85cb711 |
| bf-stop-nodebug | `BF_MODE=4 BF_STOP_DEBUG=0` | E4 | d7f9d2a6135804fc1ab9f3ef2577c600a3c98ff408f8cd693dff7f9d87ad2d07 |
| bf-stop-debug | `BF_MODE=4 BF_STOP_DEBUG=1` | E4 | 7cbe99c7d283c3123ee20d161334738b9c38f6b2037c39027cff7724a676c748 |

Harness sources: new author firmware `assessor/board_f439/` (`startup.S`,
`board.ld`, `bf_hw.[ch]`, `bf_report.[ch]`, `bf_main.c`) with **unmodified**
reference code `assessor/peripheral_depth/uart/{uart.c,target.c}`,
`assessor/serial_depth/serial_depth.c`,
`assessor/legacy_constrained/{supervision.c,power.c,power_f439.c}` and
`assessor/execution/support.c`. Any change to these files changes the hashes
and needs re-review.

Output contract: `BF1 <TAG> name=value ...` CR LF (hex `0x` + 8 digits). All
images run on the reset clock (HSI 16 MHz nominal, no PLL, prescalers 1). At
boot each harness image records RCC_CSR and DBGMCU_CR (`dbg0`) and sets RMVF,
so a boot's `csr` reflects only the reset that started it. Host commands are
DLE (0x10) plus a letter: `S` AIRCR software reset, `B` 11520-byte `U`
burst, `R` reprint the boot block (report image); `P`/`Q` start/stop the
sleep probes (clock image); `G` start the Stop cycles (Stop image). The
receiver is polled and USART3 holds one byte, so a second byte arriving
while an image prints a line or sleeps in a probe would be overrun. The
prefix and the letter are therefore always sent as two single-byte writes,
spaced longer than the longest busy or sleep interval of that image (0.2 s
for the report and Stop images while awake, 3 s for the clock image, 10 s
for the HCLK/8 clock image). Each clock capture sends the DLE `Q` pair
twice; the second DLE follows the first `Q` by at least 9 s, longer than
the 8 s HCLK/8 tick (the longest probe sleep), so it cannot overrun a first
`Q` still unread in the receiver. A repeated `Q` is idempotent:
`probe_command` in `bf_main.c` returns the unchanged state and `run_clock`
prints `E STATE` only when the state changes, so the firmware is unchanged
and the image hashes below still apply.

DBGMCU_CR handling (M5): the report/park image sets DBG_SLEEP (bit 0) at boot,
so later images that end in WFI (bu-*) stay attachable regardless of
power-on history; the clock image changes DBG_SLEEP only for the duration of
one probe sleep and restores it on wake, and probes only between DLE `P` and
DLE `Q`; the Stop images write 0x0 or 0x2 at boot and stay awake until DLE
`G` and again after 10 cycles. Sequence in §6: after E4 the register holds
0x2 (bf-stop-debug), and the final park sets bit 0, leaving **0x3 until
power-on reset**.

## 5. Experiments

*raw* = a byte, line field or register value as delivered; *derived* =
computed by a pre-registered function (`analyze ...`); *[doc-check]* = an
unconfirmed fact from an unavailable primary document (§9). Obligation IDs
are addressed in their physical-observation dimension only, partially unless
stated; "-DELAYED" twins share that state but not their delayed-transfer
dimension. HSI tolerances: ±1 % is the datasheet figure at 25 °C
*[doc-check DS9484]*; ambient temperature is not measured, so a derived
frequency outside that band is reported with temperature as the competing
explanation, not as a falsification.

### E1 USART3 over the ST-LINK virtual COM port

- Images: bf-report-115200; changed conditions bf-report-57600 (target side)
  and host-side parity and baud.
- Arithmetic (not observation): `sd_uart(16 MHz, 115200, OVER8=0)` → divisor
  139, BRR `0x008B`, 139 APB1 cycles per bit, 115107.9 baud nominal (-0.077 %);
  at 57600: divisor 278, BRR `0x0116`, 57553.96 baud (-0.080 %).
- Captures (§6): `e1-baseline` 20 s, host 115200 8N1, sends `00` 3.0 s, `41`
  3.4 s, `ff` 3.8 s, DLE 4.2 s + `R` 4.4 s, DLE 5.0 s + `B` 5.2 s;
  `e1-parity-even` 10 s at 8E1; `e1-host-57600` 10 s; after bf-report-57600:
  `e1-t57600-h115200` 10 s and control `e1-t57600-h57600` 12 s with DLE 3.0 s
  + `B` 3.2 s.

| ID | Kind | Prediction (tolerance) | Competing explanation |
|---|---|---|---|
| P1.1 | raw | `e1-baseline`: >= 15 `R` lines; consecutive `seq` differ by 1; `tx=0`; one `csr` throughout | bridge loss; driver poll budget exhausted (`tx>0`) |
| P1.2 | raw | the three bytes appear as `RX` lines in order (`v=0x00000000`, `0x00000041`, `0x000000ff`, each `e=0x00000000`) as a subsequence of all `RX` lines. Every other `RX` line is listed by `analyze uart` (time, value, error) as *extra*; extra printable bytes are attributed to a host prober only if that capture's ModemManager journal window shows activity at that time, otherwise *unexplained*. Extras do not change the verdict; a missing, reordered or errored expected byte fails P1.2 | `e=0x00000008` (ORE): bytes faster than the polled reader; FE/NF: bridge or line |
| P1.3 | raw | DLE `R` reprints `BOOT`/`CHK`/`CLK`; `CHK lma=1 bss=1 vec=1 v0=0x2001f000 v1=0x080001ad vtor=0x08000000`; `memrmp=0x00000000` *[doc-check: MEM_MODE=00, BOOT0 low]* | `vec=0`: wrong vector table; `lma=0`: .data copy fault |
| P1.4 | raw | one contiguous run of exactly 11520 `0x55` and `BURST bytes=11520` | bridge loss under load |
| P1.5 | raw (target count) | `BURST cycles` in [16 012 800, 16 973 568]: 11520 frames x 10 bits x 139 cycles is a floor; +6 % for the per-8-byte TC wait | below the floor: USART3 kernel clock is not the core clock or BRR is not 0x8B |
| P1.6 | derived (`analyze uart`) | host span of the run in [0.95, 1.12] s (cycles above at 16 MHz ±1 %, ±20 ms delivery) | temperature-dependent HSI error, or bridge buffering; then inconclusive pending E4 |
| P1.7 | raw | `e1-t57600-h115200`: zero parseable `BF1` lines; byte count and TIOCGICOUNT counters recorded, not predicted | a parse means the rates are not as declared |
| P1.8 | raw | control: >= 7 `R` lines, consecutive `seq`; `BURST bytes=11520`, cycles in [32 025 600, 33 947 136] | 57600 image fault |
| P1.9 | raw | `e1-parity-even`: fewer than half the baseline's `R` lines per 10 s parse, or icount parity/frame counters rise, or bytes are missing or altered relative to 8N1 text. PARMRK is set in termios but no marker is expected: the bridge's error reporting is unknown | every line parses unchanged: the bridge did not apply the requested parity (a bridge observation) |
| P1.10 | raw | `e1-host-57600` (target 115200): zero parseable `BF1` lines | as P1.7 |

Not measurable: on-wire bit period, baud error and edges (decoding is done by
the ST-LINK's USART); bridge error handling (cdc-acm source not inspected).

Obligations: AX-PR-T28-02-T28.05.a (waveform accuracy stays instrument-only),
AX-PR-T28-02-T28.03.a and .b, X-T38-002 and X-T38-002-V1 (SR-then-DR receive on
F439 with a host-side parity change; target-side word/parity combinations not
exercised), AX-PR-CAP-MCU-01-T20.01.b (support only). AX-PR-T38-01-T38.01.b
(CCM DMA) is **not** addressed. physical-evidence.md UART row: partial.

record-physical (route verified): `cmastery init PR-T38-01 <ws> --unit
U-T38-01-03`; `target: "interface-bench"`, `interfaces: ["UART"]`,
`board_revision` (operator-read label or `"unknown"`), `silicon_revision`
(`"DBGMCU_IDCODE 0x20036419 (REV_ID 0x2003) from run <dir>"`), `clocks` (raw
`CLK` line), `cache_mpu` (`"Cortex-M4F, no data cache; FLASH_ACR=<raw>; MPU
unused"`), `instrument` (`"none: ST-LINK VCP decoded bytes, host
CLOCK_MONOTONIC at read(); openocd 0.12.0 SWD reads"`),
`instrumentation_overhead` (report lines take 5-20 ms each at 115200;
timestamps are read() return times), `trace_overruns` (stop_reason, icount,
`tx`, termios_unchanged, ModemManager journal entries), `observations` (raw
values per prediction ID; derived estimates labelled; `provenance`: HEAD,
image manifest SHA-256, and that the image was built from repository author
sources, not the workspace), `source_sha256` (workspace snapshot, as cli.py
requires), `images` (`image.elf`, `manifest.json`), `captures` (`<label>.bin`,
`.index.jsonl`, `.json`, journal file, `run.json`, `SHA256SUMS`, step logs).

### E2 Reset causes and bring-up

RCC_CSR bits per BOARD_CAPTURE.md's RM0090 citation: BORRSTF 25, PINRSTF 26,
PORRSTF 27, SFTRSTF 28, IWDGRSTF 29, WWDGRSTF 30 (LPWRRSTF 31 *[doc-check]*).
Flags are sticky and not mutually exclusive. **Internal resets (software,
watchdog) drive NRST low through the reset pulse generator *[doc-check
RM0090 reset circuit]*, so PINRSTF may be set on every reset.** The
debugger reset (ST-LINK drives NRST and sends its system-reset command) and
the AIRCR software reset may then be distinguishable only by the procedure
logs (which command ran, and when) and the `ACK S` line, unless SFTRSTF
differs between them; the predictions keep both outcomes.

| ID | Step | Prediction | Competing explanation |
|---|---|---|---|
| P2.0 | `probe` before any flash | RCC_CSR recorded raw (history unknown); DBGMCU_CR `0x00000007`, APB1_FZ `0x00001800` (Finding 1) | `0x0`/`0x0`: power-cycled since the identity probe |
| P2.1 | `e2-program-reset` (bf-report after other bf images) | `R csr`: bit 28 or bit 26 set (at least one); bits 25, 27, 29, 30 = 0; `dbg0` recorded | both 26 and 28 clear: the reset did not come from NRST or a system-reset request |
| P2.2 | `e2-debugger-reset` (`--reset-at 2`) | a `BOOT` line within 3 s after the reset; `boots` +1; `ret=1`; `h0` = previous `csr`; bit 26 or 28 set, bits 25/27/29/30 = 0 | no BOOT: reset not applied or attach failed (step log) |
| P2.3 | `e2-software-reset` (DLE `S` at 2.0 s) | `ACK S` then `BOOT` with bit 28 = 1, bit 26 recorded, bits 25/27/29/30 = 0, `boots` +1, `ret=1` | bit 28 clear: AIRCR reset not recorded as SFTRSTF |
| P2.5 | `read bu-cold-cross` | `bu_stage = 0x00000005`; `--vectors 107` equals the ELF (`vectors_match_elf: true`; first words `0x20010000`, `0x080001ad`); SCB_VTOR `0x08000000`; `bu_reset_flags` bit 26 or 28 set | stage 3: a startup check failed (diagnose from code) |
| P2.6 | `read bu-software-reset-cross` | `bu_stage = 5`; `bu_retained = 0x42553237`; `bu_reset_flags` bit 28 = 1 (sticky: this image never clears flags) | stage 3 with marker: the reset did not occur |
| X1 | `read bu-guard-cross` | `bu_stage = 0x00000003`; SCB_CFSR low byte `0x82` (MMARVALID, DACCVIOL); SCB_MMFAR `0x2000F7FC` (`_stack_low` - 4); SCB_SHCSR bits 16 and 0 set; SCB_HFSR `0x0` | HFSR FORCED: MemManage escalated |
| P2.4 | optional `e2-power-on`, **after the final park** (operator USB replug, explicit user approval) | `R csr` bit 27 = 1 and bit 25 = 1 *[doc-check]*; `dbg0=0x00000000` (DBGMCU_CR reset by power-on only *[doc-check]*); then `probe` DBGMCU_CR `0x00000001` (set by the park image), APB1_FZ `0x0` | `ret=1` after power loss is possible but not expected |

Obligations: AX-PR-T27-01-T27.02.b (with E3), AX-PR-T27-01-T27.01.a and
T27.02.a (board marking needs visual inspection), AX-PR-T27-02-T27.03.a and
.b, AX-PR-T27-03-T27.05.b and .c, AX-PR-T27-03-T27.07.b, AX-PR-T27-04-T27.08.b
and .c, AX-PR-P4-01-T27.08.b, AX-PR-T27-05-T27.09.a and .c,
AX-PR-T27-05-T27.10.a and .b (T27.10.c only if P2.4 runs),
AX-PR-CAP-MCU-02-T27.10.b, AX-PR-CAP-MCU-02-T31.08.b. X1: AX-PR-T27-02-T27.04.a
and the PR-T31-02 physical state (MPU MemManage only).

record-physical: `PR-CAP-MCU-02 --unit U-CAP-MCU-02-03` (`target:
"stm32f439"`), fields as E1 with `instrument: "none: SWD reads via openocd
0.12.0 (handlers disabled) and VCP bytes"`. No route for the T27 IDs.

### E3 Independent watchdog

- E3a bu-watchdog-cross (BOARD_CAPTURE.md): first boot captures RCC_CSR,
  enables LSI, writes PR=6 (/256) and RLR=511 (100000-poll bounds), sets the
  marker and RMVF, starts and stops feeding; the IWDG reset's boot needs bit
  29 and the marker to reach stage 5, then busy-loops feeding a stopped
  watchdog (no WFI). Pre-condition: bf-report-115200 boots first, which
  clears IWDGRSTF.
- E3b bf-iwdg-fed / bf-iwdg-starve: progress epochs from
  `assessor/legacy_constrained/supervision.c`; every 10 ms tick advances task
  0 and, unless starving, task 1; every 50 ticks `cs_iwdg_feed` feeds only if
  both advanced. `cs_iwdg_start(6, 511)`. The starving image stops task 1
  after window 10 while SysTick keeps running.

| ID | Kind | Prediction | Competing explanation |
|---|---|---|---|
| P3.1 | raw (SWD, >= 20 s after flashing) | `bu_stage = 5`, `bu_retained = 0x57443237`, `bu_reset_flags` bit 29 = 1, bits 25/27/28/30 = 0 (26 either); IWDG_SR `0x0`; APB1_FZ and DBGMCU_CR recorded | stage 3, bit 29 = 0: the PR/RLR update before start never completed (this image writes PR/RLR before `0xCCCC`): a finding about the image |
| P3.2 | raw (`e3-fed`, 70 s, reset at 2 s) | `BOOT` and `W START init=0 start=0` after the reset; >= 120 `W` lines, `win` consecutive, every `feed=0` (CS_OK), `starving=0`, `start=0`; no further `BOOT` | a `BOOT` with bit 29: feeding interval or window logic wrong |
| P3.3 | raw (`e3-starve`, 45 s, reset at 2 s) | cycles: windows 1-11 `feed=0`, >= 12 `feed=1` (CS_WAIT) `starving=1`, then `BOOT` with bit 29 = 1, bits 25/27/28/30 = 0, `ret=1`, `boots` +1; >= 3 cycles | no reset: IWDG not running (`start` != 0) |
| P3.4 | derived (`analyze starvation`) | last fed `W` to next `BOOT`: [2.75, 7.75] s from (511+1) x 256 / f_LSI with f_LSI 17-47 kHz *[doc-check DS9484]*, -12 ms line print, +5 ms boot print; nominal 4.10 s; f_LSI = 131072 / interval with cycle-to-cycle spread | outside: LSI out of range, or IWDG frozen by a halt (none issued) |

Obligations: AX-PR-T30-05-T30.08.a and .b, AX-PR-T30-05-T30.08.c (the
"at most 3 recovery attempts then safe state" policy is not implemented, so
that aspect stays open), AX-PR-CAP-MCU-03-T31.10.c, AX-PR-T27-01-T27.02.b.
record-physical: `PR-CAP-MCU-03 --unit U-CAP-MCU-03-03` (`stm32f439`).

### E4 Clock, SysTick/DWT, WFE latch and Stop

| ID | Kind | Prediction | Competing explanation |
|---|---|---|---|
| P4.1 | raw (`CLK`, every bf image) | RCC_CR & 0x3 = 0x3; (RCC_CR >> 3) & 0x1F = 0x10; RCC_CR bit 24 = 0; RCC_CFGR `0x00000000`; RCC_PLLCFGR `0x24003010`; FLASH_ACR `0x00000000` *[doc-check reset values]* | another value: the reset values differ from the cited ones (re-inspect RM0090) |
| P4.2 | raw (`e4-clock-core`) | `T START syst=0x00000007 reload=15999999`; `T` lines `skipped=0`, `n` consecutive; for even `n` after DLE `P` and for all `n` before it, `d = 16 000 000 ± 16` | systematic offset: CYCCNT and SysTick not on one clock |
| P4.3 | derived (`analyze ticks --divider 1`) | core clock in [15.84, 16.16] MHz; residual RMS reported as the timing uncertainty (expected <= 3 ms) | temperature-dependent HSI error; host clock |
| P4.4 | raw (`E` lines) | probes appear only between `E STATE probing=1` and `probing=0`; exactly one `E STATE probing=0` line, whichever of the two DLE `Q` pairs took effect (revision 3); `latched` <= 40 cycles (SEV latches the event register *[doc-check Armv7-M]*); `dbg` bit 0 follows 0,0,1,1 per probe; `restored` equals the `CLK` `dbg` value | `latched` large: WFE slept despite the latched event; `restored` differs: restore failed; no `probing=0` line: both `Q` pairs lost or the line lost in transit (then `analyze probing` exits 1 and §7 applies) |
| P4.5 | raw (no directional prediction) | for each of WFE/WFI x DBG_SLEEP 0/1: H-count `slept` in [`cvr`, `cvr` + 400] or H-stop `slept` <= 1000; odd-`n` `T` deltas must agree | neither: inconclusive |
| P4.6 | raw + derived (`e4-clock-div8`, `--divider 8`) | `syst=0x00000003`; even-`n` `d = 128 000 000 ± 64`; core clock in [15.84, 16.16] MHz and within 3 combined standard errors of P4.3 (SysTick reference HCLK/8 *[doc-check RM0090 clock tree]*); H-count variant `slept` near 8 x `cvr` | ratio not 8: the external reference is not HCLK/8; temperature drift between runs |
| P4.7 | raw (`e4-stop-nodebug`) | `S READY dbg=0x00000000` until DLE `G`; 10 `ENTER`/`WOKE` pairs, `exti` increasing, after wake `cr & 3 = 3` and `cfgr = 0x00000000`, then `S DONE`. Per cycle: `entered=0 finished=0` (CS_OK) and the `WOKE` line arrives after its wake byte | `entered=3`: a `cs_stop_enter` premise failed, Stop not entered |
| P4.8 | derived (`analyze stop`), **only for cycles that satisfy P4.7** (`valid: true`) | `cycles_per_host_s` <= 160 000 (< 1 % of 16 MHz): the core clock stopped during Stop; cycles with `valid: false` are reported inconclusive | ~16 MHz on a valid cycle: clocks kept running |
| P4.9 | changed condition (`e4-stop-debug`) | `dbg=0x00000002`, `entered=finished=255` (raw path); for valid cycles `cycles_per_host_s` in [0.97, 1.03] x 16 MHz (DBG_STOP keeps FCLK/HCLK on HSI *[doc-check RM0090 DBGMCU_CR]*) | far below: DBG_STOP keeps debug alive but not the CYCCNT clock; temperature |
| P4.10 | raw (`read bf-stop-nodebug` after its capture) | DBGMCU_CR `0x00000000`: the `examine-end` override held for the attach; `0x00000007` would mean the handler ran | another value: an unexpected writer |

USART3 SR/DR after a Stop wake are recorded only. Not measurable: Stop or
Sleep current and energy (T30.09.c stays open), wake latency below ~ms,
regulator-mode effects.

Obligations: AX-PR-T30-05-T30.09.a, AX-PR-T30-05-T30.09.b, AX-PR-T30-04-T30.10.c
and AX-PR-CAP-MCU-04-T30.10.c; AX-PR-T30-05-T30.09.c is **not** addressed.
record-physical: `PR-CAP-MCU-04 --unit U-CAP-MCU-04-03` (`stm32f439`).

### Safety-layer logs as observations

Each session records the SWD transport, reset strategy, probe firmware
strings, openocd command and pinned configuration digests, target IDs and the
ST-LINK's own target-voltage reading, and classifies connection, program and
reset outcomes: support for AX-PR-T31-01-T31.01.a, .b and .c (the voltage is
the probe's reading, not an instrument).

### Further F439-alone observations from the PR-*.json scan

Scanned `curriculum/v5/expansion/legacy_assessments/PR-{CAP-MCU-0[1-4],
P4-0[12],T27-*,T28-*,T29-*,T30-*,T31-*}.json` against the inventory's
unclassified items. Satisfiable by these images (physical dimension, partial)
and folded in above: T27.01.a, T27.02.a/b/c, T27.03.a/b, T27.05.b/c,
T27.07.b, T27.08.b/c, T27.09.a/c, T27.10.a/b/c, T28.03.a/b, T28.05.a,
T30.10.c, T31.01.a/b/c, T31.10.c, CAP-MCU-02 T27.10.b/T31.08.b, CAP-MCU-04
T30.10.c. Need a separately reviewed procedure (halting and watchpoints are
denylisted): AX-PR-T31-01-T31.02.b, AX-PR-T31-03-T31.06.c,
AX-PR-T29-01-T29.01.a-c. Not satisfiable without instruments or peers:
T28.06.c, T29.10.b, T30.09.c, T31.05.c. Not addressed: AX-PR-T29-04-T29.08.b,
AX-PR-T38-01-T38.01.b.

## 6. Operator command list (phase 2)

Run only after the second review, after the orchestrator has committed the
reviewed sources (including `tools/f439_board.py`), and with the user's
go-ahead for hardware. Stop at the first refusal, identity mismatch,
FLASH_OPTCR/OPTCR1 change, program, verify or readback failure, termios
change, failed in-capture reset or non-zero `analyze probing`; do not retry;
go to §7.

```sh
cd /home/fred/dev/c_mastery
export TMPDIR=/home/fred/.cache/cmastery-tmp/b6
B="python3 tools/f439_board.py"
last() { ls -d build/f439/runs/*-capture-"$1" | tail -1; }

# 0  offline (no hardware)
python3 tools/test_f439_board.py
python3 tools/test_f439_board.py --mutants   # exit 0: every mutant killed
$B build                       # every ELF hash must equal section 4
$B --dry-run flash bf-report-115200   # must plan, not refuse
$B selfcheck-openocd           # openocd parses config, no init; both markers empty

# 1  identity and pre-state
$B probe                       # P2.0; st-info --probe --hot-plug + openocd reads;
                               # records FLASH_OPTCR1 in build/f439/probe-baseline.json
$B dump-flash                  # optional: first 16 KiB of the existing firmware, never re-flashed

# 2  E1. The first park programming uses `recover`, so its first use is on a
#    known-good board, not an emergency: refused unless the step-1 baseline
#    exists; connect under reset, DBGMCU_IDCODE only, program bf-report-115200
#    under reset, then identity + readback; run.json: status completed,
#    readback_match true, identity_after.optcr1 == optcr1_baseline.optcr1
$B recover
$B capture e1-baseline --seconds 20 --max-bytes 1048576 \
   --send 3.0:00 --send 3.4:41 --send 3.8:ff --send 4.2:10 --send 4.4:52 \
   --send 5.0:10 --send 5.2:42
$B capture e1-parity-even --seconds 10 --max-bytes 262144 --parity even
$B capture e1-host-57600 --seconds 10 --max-bytes 262144 --baud 57600
$B flash bf-report-57600
$B capture e1-t57600-h115200 --seconds 10 --max-bytes 262144 --baud 115200
$B capture e1-t57600-h57600 --seconds 12 --max-bytes 262144 --baud 57600 \
   --send 3.0:10 --send 3.2:42
for c in e1-baseline e1-parity-even e1-host-57600 e1-t57600-h115200 e1-t57600-h57600; do
  $B analyze uart "$(last $c)/$c.bin"; done

# 3  E2 (+X1)
$B flash bf-report-115200      # also sets DBG_SLEEP for the bu-* WFI loops below
$B capture e2-program-reset --seconds 6 --max-bytes 65536
$B capture e2-debugger-reset --seconds 12 --max-bytes 65536 --reset-at 2
$B capture e2-software-reset --seconds 10 --max-bytes 65536 --send 2.0:10 --send 2.2:53
$B flash bu-cold-cross
$B read bu-cold-cross --symbol bu_stage --symbol bu_reset_flags --symbol bu_retained \
   --vectors 107 --register SCB_VTOR --register RCC_CSR
$B flash bu-software-reset-cross
$B read bu-software-reset-cross --symbol bu_stage --symbol bu_reset_flags --symbol bu_retained --register RCC_CSR
$B flash bu-guard-cross
$B read bu-guard-cross --symbol bu_stage --register SCB_CFSR --register SCB_MMFAR \
   --register SCB_SHCSR --register SCB_HFSR

# 4  E3
$B flash bf-report-115200      # clears sticky flags; DBG_SLEEP stays set
$B flash bu-watchdog-cross
sleep 20
$B read bu-watchdog-cross --symbol bu_stage --symbol bu_reset_flags --symbol bu_retained \
   --register RCC_CSR --register IWDG_SR --register DBGMCU_APB1_FZ --register DBGMCU_CR
$B flash bf-iwdg-fed
$B capture e3-fed --seconds 70 --max-bytes 1048576 --reset-at 2
$B flash bf-iwdg-starve
$B capture e3-starve --seconds 45 --max-bytes 1048576 --reset-at 2
$B analyze starvation "$(last e3-starve)/e3-starve.bin"

# 5  E4 (probes only between DLE P and DLE Q; no attach while probing). Each
#    clock capture sends DLE Q twice, the second DLE >= 9 s after the first Q.
#    `analyze probing` must exit 0 (last E STATE line probing=0, no probe line
#    after it) before the next flash; exit 1: do not flash, go to section 7.
$B flash bf-clock-core
$B capture e4-clock-core --seconds 90 --max-bytes 1048576 --reset-at 2 \
   --send 6.0:10 --send 6.5:50 --send 66.0:10 --send 69.0:51 \
   --send 78.0:10 --send 81.0:51
$B analyze probing "$(last e4-clock-core)/e4-clock-core.bin"
$B analyze ticks "$(last e4-clock-core)/e4-clock-core.bin" --reload 15999999 --divider 1
$B flash bf-clock-div8
$B capture e4-clock-div8 --seconds 165 --max-bytes 1048576 --reset-at 2 \
   --send 11.0:10 --send 11.5:50 --send 120.0:10 --send 130.0:51 \
   --send 139.0:10 --send 149.0:51
$B analyze probing "$(last e4-clock-div8)/e4-clock-div8.bin"
$B analyze ticks "$(last e4-clock-div8)/e4-clock-div8.bin" --reload 15999999 --divider 8
WAKE="--send 2.0:10 --send 2.3:47"; for t in 5 8 11 14 17 20 23 26 29 32; do WAKE="$WAKE --send $t.0:00"; done
$B flash bf-stop-nodebug
$B capture e4-stop-nodebug --seconds 45 --max-bytes 262144 $WAKE
$B analyze stop "$(last e4-stop-nodebug)/e4-stop-nodebug.bin"
$B read bf-stop-nodebug --register DBGMCU_CR --register DBGMCU_APB1_FZ   # P4.10
$B flash bf-stop-debug
$B capture e4-stop-debug --seconds 45 --max-bytes 262144 $WAKE
$B analyze stop "$(last e4-stop-debug)/e4-stop-debug.bin"

# 6  final known state
$B park
$B capture final-park --seconds 5 --max-bytes 65536
$B probe                       # identity, FLASH_OPTCR 0x0FFFAAED, FLASH_OPTCR1 = step-1 baseline; DBGMCU_CR 0x3 expected

# 7  optional, only with explicit user approval: power-on (P2.4)
#   unplug CN1 USB >= 5 s, replug, wait for
#   /dev/serial/by-id/usb-STMicroelectronics_STM32_STLink_0669FF485753667187124334-if02, then:
#   $B capture e2-power-on --seconds 8 --max-bytes 65536 && $B probe
```

About 30 minutes, 15 program cycles (one of them by `recover` in step 2),
all of sector 0. Raw run directories stay
under `build/f439/runs/`; the orchestrator decides what to copy into this
packet. Every capture records its ModemManager journal window; an entry inside
a window is reported with that capture.

## 7. Rollback and known state

- Known state: bf-report-115200 programmed, read back and running from reset
  clocks (step 6), identity, FLASH_OPTCR and FLASH_OPTCR1 unchanged,
  DBGMCU_CR 0x3 until power-on reset. The pre-existing firmware is not
  restored (not a repository image); step 1 may dump and hash its first
  16 KiB.
- Attach fails, or `analyze probing` exits 1 after a clock capture: `$B
  recover` (connect under reset, DBGMCU_IDCODE only, program the park image
  under reset, then full identity and readback; FLASH_OPTCR1 compared with
  the step-1 baseline in `build/f439/probe-baseline.json`, without which
  `recover` refuses before any tool). Step 2 has already run `recover` once
  on a known-good board. In the normal sequence no image sleeps with its
  debug clock off when an attach happens: the clock image probes only
  between DLE `P` and `Q` (sent twice, confirmed by `analyze probing`), and
  the Stop images are awake before DLE `G` and after their 10 cycles.
- `recover` fails: stop; removing USB power is the operator's last resort,
  then `$B probe` and `$B park`.
- No image or command writes the backup domain (RTC, RCC_BDCR) or option
  bytes.

## 8. Risks

1. The handler override has never run; `selfcheck-openocd` (no `init`)
   checks both bodies first. If it failed, attaches would write the two
   DBGMCU registers again; P4.10 detects that.
2. openocd/hla reset behaviour and the attach's own debug-register writes are
   recalled, not inspected (Finding 2); E2 predictions keep both outcomes.
3. ModemManager can open the port as root despite TIOCEXCL, and its root
   processes are invisible to the holder scan. Evidence of interference: the
   journal window, termios changes (fail the capture), extra `RX` lines.
4. Nothing in `tools/f439_board.py` has run against openocd or st-info;
   argument mistakes surface as tool errors (refusals, no writes).
5. Image hashes depend on the reused assessor sources; a later edit needs a
   rebuild and review.
6. Host timestamps are read() return times through an undocumented bridge;
   derived timing carries regression residuals or stated offsets as
   uncertainty, and nothing below ~ms is claimed.
7. record-physical ties captures to a learner workspace snapshot; image
   provenance goes in `observations.provenance`.
8. Flash wear: 15 cycles of sector 0 *[doc-check DS9484 endurance]*.
9. A lost command letter (for example `Q`, leaving the clock image
   probing) could make the next attach meet a core asleep with DBG_SLEEP
   cleared. Each clock capture therefore sends DLE `Q` twice (§4), and
   `analyze probing` must exit 0 before the next flash; otherwise `recover`
   (§7), which step 2 has already exercised.
10. **Accepted residual risk: same-user tampering.** Another process running
    as this user can write into the `mkdtemp` sandbox (mode 0700, owned by
    the user) or replace the session copy of the ELF between the checks
    (empty sandbox, re-hash and `check_elf` of the copy) and openocd
    starting, delete a lock file while it is held, or edit
    `probe-baseline.json` between sessions. The checks narrow
    that window but cannot close it. The independent flash readback detects
    a swapped image, but only after it has been programmed and has run.
    Isolating the board session from other processes of the same user (a
    separate account or container) is out of scope.
11. The locks are advisory: only this tool takes them. Other board tools
    are caught only by the name-based process scan (a renamed binary is
    not). Another local user can pre-create
    `/run/lock/cmastery-f439.lock`; every session then refuses until the
    file is removed or the host reboots (availability, not safety).

## 9. Document checks before phase 2

Inspected in phase 1: openocd 0.12.0 installed scripts (the five pinned files)
and the `program` procedure embedded in `/usr/bin/openocd`; openocd.info
(script search order, `-s`, `adapter serial`, `gdb_port`/`telnet_port`/
`tcl_port disabled`, `program`, `reset`, `mdw`, `dump_image`, `verify_image`,
stm32f2x commands, `cget -event`, target events); st-info 1.8.0 usage text
(`--hot-plug`, by `strings`); Linux man-pages 6.17 `termios(3)`,
`TIOCEXCL(2const)`, `TIOCGICOUNT(2const)`, `/usr/include/asm-generic/
ioctls.h` (TIOCGEXCL `_IOR('T', 0x40, int)`) and `/usr/include/linux/
serial.h`; repository `cmastery/cli.py`, `labs/bringup_depth/
BOARD_CAPTURE.md`, `curriculum/guides/physical-evidence.md`, the identity
receipt and the reused assessor sources. Added in revision 3: hier(7)
(`/var/lock`), `/usr/lib/tmpfiles.d/debian.conf` and `legacy.conf`
(`/run/lock` 1777 root, `/var/lock` link), pam_systemd(8)
(`$XDG_RUNTIME_DIR` lifetime and when it is unset), open(2) (`O_NOFOLLOW`),
flock(2) (locks belong to the open file description), proc_sys_fs(5)
(`protected_regular`), and `assessor/board_f439/bf_main.c`
(`probe_command`, `run_clock`: a repeated `Q` changes nothing and prints
nothing).

Unavailable, therefore every dependent fact is unconfirmed and must be
re-inspected before execution:

- RM0090 Rev21: RCC_CSR bit map (including LPWRRSTF) and reset scope;
  internal resets driving NRST; RCC_CR, RCC_CFGR, RCC_PLLCFGR, FLASH_ACR and
  PWR_CR reset values; Stop entry bits; DBGMCU_CR and APB1_FZ semantics and
  power-on-only reset; IWDG register protection and update ordering
  (repository citation RM0090-SLEEP 21.1-21.3); SysTick external reference
  HCLK/8; SYSCFG_EXTICR3 and EXTI9_5 at IRQ 23; SYSCFG_MEMRMP MEM_MODE.
- DS9484 Rev14: HSI accuracy and its temperature range, LSI
  minimum/typical/maximum, flash endurance.
- UM1974: VCP on PD8/PD9 through SB5/SB6 (repository citation Rev11 Table 12
  p30), BOOT0 default, ST-LINK NRST connection, IDD jumper.
- PM0214 / Armv7-M Architecture Reference Manual: event register and WFE
  wake conditions, DWT CYCCNT in sleep, debug register effects of an attach.
- ES0206: errata touching IWDG, Stop or debug for REV_ID 0x2003.
