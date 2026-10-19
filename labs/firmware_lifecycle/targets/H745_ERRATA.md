# Selected H745 errata disposition

Read [ES0445 revision6](https://www.st.com/resource/en/errata_sheet/es0445-stm32h745xig-stm32h755xi-stm32h747xig-stm32h757xi-device-errata-stmicroelectronics.pdf), table2, sections2.1.1–2.1.5,2.3.1,2.20.1–2.20.5. Table2 associates U/V markings with REV_ID2003; actual device identity remains unobserved.

For `TRIAL_HEALTH.md`, disabled caches exclude the cache-resident prerequisites in2.1.1/2.1.2/2.1.4. The supplied freshness callbacks contain the ordering barrier required by2.1.5. This is source/assembly reasoning, not measured erratum reproduction. No watchdog is enabled; the protocol requests full system reset, so2.3.1's limited-watchdog-reset condition is outside this profile.

The UART is asynchronous, polled, fixed baud, ONEBIT=0: the mode conditions of2.20.1/3/4/5 are absent. Section2.20.2 still requires a sufficiently clean or filtered receive signal. Error flags cannot establish that electrical condition. Signed-frame rejection contains changed payload admission; it cannot restore availability or prove noise immunity.

Required changed-condition assessment: enable a cache or USART DMA and identify the newly applicable sections before implementing the adapter. Reject a report that labels the entire errata sheet inspected or the board qualified. Physical revision, signal quality, timing and unreviewed peripheral modes remain separate obligations.
