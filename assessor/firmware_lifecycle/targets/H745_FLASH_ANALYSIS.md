# Flash-word assessor analysis

The critical new counterexample is ECC-word reuse: the old4/8-byte bit-clear
model permits updates that this selected H745 controller does not. PSIZE changes
parallelism inside an operation, not the256-bit data/ECC programming unit.
Require three body words and a distinct commit word; copying a24-byte journal
format into one flash word and updating its marker later is not a valid transfer.

The bus model checks every original register interaction. Unlock-only-if-locked
matters because the documented repeated key sequence locks until reset. Busy
status dominates EOP; a timeout never authorizes PG clearing, another client or
buffer/word reuse. A persistent uncompleted operation retains its session.
Readback/error failure can mean partial programming: repair by quarantine and
recovery from the intact older record, not optimistic retry of the same word.

The independent Python byte oracle fixes endianness, offsets and CRC separately
from the C writer/reader. Bit prefixes happen to accept only the complete record
because the final required zero bits lie at its end. This does not prove arbitrary
analog torn-flash behavior, ECC-fault recovery or resistance to malicious records.
A counterfeiter can recompute CRC; trusted signature/digest/floor checks remain
necessary. Neither a word-aligned address nor old all-one bytes establish that
the word has not already undergone a physical programming/ECC operation.

Withheld delayed task: a reset leaves the second body word readable as all ones,
the first body word valid, and the previous committed record intact. The proposed
patch retries only the second word and updates a commit marker in the first word.
Reject both actions without a proven virgin-word allocation state; quarantine the
interrupted record, use fresh space and retain the old committed image pair.
Require a different argument for F439 and exact selected-part errata before any
physical conclusion. Link/assembly output is author code evidence, never board
execution or independent learner mastery.
