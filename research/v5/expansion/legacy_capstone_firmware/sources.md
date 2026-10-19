# MCU capstone source inspection and limits

The original four MCU briefs, their twelve criterion objects and twelve named
delayed variants, integration guide, capstone contract, boundaries and IRQ fixtures
were read before composition. This successor keeps those identities and exact
tasks. It adds implementation support rather than manufacturing learner acceptance.

The actual P3 encoder/parser and P4 ring, frame, clock, full selected F439 startup,
clock/USART/DMA source and public follow-through contract were inspected. The Pd
public contract and validation/submit/cancel/poll/finalize bodies, and complete
selected target configure/enable/stop/snapshot/visibility adapter were read. The
Xf public frame interface and basic/extended/lazy-frame inspection implementation
were read. This is a bounded import, not a fresh acceptance of every producer
project. The chosen release core is the same actual CsService as the Linux
capstone, not a rewritten wire implementation.

Primary Arm DUI0553B ID012616 text was read at PDF pages40–41,251–255 and257–261:
exception stacking/return, SysTick calibration, MPU priority, address/size,
permissions and barrier requirements. No visual inspection of diagrams or reading
of page256 is claimed. Text extraction was performed with the Windows pypdf
environment after WSL pypdf/pdftotext were unavailable; exact PDF bytes were copied
from the existing private cache. Extracts remain under ignored build storage.

RM0090 Rev21 was read at printed/PDF pages65,68,314–315,321,324–327: selected DMA2
and RCC address mapping, AHB SRAM versus CPU-only CCM, memory-to-memory/FIFO
restrictions, increment, disable/configure/drain and completion/error flags. These
passages support the imported selected Pd port. They do not establish board wiring,
electrical behavior or that every F439 register has the original teaching model's
W1C/read-clear layout. USART setup remains the explicitly imported P4 contract;
no claim of newly inspecting all USART/manual/errata passages is made here.

The errors.result header and result/map implementation were inspected and actually
compared on the host. The header's setjmp dependency and whole-object longjmp
reference are separately audited on the target. A supplied private newlib header
tree is a verification input only; the selected strict image uses plain status
and does not link that library. The default target-header failure is retained.

The final published C17/C23 texts remain unavailable. C17 compiler mode does not
close that source gap. Exact caller ownership, source quiescence, serial-window
freshness, trusted update verdict and maximum nesting are contract assumptions.
MPS2 execution supports selected CPU/startup/MPU behavior, not F439 DMA or UART.
Cross-building the F439 image is not flashing, physical validation, deployed-byte
verification, IWDG timing or electrical reset evidence. No new board interaction
was authorized or performed. External review and delayed learner defense remain
separate required events.
