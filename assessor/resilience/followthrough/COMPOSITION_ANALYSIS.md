# T75.09 transport composition technical analysis

Read COMPOSITION.md with OUTBOX, SERVICE_IO, HEDGE, RETRY and TIMERS. The private
controller deliberately uses their real implementations. The initial NetStack
state is an established retained-flight fixture, not a completed handshake.
Verify retained outgoing bytes with the actual frame parser; the returning ACK
has real Ethernet/IP/TCP fields and checksums. This bounds the experiment's
transport claims without replacing the required retransmission mechanism.

The response owns an allocated18-byte blob and disposer context until the
arbiter consumes it. Winning publication transfers that owner to the caller;
losers dispose before terminal permit release. The state remains on the calling
stack until every started attempt finalizes, so disposer context remains live.
Abort transport before completing an empty response. The cleanup path abandons
publication, aborts unclosed transports, finalizes any outstanding hedge ticket,
destroys the arbiter/hub and releases the scope. It never guesses that failed
device I/O had no effect. Verify allocation failures1..4 and journal cuts with
the unchanged-output sentinel independently of the device state.

Keep `hedge_considered`: an empty original response at tick1 may be followed by
the scheduled hedge at tick2. Quiescence before that decision would permit
premature destruction. A winner instead cancels the future hedge timer. Schedule
deadline as an actual wheel event; a no-response backend acknowledges its reset
on the next tick. Pending responses keep their owner until completion, not until
deadline. Same-tick scope equality still refuses the result even if response
processing precedes the deadline event. The absolute scope rule is authoritative.

Use ceil(due/1000), with checked bounded values. Mutant111 floors1501 to tick1;
the transport correctly refuses the premature dispatch, so the adapter loses
its wake and the exact expected retry/reply trace fails. Suppressing the
transport's due check would hide one symptom while violating another contract.
Mutant112 clears restored pending count after pre-ACK restart. Durable event
count remains nonzero, but delivery cannot progress; the controller reports
BUSY and the expected two deliveries/one ACK is violated. Do not repair this by
discarding the event or falsely declaring external delivery complete.

The independent Python model orders scalar events and counts durable logical
records; it does not call the C serializer, queue, timer or retry code. Its9096
histories are a bounded differential check. The C assertions add allocation and
partial-write failures absent from that model; adjacent native tests exercise
the actual synchronization and process boundaries. No one result proves all
interleavings, real network behavior or every possible persistent device fault.

Cost review uses real dispatch/write/flush/allocation counts before host timing.
Compare long-tail hedged versus unhedged with identical ownership/durability;
compare correlated-loss pairs with the same fault assumptions. Faster client
reply need not mean earlier quiescence or less total work. Slow notification
consumption affects credit release, while the client can already have a result.
This is an author experiment with a disclosed prior functional exploration,
not an independently sealed learner decision.

R1: demand the complete owner/event ledger including counterexamples to
committed=>replied and durable=>once-delivered. R2: inspect actual linked symbols
and parse/ACK adapters; a second simulator cannot replace the imported code.
R3: require each failed prefix's surviving device and owner state, plus exact
mutant rejection. R4: require all13 dimensions with actual raw measurements and
their limits. Levels0/1 cannot explain the failing mechanism; level2 is supported
implementation/analysis; level3 requires later independent changed conditions.
