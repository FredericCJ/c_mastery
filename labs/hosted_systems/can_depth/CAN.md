# One identifier is not one frame format

Implement `frames.c` under CONTRACTS.md. Use the original public tests as the
starting boundary cases, and submit independent tests and reasoning. Read the
kernel's [SocketCAN frame, filter and loopback sections](https://docs.kernel.org/networking/can.html).
Exact inspected scopes are in the source register; termios documentation alone
does not establish SocketCAN behavior. This corrects that omission additively
without replacing the original assessment's historical reference list.

Construct a standard data frame ID0x123 containing the bytes0x12,0x34 and an
extended frame whose low11 bits are0x123. Explain why a filter testing only those
low bits admits both, then derive a filter that distinguishes format and remote
request. Include zero length, length8, invalid9, the maximum standard/extended
ID, error-class masks, and an unaligned input copy. Preserve rejected outputs.
The provided sweep covers all256 length values and all256 raw-DLC values; an
independent tuple comparison checks the format/request/identifier relation.
Do not derive the expected filter result using the same mask expression.

The requested local experiment uses an already provisioned **isolated** vcan
interface or this frame adapter when none is available. Read-only discovery
must record the interface type and namespace; a name beginning with `vcan` is
not sufficient. Never bind index0/all interfaces. No command in the author
campaign creates, brings up, reconfigures or transmits on an interface. The
adapter is an explicit permitted backend, not evidence of SocketCAN execution.
Use only original synthetic frames. An operational vehicle/deployed bus is
outside this exercise.

Derive the two-endpoint delivery table before running it: loopback disabled,
loopback enabled with own reception disabled, and own reception enabled. Distinguish
the separate peer from the transmitting socket. Add a mismatched filter and
an error frame with error-mask0 versus BUSOFF subscribed. Error-class admission
does not demonstrate that a controller actually entered bus-off. A successful
software roundtrip cannot establish arbitration, acknowledgment, termination,
transceiver voltages, bit timing, error confinement or physical delivery.

Compile from the workspace root:

```sh
cc -std=c17 -O2 -Wall -Wextra -Werror -Ilabs/hosted_systems/can_depth \
  labs/hosted_systems/can_depth/tests.c labs/hosted_systems/can_depth/frames.c -o can-test
./can-test
python3 labs/hosted_systems/can_depth/check_bench.py --self-test
```

Deliver the adapter, independent filter oracle, invalid-span cases, delivery
table, scope manifest, native ABI/header/compiler identities, and import/size
output. Compare one deliberately flawed format-insensitive filter and one
error-admission defect with the repaired implementation. They must compile
and fail a discriminating semantic check. UBSan/ASan results are bounded
observations; no crash is called an arbitrary-code-execution demonstration.

For the original changed condition, prepare the complete two-node packet in
BENCH.md and bench_plan.json. Supply calculations, capture formats, inspection
checklist and acceptance thresholds tied to selected authoritative device
documents before execution. Actual board selection, measurements and delayed
learner work remain separately pending. A software adapter is not an equivalent
physical experiment.

Graduated hints: (1) distinguish the ID value from its flag bits; (2) evaluate
standard0x123 and extended0x1abc0123 against the same low-bit mask; (3) draw
three separate paths for external frames, local peer echo and own-socket echo.
For the bench, begin with the ownership/isolation manifest before choosing a
bit rate; derive register encodings only from the actual controller manual.

Assessment: level0 violates containment, loses representation information or
claims physical success without observations. Level1 passes ordinary frames
but cannot explain false format matches or local echo. Level2 derives contracts,
falsifies each misconception and supplies independent boundary evidence.
Level3 independently adapts the contract to the specified two-node environment
after at least seven days, states the additional assumptions and evaluates the
actual evidence without substituting a virtual trace. Assess the other two
PR-T24-06 criteria using their existing terminal package, not this CAN test.
