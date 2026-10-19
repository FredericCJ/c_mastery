# H745 cache/peer assessor analysis

The reference keeps the last line address representable by testing the remaining
length before increment. A naive `end=address+length` wraps on the last valid
physical line. Rounding outward would accept a borrower that cannot authorize
invalidation of dirty neighbours. The correct answer rejects that input or
changes allocation/ownership; inserting a barrier does not recover lost bytes.

The cache trace is DSB,address operations,DSB,ISB. The second DSB completes the
operations before HSEM publication. ISB does not substitute for it. M4 shares
bus memory but not the M7 cache; HSEM is mutual exclusion, not automatic cache
maintenance. The noncacheable baseline removes this cache ownership requirement
only for the specifically mapped storage. Neither case grants arbitrary DMA
coherence. Masking PRIMASK serializes ordinary local interrupt entrants but not
NMI/fault reentrancy or a second user holding process0 outside this protocol.

Generation equality prevents stale work from returning new reservations.
FAIl_PEER never zeroes outstanding counts, so it cannot transform a timeout into
quiescence. `confirmed` records only current readiness; it is not the journal
commit marker. Failed health after one acknowledgment prevents readiness. A
failure after readiness clears it and leaves the durable journal question to
the actual transaction layer. Accept a design that binds a persisted generation
under explicitly bounded failure assumptions; reject claims that this volatile
word provides crash consistency or proves continued health after the check.

Objective-specific assessment: T59.04.a derives the joint-generation commit
boundary and its limits; .b implements both peer orderings, failure after the
first acknowledgment, and outstanding-drain invariants; .c independently changes
the failure/commit boundary and explains the necessary persistent adapter.
T59.07.a derives cache-line ownership from the selected manual, distinguishing
ISO C/compiler/ISA/SoC roles; .b implements and diagnoses missing completion,
dirty-neighbour and stale-generation failures; .c transfers to interrupt/DMA
publication and supplies actual dedicated-board traces before claiming physical
acceptance. Passing the author tests does not award any independent criterion.

Withheld changed condition: a DMA completion for generation8 arrives after
generation9 has been staged; the M7 also has an unrelated dirty byte at offset31.
The correct repair cannot be 'invalidate the requested3bytes and decrement the
current counter'. Require an exclusive aligned allocation, a generation-qualified
completion and proven DMA quiescence before reuse. A simulator can demonstrate
the violation, but the physical variant must observe actual controller/cache
behavior. Keep those evidence states distinct.
