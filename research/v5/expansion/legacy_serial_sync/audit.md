# T38.03 actual producer extension

The original PR-T38-02 project, three AX criteria and seven-day V1 tasks, full
T38.03 teaching and assessor discriminators were inspected. The prior serial_depth
contract and actual sd_sync/sd_uart bodies supply finite edge/framing calculations.
They do not write registers or perform a synchronous exchange. The retained
PuDriver/PuRx service and F439 adapter were read and are imported unchanged.
They configure asynchronous 8N1 and supply bounded transmit, SR-before-DR receive,
clock-generation and separate source-buffer/line ownership mechanisms.

The new Su adapter adds guarded CPOL/CPHA/LBCL configuration, one-write TE/RE
enable, explicit clock-generating transmit for each received byte, an eight-byte
matched exchange and original deadline/error cleanup. Each byte uses the real
PuDriver and PuRx operations. A RAM-register fixture exercises actual F439 target
adapter writes and the new PD10 CK setup. MPS2 executes those same C operations
with imported reset/vector assembly unchanged; it is not an F439 USART simulator.

Actual RM0090 Rev21 passages were read at printed pp973–977,981,991,
997–999,1010–1017. Figure308 on p998 was rendered and visually inspected.
Its arrows and capture strobes distinguish idle polarity, capture phase, the
suppressed last data clock, and start/stop intervals. The actual DS9484 Rev14
web PDF Table12 header and PD8/PD9/PD10 rows, printed pp78–79, were inspected:
USART3 CK is PD10 AF7. The physical selected package/board is still unknown.
The new pin helper therefore assumes exclusive ownership and a separately
verified board route; a register-image observation cannot certify wiring.

Initial native pilot exposed a real adapter defect: the underlying cancellation
return overwrote the original receive error. The correction preserves the first
error while still waiting for the real lower-layer TC condition. Another pilot
diagnosed two misleading-indentation warnings in the test dispatcher. They are
author-development failures, not learner observations. Exact final campaign
results are separately source-bound in validation.json; no source inspection or
successful execution is inferred from a title or this audit.

No full T38 closure: PR01 and PR03–12 retain their own tasks. Physical matched
CK/TX/RX, board routing, receiver setup/hold, independent review and delayed learner
attempts remain required. The exact published ISO C17/C23 access gaps remain.
The original criteria and historical acceptance evidence are preserved verbatim.
