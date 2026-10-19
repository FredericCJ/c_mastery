# Private analysis: X-T68-010 R1-R4

The reference normalizes nodes by logical ID before passing canonical byte
descriptors to the actual retained topological/hash producer. It does not hash
native padding or assume caller storage order is logical order. Each descriptor
is7 fixed bytes plus six length bytes plus at most48 bytes, hence61. The complete
closure representation is at most3+8*(1+61)=499. Including every transitive
descriptor makes exact equality independent of the collision behavior of FNV.
Changing an input outside the closure does not affect this vector or its artifact.

R1: reject explanations that omit environment/schema/include fields or equate
one equal64-bit key with equal actions. Force every lookup key to zero and retain
the independently computed artifact key. Full bytes, not the forced key, decide
a match. This pedagogic cache regenerates its cheap expected artifact, so there
is no claimed generator-work speedup. Artifact equality is needed as well as
input equality: storage can corrupt the value while keeping the key/input.

R2: in each topological iteration at least one new node enters done or no ready
node exists. With all masks validated, a nonempty remainder with no ready node
contains a directed cycle. Dependency keys are available before the consumer
hash and closure union. Runtime workers use a different state partition:
pending/running/done, plus failure. The last successful finish permits publication;
a failed finish cancels only pending work. Epoch and owner checks prevent stale
completion while the control's lifetime remains valid. They cannot make a dangling
owner pointer safe, so joined/quiescent reuse is a caller premise.

R3: the independent Python model uses arbitrary integers, explicit descriptor
concatenation and a separate min-ready set. It enumerates512 three-node dependency
graphs, both collision modes, all six storage permutations,4000 fixed-seed random
graphs/length/extrema cases and empty input:10145 total. Expected distribution is
2039 successes,941 invalid-input refusals and7165 cycles. It compares complete
input/artifact bytes, not merely keys. Tests preserve a sentinel output on failure.
The actual Ninja companion executes the generated table. A stale missing-input
case and a stale order-only case each have a changed independently expected key;
implicit repair and a command-argument change make the key current.

R4:17 controlled child exits include12 partial writes and publication/synchronization
boundaries. The authoritative old/new switch is rename at16. Process death after
rename can show the new entry but cannot demonstrate a power-loss durability
guarantee. Callback refusal after rename reports uncertainty. Do not accept an
explanation that fsync(file) alone persists the directory entry, or that any failed
operation has rolled back. Local filesystem authority excludes NFS ambiguous
rename errors and competing namespace writers. No hostile cached C source runs.

The mutex protects admission/terminal transitions, not the whole generation job.
Workers own separate result copies and must both join before publish/destroy.
The first-job barrier proves two live owners in the supplied run. It is not a
general scheduler progress proof, and unexpected pthread creation failure makes
the harness experiment fail rather than establishing safe partial thread startup.

Compiled controls1..10 omit the environment, trust a colliding key, admit unmet
dependencies, omit running release, ignore artifact bytes, publish live work,
restore retained omitted-configuration MOD19, restore omitted-dependency MOD20,
truncate the authoritative file in place, and compare only an eight-byte prefix.
Require each named ordinary semantic assertion to reject its compiled control.

## Sealed delayed decision

Give a four-node diamond with storage permuted, shared include input, a callback
refusal immediately after rename, and two workers already running when a third
job fails. Ask which outputs can be reused, which work may be reclaimed and what
the caller may report about publication. Then change the filesystem premise to
an NFS server that can report a failed rename after server-side success. A correct
answer preserves the local proof but refuses to transfer the pre-rename IO
rollback claim to that changed environment; it specifies a new reconciliation
protocol rather than silently weakening the outcome type. Implement the local
diamond/forced-collision/worker case without the old source and derive expected
bytes independently. No external NFS target is required or authorized.

Assess all13 cost dimensions from CACHE.md. Distinguish ISO C object/lifetime
rules, POSIX/Linux operations, Ninja dependency semantics, the finite model and
programmer discipline. The expected author packet is not independent assessment,
and finite passing observations do not prove all filesystem or thread executions.
