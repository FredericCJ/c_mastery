# Assessor analysis: flags and versioned negotiation

The abstraction function of an admitted mask is the subset of{0,1,2} whose bits
are1. There are2^3=8 values. Union/intersection close over these values; runtime
revalidation prevents forged public controls being accepted merely because an
intersection deletes the offending bit. Per-bit Boolean OR/AND proves closure,
commutativity, associativity, idempotence and the two absorption laws. Empty set
is the union identity and intersection annihilator. There is no owned resource.

The two version masks describe allowed capability declarations, not an implicit
grant of every feature in a version. Let C=L intersection P, R=RL union RP,
A=AL union AP. Admission requires R subset C; enabled E=(R union A) intersection
C. Thus E subset L and E subset P, and R subset E. A is retained even when it
contains positions outside either version. Rejecting unsupported R before output
publication preserves all previous fields. Invalid capabilities take precedence
over unsupported requests. An all-zero valid negotiation is meaningful.

The erroneous union C=L union P admits8 for L=7,P=63. Both offers being well
formed does not repair this. Erasing A outside C destroys information needed
by a downstream or future participant. This original policy permits required
and advisory overlap; it does not interpret overlap as contradictory input.

N1570's complement is applied after promotions. On measured32-bit-int hosts,
uint8_t promotes to int and ~(uint8_t)7 has signed negative value; uint32_t
typically remains unsigned. On wider-int implementations uint32_t may promote
to a signed type that represents all32-bit values. The proof must concern the
tested low32 bits after the usual conversions, not the assertion that a typedef
always suppresses promotions. No signed shifts or negative shift counts occur.

Expected ordinary oracle statuses are independently computed with sets. The
constructor all32-bit campaign is stronger than a256-input sample on that
specific executable, but it does not prove the machine compiler, explain why
the source is portable, validate negotiation over all possible inputs, or assess
the learner. Require the proof alongside concrete observations. Separate-TU
no-LTO compilation keeps a visible call to the retained provider per iteration.

Sealed delayed case: sender version2/capability63 forwards advisory bit5 through
a version1/capability7 participant. First agreement retains advisory32 and
enables0. Later negotiate both version2/capability63 using that retained offer:
advisory32 is then explicitly enabled. Next change bit5 from advisory to required
while one participant is still version1: refusal is mandatory. A solution that
forwards only enabled bits loses the first request. A solution that interprets
preservation as support fails the second. Have the learner derive the transition
before implementation and retain the unaided result separately.

The harmless security counter witnesses only unsupported local feature
admission. No cryptographic identity or real privilege boundary is present.
The repair addresses capability intersection, not the truth of remote claims.
Level3 includes this distinction and the changed-version proof; passing public
test literals without understanding the policy does not earn independent mastery.
