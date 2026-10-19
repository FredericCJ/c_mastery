# PR-T38-01 scoped author audit

The original six criteria and six delayed tasks distinguish architecture,
placement and evidence from framing/baud/status. The retained serial_depth
models covered discrete frames but did not satisfy the requested real producer
composition. The unchanged PuDriver/PuRx and F439 adapter now supply actual
configuration, pin mapping, DMA aperture and separate memory/line ownership.
This successor adds a bounded16-byte aggregate and32-slot error-preserving
receive adapter; it does not replace original criteria or historical receipts.

Actual inspected code: uart.h, retained uart.c/target.c and contracts; serial
codec source/contracts; full PR-T38-01 supplement/project/fixture; the imported
frame reset/linker and explicit byte helpers; the new implementation and tests.
Fresh primary inspection includes RM0090 Rev21 pp68,973–977,981,991,1004,
1010–1017; DS9484 Rev14 Table12 pp78–79; TI MAX3232 RevO pp1,3–6,9;
MCP2515 DS20001801K pp1,3–4,15–16; AN4839 Rev2 pp3–6 inspected earlier in this
same author session. Follow-up ST URL reads intermittently returned Internal
Error; private curl attempts timed out. No private AN4839 hash is invented.
The original F7 DTCM example does not establish H745 routing.

The AN4839 model deliberately covers one write-back line and four consistency
counterexamples. It supplies changed-condition analysis, not target cache
maintenance execution. SPI-to-CAN remains a source-backed boundary analysis,
not a fabricated complete external-controller port. Physical UART/RS232
timing, independent review and delayed learner performance remain unobserved.
No other T38 project or mixed LRUN parent closes from this package.
