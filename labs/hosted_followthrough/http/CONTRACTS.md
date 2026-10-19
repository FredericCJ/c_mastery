# Hosted HTTP followthrough

This additive package completes the original PR-T42-01 through08 tasks and their exact changed conditions. `criteria.json` preserves their identifiers, wording, expected results and minimum delays. Work against those original contracts before reading the optional assessor implementation. The exported `flow.c` remains incomplete. The earlier HTTP parser, date and lifecycle learner stubs remain incomplete too; no export silently substitutes an assessor solution.

Use C17, eight-bit octets, ASCII protocol syntax and exact `uint32_t`/`uint64_t`. All core calls are serialized with live, aligned, nonoverlapping caller objects. Borrowed spans last only for a call; response bytes belong to the queue until fully accepted or aborted. Core dependencies are audited type headers, bounded byte/string operations and the existing original parser. Linux sockets, clocks, flags and signals belong to the hosted adapter. Python, curl and QEMU are verification peers, never target HTTP implementations.

The separate freestanding object audit explicitly undefines the hosted `_FORTIFY_SOURCE` macro: this installed GCC otherwise adds `__memcpy_chk` despite `-ffreestanding`. The audit object exposes only the named parser and byte/string primitives. Ordinary hosted executable profiles retain their compiler defaults. This is a per-build declaration, not a host security-setting change; the observed earlier helper remains in its preserved attempt log.

## Build and predict

Run `python labs/hosted_followthrough/http/build.py --output /tmp/http-learner` in an exported workspace. It compiles the learner stubs and semantic tests; running the result must fail until the learner supplies implementations. Use `--kind linux` for the separately incomplete hosted initializer scaffold. To review accepted implementations, explicitly pass `--flow-source`, `--parser-source`, `--depth-source` and, for Linux, `--server-source`; no environment search discovers assessor files. The repository author campaign is `python3 tools/run_hosted_http.py --report reviews/v5/expansion/legacy_hosted_http/<new-name>.json` on Linux. It refuses to replace an earlier report.

Before executing a case, write four counts: input received, parser consumed, representation published, and connection discarded. Save the immutable raw bytes. `parser_cases.py` provides finite independent examples, including every split of a46-octet request, binary bodies, trailers, capacities and EOF. These are examples to extend, not a statement of complete HTTP conformance. Start a learner diary with a prediction, observed result, minimal disagreement and correction reason. Keep your diary distinct from the author campaign.

## 1. Descriptor identity and quiescence

A descriptor integer is not a durable identity. A slot carries its generation; a close first makes the old incarnation nonlive and returns its descriptor exactly once. Admission requires an unused, unretired slot and verified nonblocking/close-on-exec flags. Admission failure leaves descriptor ownership with the accepting caller. Generation exhaustion retires a slot rather than wrapping.

For queued asynchronous callbacks, increment a pending count before enqueue. Closing revokes dispatch immediately, but it cannot release storage while a callback still owns a pointer. A successful completion decrements its own pin, dispatching only if the same incarnation remains live. Reuse is permitted only when pending reaches zero. Two queued callbacks require two completions. This package's queue model is serialized; concurrent pin/release needs a separately justified synchronization protocol.

Exercise11,12,13,14; refuse15; pin12 twice; close12; refuse reuse while either pin remains; drain both without dispatch; then reuse integer12 under a new generation. Contrast this with merely comparing file descriptor numbers. `hf_accept_batch` uses at most4 attempts and publishes only after flags succeed. EINTR retries, EAGAIN stops, and flags failure closes privately acquired ownership. A readable listener can lose its pending connection before accept. See the [Linux accept notes](https://man7.org/linux/man-pages/man2/accept.2.html).

## 2. Portable byte state and authority

Read the exact parser state and consumed count after every supplied span. A CR at the end of one span is pending, not optional. A complete header can still require a binary body. A finished parser consumes zero further bytes until explicit reinitialization. Do not retain a pointer into the caller's receive buffer.

For origin-form, require one valid Host and match it to configured authority. For absolute-form, use the request-target authority while still checking the single Host's syntax. The project accepts a bounded lexical DNS/IPv4-style host with optional decimal port. It excludes IPv6 literals and userinfo; it does not perform DNS resolution or path-to-filesystem normalization. Test a changed configured name and address, not only localhost. Keep control-byte rejection in the header state separate from unrestricted length-delimited body octets. The [HTTP/1.1 grammar and authority sections](https://www.rfc-editor.org/rfc/rfc9112.html#section-3.2) establish the protocol boundary; the supplied limits and strict subset are original project policy.

## 3. Select framing before routing

The numeric wire domain is0..4294967295, independent of `size_t`. Before each decimal update, check `value <= (UINT32_MAX-digit)/10`. Finish syntax and range validation before the1024-byte storage cap. Thus1025x is malformed even though its numeric prefix is already oversized. Duplicate Content-Length and TE+CL are rejected by this project's stricter policy. A valid coding chain ending in chunked but requiring gzip decoding is framed yet unsupported501; a request ending in gzip cannot use that boundary and is400.

The retained parser used SIZE_MAX, so on a64-bit host4294967296 was representable. The explicit successor build replaces only that branch with the separately inspected `hd_length`; a valid over-cap numeral becomes sentinel1025 until header finalization so TE+CL still takes precedence. This never admits1025 bytes. Baseline source and earlier receipts stay untouched.

The AVR exercise compiles and executes that same `hd_length` on the named ATmega328P emulator with static assertions for16-bit `size_t` and32-bit wire integers. Only a number already admitted at<=1024 converts to a storage extent. Original reset code supplies the stack; linker output exposes compiler arithmetic helpers. The USART marker is an emulator observation, not physical UART timing or a full HTTP server on the2KiB SRAM device.

Framing belongs to the message, not its method or route. Consume GET's declaredabcde before starting the nextGET. A405 route still resolves its body before a persistent response. An early header-determined Expect error may instead return a final error and close. The [message-length precedence](https://www.rfc-editor.org/rfc/rfc9112.html#section-6.3) explains why peers must agree before interpreting continuation bytes.

## 4. Chunk grammar and independent limits

Track wire bytes independently from decoded content. The supplied3;x=y / abc /2 / de trace yieldsabcde, not the size lines, extensions or trailers. Zero chunk enters the trailer section; only its terminating empty line completes the message. Parse unknown bounded extensions, including escaped quote/backslash, then ignore them. Validate trailer fields separately and discard them without overwriting Host, framing or route decisions.

Isolate each cap: decoded body1024, extension128 per chunk, trailer1024 wire bytes including finalCRLF, trailerfields8, header8192 wire bytes, headerfields32 and target1024. A line buffer needs a separate NUL slot. The retained parser checks extension capacity after its complete size line; record that first rejection location honestly. Three-receive delimiter cuts complement the two-way split sweep. Mutate only a framing byte while keeping independently counted data unchanged; a passing finite corpus or coverage percentage does not prove all malformed states reject.

## 5. Representation metadata and time

Health is exactly6f6b0a, three bytes. HEAD sends no representation body while preserving the selected length, including404 length10 and405 length19 with Allow:POST. The retained serializer used reason text withoutLF, producing9/18. The successor explicitly selects lowercase reason+LF; this is an additive implementation correction to the original contract. A UTF-8 representation c3a90a has three bytes even though it encodes two characters. Supply a byte extent instead of a character count.

The serializer validates Date when present and accepts NULL to omit it. It stages output in1600 bytes before publishing, leaving destination/length unchanged on failure. Include that transient stack buffer in the memory budget. Fixture epoch1672531200 produces the fixed2023 date. The hosted adapter's actual-clock mode uses wall time only for Date; monotonic time drives deadlines. A wall rollback cannot extend a request. An unavailable clock omits Date in both hosted and existing customTCP compositions. This follows the [Date sender rule](https://www.rfc-editor.org/rfc/rfc9110.html#section-6.6.1); no clock accuracy or leap-second handling beyond the inspected `hd_date` subset is implied.

## 6. Ordered interim and final output

Each output entry owns immutable bytes, length, offset, start time, request number and interim/final classification. The results4,EINTR,2,EAGAIN,3 on nine bytes leave offsets4,4,6,6,9. Zero, oversized callback success and EPIPE abort. Only a valid positive prefix advances. Local send acceptance says nothing about peer application consumption.

FIFO applies to interim responses too: request1final, request2's100, request2final. Keep that association even after only four bytes of response1 leave. With a single active output slot, stop parsing while full, drain the prior response, resume to enqueue100, drain100, then accept the waiting client's body. The raw peer actually waits for100. If computation2 finishes before1 in a future asynchronous design, retain sequence-indexed ready results and emit1 first. That changed-cost analysis is separate from the synchronous server implementation.

## 7. Stop, drain and deadlines

Request2close is absorbing for following request3. Otherwise request4 closes and5 is never dispatched. After the final queued response drains, shutdown the write direction and receive/discard until EOF or1000ms. Discarded bytes are not parsed. The output timeout is a different state:15 seconds with pending bytes aborts and makes no completed-response claim.

The owned unread TCP pair has a bounded setup prefill to force EAGAIN; no third-party peer is used. It then retains a real HTTP response at offset0 until the actual15-second output deadline. The raw socket campaign separately observes descriptor reclamation after the one-second receive-drain deadline. Do not confuse the prefill bytes with an HTTP response or a closed receive window with a completed close.

A request starting0 expires at15000 even after progress4000,8000,12000,14900. Check `now >= start` and `now >= progress` before subtraction; compare elapsed with>=. NearUINT64_MAX this avoids overflowing start+duration. A regressed/wrapped clock is invalid under the declared monotonic contract. A408 is available only while bounded response output remains usable.

## 8. EOF is an I/O result

HUP/readable is a reason to attempt a bounded receive, not evidence that buffered bytes have disappeared. Consume bytes before observing real recv0. EINTR/EAGAIN preserve parser state and are not EOF. The native trace inserts both between the final body byte and actual EOF; complete and incomplete bodies remain distinct. The [recv return contract](https://man7.org/linux/man-pages/man2/recv.2.html) defines orderly stream EOF; the original adapter's buffering and error decisions remain separately tested.

## 9. Memory and transport ownership

The recorded64-bit native layout is HttpRequest2224, HttpParser10640, HFOutput1632, HFFlow13960 and HFPool80 bytes; remeasure another ABI. The parser line is8193 bytes, body1024,target1025, and two output entries3264 including metadata/padding. Four flows occupy55840 bytes. The server additionally stores per-connection descriptor/handle,32-byte retained input and its offsets, the pool, poll snapshot, authority/date locals and transient serializer scratch. Compiler stack-usage files record non-inlined frame estimates; kernel TCP buffers are outside this application ledger.

The successor receives into32 bytes instead of the retained2048. Every large valid header/body is incrementally consumed without reducing wire limits. Capacity1 changes the active queue policy, not the allocation of its fixed two-element array; do not claim that alone saves memory. The actual server object ledger includes its record padding and pool. Exercise both capacities and exact wire caps before accepting a memory redesign.

The existing customTCP adapter copies response data and its flight packet. Overwriting the input packet after receive cannot affect either. Unacknowledged retransmission reproduces the same retained frame and does not advance sequence space again. This packet execution demonstrates the byte/lifetime adapter using the already available com_net implementation; cable and physical closure remain separate T41 evidence.

## 10. Local interpreter disagreement and independent peers

The two tiny original framing models deliberately choose different boundaries for one local byte string. Repair by rejecting the ambiguous combination before dispatch. Likewise, repair a deliberately flawed continuation after rejectedCONNECT/Upgrade by making close absorbing. Preserve exact input bytes and replay every selected split. These bounded teaching models are not deployed proxies or evidence of a remote exploit.

The raw client independently parses response boundaries and tests pipelines, half-close, HEAD,100 and binary data. curl separately checks ordinary method/Expect interoperability. Keep command/version, raw hex, actual clock checks and descriptor flags. Neither peer proves general RFC conformance, TLS, arbitrary authorities or a production service.

## 11. Evidence and delayed reconstruction

Author GCC/Clang debug/release execution, actual socket observations, AVR emulation, finite models, external review and unaided delayed assessment are different evidence dimensions. After at least seven days, select a new partition and a malformed boundary without consulting the implementation; predict, run, minimize, then explain which premise changes. `DELAYED.md` is deliberately unfilled. No author run today supplies that observation.

Each report pins executable sources, generated composition, binaries, input corpus, output logs and exact criterion selectors. A source mismatch invalidates its acceptance until rerun. Earlier attempts remain preserved, including the first descriptor-observation race. Export checks must prove both the incomplete learner path and explicitly selected assessor path, without exporting assessor files.
