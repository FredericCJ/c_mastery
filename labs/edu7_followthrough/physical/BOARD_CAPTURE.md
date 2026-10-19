# Required three-board, two-segment receipt

Before separately authorized physical work, identify endpoint17, bridge34 and
endpoint51 boards, packages/silicon revisions, transceiver markings, actual
schematics, supplies, references, cable/stub lengths and two120-ohm end resistors
on each electrically distinct segment. With power removed, measure the expected
parallel resistance near60ohms on each pair and explain component tolerances,
bias paths and other loads. Do not connect a meter to a powered circuit as that
resistance test. No measurement is supplied by this author package.

Use USART3 PD8/PD9/PD10 on port0 and USART2 PA2/PA3/PA1 on the bridge port1.
Confirm package routing and remove board-specific conflicting Ethernet/debug
connections through the actual board procedure before use. Record /RE tied low,
DE active-high and exact D/R/A/B pin identities; swap of a module's A/B labels
requires its actual documentation and a new ledger, not guesswork.

Capture both DE signals on each segment, UART TX/RX and differential A/B with
instrument calibration, thresholds, rate and channel skew. Execute the original
POLL, reply first1500us/final9500us witness, maximum escaped frame, no-start and
disconnect/reconnect cases. Retain raw timestamps, clock/divider, source hash,
reset state and first divergence. Bound first-data enable margin, final-stop
disable margin, each guard and same-segment driver exclusion including uncertainty.
Different-segment concurrency is not same-segment contention.

For the changed adapter, redo pin/package/polarity/supply/load/timing checks.
For lower baud, demonstrate software refusing an impossible maximum-frame budget
before any deadline change. Inject UART errors through bounded original fixture
events; do not deliberately drive unsafe electrical contention. A faulted echo
must increment error and echo evidence independently while publishing no peer data.
Record residual electrical, scheduling and observation limits. Simulator register
values, empty template fields and host-only traces cannot satisfy this receipt.
