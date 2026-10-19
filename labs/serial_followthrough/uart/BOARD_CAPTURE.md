# Required dedicated UART observation packet

No physical observations are supplied. Before any separately authorized board
experiment identify the exact F439 package/board/revision, oscillator and APB
source, USART3 PD8/PD9 route, voltage domain, connector/peer and instrument/probe
loading. A RAM address model and MPS2 CPU execution do not establish these facts.
Use only a dedicated original teaching peer and synthetic bytes; do not connect
an RS-232 line directly to a logic-level MCU pin. Declare any MAX3232 part,
capacitors, supply/load and its allowed signaling envelope independently.

Capture 0xA5 and 0x01 at the logic side with start/data/stop boundaries and the
specified parity configurations. Measure raw bit periods using calibrated time
references, uncertainty, repeated captures and actual oscillator conditions.
Compare exact divider quantization with observed error; retain a separate
receiver tolerance argument and report unexplained residuals. Observe output
at the transceiver side only with suitable rated instruments. The 3 Mbaud divider
case is not a requirement to drive a MAX3232 at an unsupported rate.

Record a bounded one..16-byte transmission and preserve clock-generation,
buffer placement and linked symbol evidence. Observe the last line interval
separately from TXE/DMA completion. Inject a controlled original-peer parity,
framing or overrun condition; retain old RDR/raw status and the application's
discard/consume decision. Document how injection and observation affect timing.
Avoid pretending that servicing all events in a host loop measures ISR latency.

For the +3% changed condition, retain analyzer settings, actual measurements and
the failed local ±1% gate even if decoding succeeds. A changed clock, M7 part,
cache mode, pin route or transceiver requires a new dossier. Authoring this
packet does not authorize flashing or connecting deployed equipment. Missing
apparatus keeps this dimension open and does not erase the required task.
