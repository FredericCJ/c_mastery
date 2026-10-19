# DMA depth audit and successor scope

The original PR-T28-06 criterion objects require admission, exact ownership return, partial/error progress and a cached shared-bank variant. The retained fw_dma test is a small abstract state machine; it neither validates buffer extents nor supplies FIFO/cache/HSEM device semantics. A heading or authored delayed task does not establish these implementations.

This successor implements all three ordinary DMA tasks and their existing delayed tasks: fixed bounded admission, copied software scatter/gather, selected DMA2 ports, conservative terminal intervals, suffix derivation, actual B06 cache/peer composition and HSEM1 metadata/payload acknowledgement. The direct EN binding is an explicitly incorrect adapter demonstration, not a rewrite of accepted baseline semantics. Public stubs remain unfinished learner assignments; private mutations remain intentional specimens. No baseline criterion/hours/receipt is rewritten.

Required separate work remains T28.11 service layers/UART-DMA composition/HAL comparison, T28.12 Ethernet/USB and T29 interrupt integration. Actual H745 boot/MPU/cache attributes are producer prerequisites from B06, not facts inferred from successful audit linking. Physical, external review and independent/delayed learner evidence remain separate. Finite host models and object audits support exactly their selected bounds.
