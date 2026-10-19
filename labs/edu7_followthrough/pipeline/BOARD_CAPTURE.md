# Required EDU7 bench packet: currently not observed

Use only three dedicated declared F439 teaching boards and four verified3.3V
THVD1400 adapters on two separate≤1m segments. Follow the original T40 topology
and physical producer BOARD_CAPTURE.md. Authoring this packet authorizes neither
flashing nor interaction with a deployed bus. Retain board revision, actual
adapter identity, pin allocation and exact firmware/source/tool hashes.

Build each node's earlier reset/clock/time artifacts with the public pipeline
API. ew_f439_open accepts a stable PuClock and absolute microseconds; R obtains
both selected ports, A/B port0. Call ew_tick from the single serialized foreground
owner. Supply EsPeer OPEN/DATA/TOKEN/RESUME and consume copied EsEvents according
to the learner contract. The assessor three-node CPU harness demonstrates that
protocol sequence; its register callbacks are not a physical firmware port.
The separately linked F439 adapter proves only its selected source/helper closure.

Capture power-off termination resistance at both ends of each segment; measured
supply/reference/polarity; actual UART8N1 bit time and uncertainty;200us pre-enable,
last stop/TC/DE release and200us post-DE guard; no overlapping drivers on each
segment; exact SET42 and reverse response on both sides of R; and a controlled
single-segment disconnect/reconnect with no partial application publication.
Time-align raw captures with node/link/port/transfer/session/checkpoint/request
identities. Preserve raw samples, instrument configuration and uncertainty. Never
label the model's87us/10us scheduling numbers measured physical observations.

Compare ordinary THVD1400 and changed THVD1410 only under each part's actual
selected supply/load/pin/timing contract. PA1/PA2 conflict with Ethernet usage;
reserve them before any separately authorized bench. Leave every missing item
not-run. Checklists and a nonzero evidence mask do not authenticate captures.
