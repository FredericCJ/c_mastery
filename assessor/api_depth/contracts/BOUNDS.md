# Withheld contract-boundary analysis and transfer

Assess the ten exact ordinary/changed pairs named in the public BOUNDS.md.
Do not assign .01b/.02c credit until the ownership/lifecycle package is also
present. Require each ordinary reasoning and changed-condition argument even
when one executable exercises several fields.

For the8-bit deadline, actual elapsed E in[0,127] implies
(now-start) mod256=E. Therefore E>=duration is the desired relation. Values
128..255 are rejected, and E+256 is an explicit indistinguishability example
outside the caller contract. Merely testing now>=start+duration fails across
wrap. Negative legacy timeouts are classified before conversion; the contract
accepts all negative int values, not just -1. A proposed signed-cast modular
test needs representability and implementation assumptions that this proof
avoids. Accept a different counter representation with an equally explicit
observation/retention bound and independently derived wrap cases.

The16-bit-unit exercise is a numerical representation simulation. The host
remains CHAR_BIT8. Capacity is in octets, so two units require four outputs.
The compile-time branch for UINT_LEAST16_MAX avoids an always-false comparison
on a16-bit least type while retaining admission on wider types. Do not claim
that a typedef, sizeof result or host endian choice establishes an actual
16-bit-byte C implementation. For a changed real target the transport must
define how octets are carried by its wider addressable units.

The unchanged api_append provider rejects all object-representation overlap,
including metadata. AcSlice's stronger unique-owner contract allows constant
admission work; copying still costs n. The reference deliberately does not call
api_append after proving disjoint owner identity, because that would retain the
quadratic scan. Actual api_append is separately executed against the independent
18,513-case field/element oracle. The overlap-allowing transfer actually calls
f_append, so its32-byte stage and generation conditions must be included in the
proof. A source in unused-but-initialized future suffix storage is valid; an
uninitialized source or fabricated extent is not made safe by capacity checks.

Found0 and found-null are valid payloads. The sentinel mutant is rejected by a
concrete key0 call. Product status/progress permits a terminal error and an
already committed prefix simultaneously. For malformed input encountered at
the same step as full output, the published parser's validation priority is
INVALID. A different priority is acceptable only in a separately versioned
contract and its corresponding tests. The atomic copy policy deliberately
differs from the prefix policy; conflating them destroys failure guarantees.

The assertion-side-effect mutant passes a build with assertions and fails with
NDEBUG. It still compiles in both; that is meaningful evidence that required
work was hidden in assert. Optional instrumentation is a different contract:
its disabled expression must not execute at all. Attack-controlled lengths are
admitted before creating trusted internal state. The service fault cases use
ordinary declared statuses and owned child exit codes, not a crash as an oracle
or a demonstration of privilege escalation. No assertion is a substitute for
input validation when the assertion may disappear.

At minimum7days give the original changed tasks, the interfaces and fresh
counter/ownership premises without this analysis or reference source. Ask the
learner to exhibit the deadline's indistinguishable full-wrap history, to derive
owner-offset validation under separate versus shared allocations, to migrate a
consumer relying on preserved failure output, and to show the value lost by a
chosen sentinel. A proof of an unsupported arbitrary-pointer validator or an
all-input portability claim fails even if supplied executables pass. Accept
alternative implementations only when they preserve the exact original contract
or explicitly fulfill its versioned changed condition.
