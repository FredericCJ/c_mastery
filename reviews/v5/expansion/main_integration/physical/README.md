# NUCLEO-F439ZI observation, 20 November 2026

The physical run used the author bench and the ST-LINK virtual serial port. The 57,600-baud course ELF was built twice from this checkout. Its hash matches the historical E1 executable. All twelve offline builds and their hashes are listed in [inputs.json](inputs.json).

The two independent pre-programming reads covered all 16,384 bytes of sector 0. After programming, OpenOCD verified the trial ELF and an independent SWD read matched its loadable segments. The first two vector words were 0x2001f000 and 0x080001ad. A read at SRAM address 0x20000000 returned the initialized C object value 0x5aa5c33c. The device reported successful data initialization, BSS clearing and vector checks over its USART3 virtual port. The four-second serial capture includes raw bytes and host monotonic offsets. Two unsolicited receive records precede the requested startup report. Their origin was not established. [Run record](run.json) · [SWD read](06-identity.stderr) · [UART metadata](trial-uart.json)

OpenOCD then programmed the original full sector and reset the target. A final SWD read of the complete sector matched both pre-programming reads. All three SHA-256 digests are `d967eefeac0c67c65f5e3b1e7503854facd0485e079bed8ff25238e3bb0d0863`. Identity and both option registers matched throughout. The last debug status sample showed a running core. The original firmware bytes and recovery ELF are retained in the operator's private recovery directory. [Restoration log](08-program.stderr) · [Final read](09-identity.stderr)

This receipt covers a single board, one course executable and one reboot into the restored firmware. The October bench runs retain their own dates and scope. Learner performance, native Linux PMU event counts and wall-clock accuracy require separate evidence.

To verify the archived files, run `sha256sum -c SHA256SUMS` from this directory. Exercise the rollback fixtures with `python3 -m unittest discover -s tools -p 'test_f439_restore.py'` from the repository root. [Bench instructions](../../../../../assessor/board_f439/README.md)
