# G474 source and depth audit

The old T28.11 changed-device experiment assumed a preconfigured USART and implemented ISR/TDR mapping. It did not initialize HSI/kernel clock, GPIO alternate function, FIFO/baud/enable or a board route. That gap was real. This successor supplies the original bounded initializer, strict target integration, partial-progress/retry models, exact divisor oracle, and a physical capture protocol. It preserves the original HAL/generated/direct comparison and original gap identity.

The full RM0440 PDF remains unavailable after renewed canonical and range requests. Named pinned ST LL/device sources were read, including initialization and clock/readback fields. Actual DS12288, UM2505 and ES0430 page bodies were inspected in the browser; local downloads timed out, so no PDF hash is invented. The board route describes the physical PA2 net separately from its AF7 USART2 function. HSI characterization and elapsed stabilization are separated from poll counts.

Original target images use seven explicit original objects. Primary vendor headers are compile-only layout oracles. Native arrays are not peripheral emulation; no board was flashed or observed. Author-side mutation and image validation are distinct from external review and delayed learner performance.
