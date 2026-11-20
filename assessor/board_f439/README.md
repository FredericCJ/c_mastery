# NUCLEO-F439ZI author bench

The board exercises use an STM32F439 Cortex-M4, its on-board ST-LINK and the ST-LINK virtual serial port. This checkout includes the original startup/UART/clock/supervision bench. Its DWT cycle counter measures target core cycles. The Linux PMU exercises use host counters and have separate evidence requirements.

From the repository root, build all twelve images and run the offline board tests.

```sh
python3 tools/f439_board.py build
python3 -m unittest discover -s tools -p 'test_f439*.py' -q
```

The following sequence runs one observed change of baud rate on the paired board. The command checks the pinned OpenOCD and st-info tools, serial and MCU identity before use. It independently rebuilds the exact previously measured course executable.

```sh
python3 tools/f439_board.py selfcheck-openocd
python3 tools/f439_board.py probe
python3 tools/f439_board.py roundtrip
```

The transaction holds the board lock throughout. It reads sector 0 (0x08000000 through 0x08003fff) twice and saves the matching bytes, a single-segment recovery ELF and their hashes in a private transaction directory under `~/.local/state/c-mastery/f439/<serial>/`. It programs the 57,600-baud report, compares its load bytes with a separate SWD read, reads the startup canary in SRAM and requests the startup report over the virtual port. It then restores the original sector, resets the target through OpenOCD, reads all 16,384 bytes back and compares them to the saved bytes. The final identity read resumes the core. Reset restarts firmware and changes volatile execution state.

After an interrupted run, reconnect the same board under the same user and run `python3 tools/f439_board.py restore-original`. This resumes the pending journal and checks the original UID, option registers, backup digest and complete payload before restoring. The journal blocks other sessions until the verified restoration completes. The host backup remains available afterward. The historical `flash`, `park` and `recover` commands install persistent images. Use the round-trip command for a temporary trial.

See the [measured trial](../../reviews/v5/expansion/main_integration/physical/README.md) and [historical bench protocol](../../reviews/v5/expansion/polish_revision/physical/PLAN.md).
