# Classical CAN frame adapter — T24.10.c

Stable criteria: AX-PR-T24-06-T24.10.c and its original seven-day DELAYED variant.
Prerequisites: T18 borrowed spans/lifetime, T23 local descriptors, T24 framing
and the retained PR-T24-06 PTY/parser work. This package adds CAN interpretation;
it does not replace that earlier terminal adapter. Baseline C17, hosted Linux
headers/libc, GCC/Clang. No CAN application library supplies the mechanism.
The target is original in-memory fixtures; there is no external bus connection.
Host Python checking is distinct from the C implementation.

The installed Linux `struct can_frame` is an ABI representation, not portable
wire serialization. The inspected kernel documentation and header identities
are recorded in `research/v5/expansion/legacy_can/resources.json`. The normal
payload is at most8 bytes, and CAN ID format flags must be interpreted
separately. A remote request has a requested length but supplies no application
payload. Linux error messages represent diagnostic classes; treat them as
diagnostics, not normal application messages. CAN FD/XL are distinct interfaces;
this adapter rejects their sizes. These are Linux/CAN contracts, not ISO C17
features. C23 does not automatically validate an identifier, frame or lifetime.

`CaFrame` owns its fixed eight-byte value; it never borrows the input span.
`ca_decode(bytes,length,msg_flags,out)` returns0 only for a complete classical
frame and valid teaching-policy fields. Read a potentially unaligned ABI span
by copying into an aligned local object, not by casting its address. Reject
MSG_TRUNC/MSG_CTRUNC, oversized payloads, unflagged identifiers above11 bits,
and an error frame combined with EFF/RTR. Standard and extended data/request
IDs are bounded to11/29 bits. Accept `len8_dlc`0, or9..15 only when length8;
error frames require length8 and raw DLC0. These strict normalization choices
are the exercise's declared policy, not a claim that every driver rejects the
same malformed input. Decode local/own metadata from MSG_DONTROUTE/MSG_CONFIRM;
do not infer acknowledgment by an application or physical receiver from it.
Unused output payload bytes are zero, including all bytes for remote requests.

`ca_encode` accepts valid data/request values, initializes the entire ABI object
and copies only the meaningful payload. It rejects error-message transmission:
the teaching target receives synthetic diagnostics, it never injects errors
on a real bus. All invalid operations return-1 and preserve the output object.
Unknown `CaKind` values fail. Pointers designate valid nonoverlapping live input
and output objects; ISO C cannot recover from an invalid caller pointer.

`ca_filter(id,extended,remote,out)` constructs an exact positive filter whose
mask distinguishes format and request type. `ca_accept` compares one such
filter (or an explicitly supplied positive mask) and routes error messages
through a separate error-class mask. Filter inversion, filter-list OR/AND and
CAN FD/XL are not silently emulated by this API. `ca_loopback` is a two-endpoint
local delivery **model**: sender/receiver indices0..1, loopback flag0..1 and
receive-own flag0..1. A matching peer receives when loopback is enabled; the
sender additionally needs receive-own. Delivered output has local1 and own set
only for the sender. Return1 for delivery,0 for filtering/disabled delivery,
and-1 for invalid inputs; rejected/not-delivered outputs stay unchanged.
This models software options, not kernel scheduling or bus arbitration.

All operations are bounded O(1), fixed storage and no allocation, queues or
implicit background workers. No atomic/thread safety is implied; serialize
shared output objects. Padding and endianness remain ABI properties. If the
adapter is moved to a protocol stream, derive an explicit wire format rather
than transmitting these structs. Actual object imports/size are part of the
author campaign. Reference C files stay assessor-only; public frames.c is
intentionally unfinished. Finite success establishes the tested interpretations,
not physical reliability or absolute mastery.
