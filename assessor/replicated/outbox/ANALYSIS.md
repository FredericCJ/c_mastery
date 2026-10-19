# Inbox/outbox technical assessment

The source log itself is the durable obligation, which removes a second
publication between event append and queue insertion. Given source prefix n,
ACK prefix a and receiver prefix r, safety requires0<=a<=n,0<=r<=16 and exact
immutable identity at each retained position. The source and receiver may be
at different prefixes because reply loss does not lose the authoritative inbox.
There is no invariant a=r during failures. Eventually catching up additionally
requires a live peer, retry scheduling, storage capacity and successful I/O.

A successful next receive first commits one actual RplLog event, then rebuilds
the sum through rp_project and publishes its reply. Under admitted nonforged
controls, projection cannot fail after a valid append: positions remain1..16,
each prior projection step succeeds, and sixteen signed32 values fit signed64.
Reopen reconstructs the same source records, not a second nontransactional total.
An identical old event returns the total through that event's position, which is
essential when an old reply is delayed while newer inbox data exists. Comparing
only key/value but forgetting kind would conflate recorded-query semantics.

The ACK append has its own uncertain boundary. Reopening it may expose the old
or new prefix; in either case a duplicate receive is safe. Updating a volatile
ACK counter before its durable append would incorrectly hide pending work after
failure. The actual adapter instead uses the recovered ACK log's count, and
checks identity/position/sum before appending. The source event must remain
immutable while borrowed; the API cannot defend against an illicit caller that
rewrites a live store or aliases two stores to the same backing medium.

The callback fault campaign covers37 page-write/flush callbacks after a clean
checkpoint:34 writes and3 flushes. For each receiver/ACK boundary and each
discard/all-pending-persist mode, recover and retry. Only persistence at the
complete commit write or final flush can expose the new event. Other partial
prefixes are covered by the separate durable-log campaign; these148 protocol
cases are not an enumeration of arbitrary sector ordering.

Actual POSIX process tests retain three independently backed unlinked files.
A source child commits event2 and exits before delivery; a receiver child exits
after event1 commit before replying; a sender child obtains an intact reply but
exits before ACK. The surviving parent reconstructs state and retries. The final
inbox has two events,total5,ACK count2. The separate flawed effect file increments
before its missing inbox commit; terminating at that point and retrying produces
counter2. This is a harmless, inspectable violation of an at-most-once external
effect claim, not a claim of arbitrary code execution. Moving the counter into
the authoritative event transition (or using a receiver-owned atomic idempotency
transaction) changes the contract; merely naming a model CQRS does not.

The stream layer imports actual rf_feed/rf_eof/rf_message_init/rf_outbox calls.
Its byte staging is bounded64, expected payload lengths are exactly8, and the
logical record ends only at EOF. Arbitrary short reads leave the parser between
states; no output is published until the full validated record and clean EOF.
Every incomplete prefix rejects. The synchronous writer cancels outstanding
stack descriptors on error; a backend retaining their pointers is outside the
contract. Unknown/malicious peers remain outside the trusted-ACK premise:
anyone who knows source events can forge the expected reply without doing work.

Controls11/13 break duplicate/gap admission;12/14 break reply/log identity;
15/16 admit truncation/noncanonical frame boundaries;17/18 publish output early
or return success on a failed durable receive;19/20 duplicate after recovery or
omit durable ACK publication in the actual process experiment. All must exit through a
semantic RP_CHECK assertion, not an unrelated crash or sanitizer finding.
The independent Python oracle uses lists and integer sums, not the C reference
or its branch text; it checks every state row including rejection output.

For the delayed advancing-prefix variant, first deliver event1, then allow
the source/inbox to advance to event2 before replaying event1. Its reply still
certifies prefix1. Once ACK1 exists, a delayed reply1 cannot append ACK2 even if
the two event values happen to yield equal totals. Position and UUID are part
of the admission predicate, not merely redundant checks on the sum. Require the
learner to derive the partial order and unchanged ACK2 refusal independently;
passing the supplied receive/ACK examples is not an unaided assessment.
