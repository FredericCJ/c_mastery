# Assessor-only I2C analysis and transfer packets

<a id="08.a"></a>
## 08.a

Address52hex givesa4/a5. Ordinary state sequence is SB/address-write, ADDR/readSR2,TXE/register10,BTF/repeatedSTART,SB/address-read,ADDR/ACK0/readSR2/STOP,RXNE/read5a,STOP-cleared/publication. The zero-address own-match configuration and disabled general call are explicit premises for idle/STOP SR2 inspection. ReadSR1/readSR2 is not a harmless combined status poll. For two bytes, ACK0/POS1 precede ADDR clear, BTF precedes STOP and both DR reads. A driver that substitutes RXNE for BTF can read a nonexistent second byte and send NACK/STOP at the wrong boundary. Mutants1/2 test these distinctions, including output preservation and exact wire addresses. Bit-width/endianness changes do not change a seven-bit address's shift/direction bit.

Delayed packet: address52hex/register10, response5a/c3; ask for the complete two-byte procedure without giving its event ordering. Require the source locator and the sequence proof, then run RXNE-only before BTF. Merely extending a one-byte loop fails. State why the N>2 procedure cannot be inferred from this two-byte loop. Native models are observations of encoded register effects, not proof of silicon behavior.

<a id="08.b"></a>
## 08.b

ARLO wins over simultaneous NACK: it means another master owns the bus, so release and do not emit STOP. An address NACK and data NACK have different progress evidence. PI_TIMEOUT records the budget expiration but may leave cleanup active until CR1 commands complete. The late START case is essential: master ownership may arise after the ordinary timeout branch, and retirement before STOP would leave an active controller. Mutant3 drives STOP after ARLO;4 admits recovery with peers;6 retires a pending START prematurely. Each must compile before semantic rejection is counted.

Exclusive recovery has a half-period deadline plus a total bound, never more than nine requested recovery clocks, and a STOP only with SCL observed high. It releases output drivers on failure; this does not make external held lines high. The actual fw_i2c_recover and sd_i2c_clear imports agree for their declared exclusive simple cases. They have different temporal contracts: sd_i2c_clear consumes an already abstract release-after count, while this successor waits for physical-level observations. A model match does not prove equal real-time behavior.

Delayed packet: introduce a second master holding the bus. Require no GPIO writes from rejected recovery, passive admission while BUSY, ARLO without STOP and a separately justified recovery lease. Test both no-lease and peers-present cases, plus an SCL-low timeout. Describe the exact authority that establishes exclusivity on a real teaching bench; no program flag can prove an unobserved physical topology.

<a id="08.c"></a>
## 08.c

For value42, read with a lost acknowledgement leaves42 and is retryable under the immutable-state premise. Increment with lost acknowledgement leaves43; replay leaves44. The unchanged bus implementation cannot distinguish an unexecuted request from an executed request with a lost acknowledgement solely from timeout. The result is uncertain, not a fabricated rollback. Mutant5 declares retry safe after a possibly applied increment. Device overflow at255 is a defined rejected command, and invalid/caller/stale statuses are not retry-eligible bus failures.

Delayed packet: replace only register-read with increment and arrange application before ACK loss. Require two separate histories indistinguishable at the requester, the resulting retry policy and a concrete duplicate-effect counterexample. Alternatives are valid if their added premise is explicit: a persistent operation ID plus deduplication, an externally reconciled counter, or an idempotent desired-state command. An in-memory local sequence number is not evidence of crash-persistent exactly-once execution.

## Rubric and verification scope

Assess each objective separately.0: invalid sequence/ownership, lost uncertainty or fabricated board claim.1: ordinary fixture matches but a criterion-specific counterexample survives.2: implements and explains both positive and negative cases, with correct source/ABI/ISA boundaries, failure states and costs.3: independent completion of that objective's changed packet after at least seven days, revised invariants and diagnosis under an unfamiliar trace. Author runs do not award learner scores or review independence.

oracle.py uses independent wire arithmetic, prefix boundaries and device-effect equations for2921 rows/profile:45 clock requests,1344 completed exchanges,32 wait cutoffs,440 recovery cases,1024 effect cases and36 policy cases. Public/target mutants discriminate distinct faults. Repeated finite success, sanitizer cleanliness and object closure are bounded evidence. Native target tests exercise actual addresses/fields and PRIMASK restoration in a model; M4 disassembly must include actual save/disable/restore instructions. The linked image is not booted firmware. Physical bus captures, load/pullup/uncertainty measurements and T29 startup integration remain separate.
