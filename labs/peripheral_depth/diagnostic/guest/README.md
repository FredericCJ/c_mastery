# Actual USB provider integration

First complete the portable request/packet model and its invariants. Implement
this gadget adapter against the explicitly selected Linux kernel headers, with
the actual earlier diagnostic core and `f_take` implementation. Kernel compilation
uses its GNU dialect and API, separate from the C17 portable-core audit. The
three local type-header shims translate only integer/size definitions; they do
not introduce a standard C library or implement a diagnostic mechanism.

The adapter must allocate/free EP0, OUT and IN requests, publish descriptors,
handle setup/configuration/reset, preserve ownership through submit/completion/
dequeue/disable, and expose only the six synthetic counters specified by the
shared contract and assessor probe. Endpoint autoconfiguration may change packet
size: the enabled endpoint and advertised descriptor must agree. Support a
callback that arrives before the selected provider's queue returns, while
explaining how that observed provider path compares with its generic API docs.

Use `build.py --kernel-headers PATH --dummy-source PATH --directory BUILD`
to build the learner scaffold; `--source`, `--payload` and `--foundation` select
explicit externally supplied implementations. There is no private-source fallback.
Build output is not a successful exercise: the untouched scaffold refuses binding
with `-ENOSYS` and a `DG_GADGET_STUB` marker.

Run modules only in a dedicated diskless networkless QEMU guest with matching
kernel image, headers and module dependencies. No USB passthrough or host insmod.
The assessor producer uses the selected dummy host/device controller, records
kernel configuration and versions, and checks positive traffic, cancellation,
descriptor-size and replay mutants. Preserve raw errors; do not change host
module signing/security policy. Physical USB, arbitrary UDC support, learner
performance and external technical review remain separate evidence dimensions.

The laboratory protocol uses synthetic VID `ffff`, PID `0001`, one configuration,
one vendor interface (number/alternate zero), and bulk IN/OUT endpoints with an
eight-byte maximum packet. Discover endpoint addresses from the configuration
descriptor; autoconfiguration need not select OUT endpoint one. Never claim this
synthetic VID is assigned or use the device on an operational USB bus. Every
bulk OUT transfer carries exactly one big-endian diagnostic message. A multiple
of eight bytes requires a terminating zero-length packet. Successful messages
receive an identical IN echo; malformed/replayed transfers receive no echo.

Three vendor/device requests have `wValue = wIndex = 0`:

| bmRequestType / bRequest | wLength | Contract |
| --- | --- | --- |
| `c0 / 51` | 24 | Return six unsigned little-endian 32-bit counters, in order: accepted application messages; rejected application messages; IN completion callbacks observed before queue returned; successful IN completions; cancelled IN completions; cancelled OUT completions. |
| `40 / 52` | 0 | Request cancellation of the outstanding OUT request; ownership remains with the provider until completion. |
| `40 / 53` | 0 | Requeue OUT only after cancellation completion has returned ownership. Return an error if still owned. |

Counters concern the original gadget, not host URB ownership. The host must
separately submit, discard, and reap an IN URB, verify the returned identity and
negative completion status, and retain its buffer until reap. A completed gadget
request may mean a controller FIFO copied the bytes; it does not prove host
consumption. Document the finite test's counter bounds rather than inventing a
portable number of inline callbacks. The selected controller is expected to
exercise at least one inline completion; other controllers require another
explicit callback-interleaving test.
