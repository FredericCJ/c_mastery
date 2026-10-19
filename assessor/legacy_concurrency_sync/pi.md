# Assessor: exact T51.04 successor

Inspect the original criterion IDs, the public contract and the actual CMX
wait graph before grading. The reference uses bitset reachability instead of
copying CMX's iterative priority relaxation. In a valid finite acyclic graph,
the effective priority of v is the minimum seed of v and every vertex that
reaches v. A seed is its base or applicable immediate ceiling, whichever is
more urgent. Reachability includes all path lengths after the final
intermediate vertex. A diagonal bit detects a cycle before publication.
Numeric minimum is monotone and idempotent, so this is the required least
donation fixed point; the order of individual source-edge insertion is irrelevant.

The reference uses three16-element arrays (two unsigned, one uint32_t) plus
constant locals. It makes256 closure probes and256 donor-pair probes at the
fixed limit; it does not allocate. Parameterizing N costs O(N squared) bitset
word operations while N fits one word, plus registration validation scans;
generalizing beyond the word requires a different representation and analysis.
Do not repeat a generic O(N cubed) claim without stating whether a bitset union
or an individual Boolean cell operation is the unit of cost.

L9/M7/H3 yields L3/M3/H3. After H3 expires, H5 still donates directly to L;
M returns7 and L remains5. Canceling M does not remove H5's separate edge.
When the final H5 edge disappears, L returns9. A base11 assigned under donation
must later restore11. Replacing the base with the donated3 destroys information;
retaining a previous effective3 after raising a donor to6 is a stale donation.

With L holding A and B, H3 on B and H5 on A, releasing B transfers B to H3
and leaves L5. The final A wait removal returns the newly assigned L base12.
Cycle rejection must preserve both owners and all preexisting memberships.
Refusing destruction/reprioritization is different from silently removing a
wait edge whose resource remains referenced.

The actual scheduler trace is LLLLLH with inheritance and MMMMMMLLLLLH under
the explicit base-priority projection. This projection is an experimental
comparison policy, not an alternative CMX implementation or hidden producer
mutation. L has five units of remaining nested work in both traces; the six
M units appear in H's delay only without inheritance. H costs one unit after
acquisition. Neither trace includes unknown scheduler, interrupt or cache costs.

For the restricted delayed response-time worksheet, a task with C=2, a
justified single blocking B=5 and one higher-priority task C_h=1,T_h=10 has
R0=7, R1=8, R2=8 in the usual workload fixed point. This derivation is original
course analysis under the explicit assumptions in the contract, not a claim
to have read an unavailable response-time paper. Deadline8 is feasible in
that model; an added unit of nonpreemptible interference invalidates that
conclusion. Multiple resource requests or nested dependencies require an
independent blocking argument before substituting B. The host cost samples
are useful experimental inputs, not certified execution bounds.

The five-task oracle enumerates only owner edges to larger-index tasks or no
owner, thereby covering all graphs in its declared acyclic ordered universe.
It independently walks each donor's supplied owner path. It does not cover
arbitrary ID permutations, all16 tasks, cycle handling or ceiling protocols;
the focused tests cover selected additional cases. Sanitizers and finite
enumeration cannot prove liveness, arbitrary weak-memory behavior or mastery.

Keep the completed implementation private. Require the learner to explain
counterexamples before crediting the exact per-objective level3 rubric.
