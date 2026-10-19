# Merkle proof/repair assessor analysis

The RFC raw root is distinct from our application commitment. The outer02,
BE32 domain/count and raw root bind the selected interpretation. The retained
u32 producer cross-check is therefore performed before wrapping. Proof shape
is derived from count/index; extra hashes cannot be silently ignored, and an
index modulo count is not an admissible repair. The Python generator recurses,
while its separate verifier uses the iterative fn/sn condition.

Selective repair validates the root response before trusting child expectations.
Every skipped local range has the same domain/count/range identity and a matching
raw hash. Every fetched child must match its expected hash. Final root recompute
also checks the staged tree. Missing or inconsistent later nodes cannot expose
an earlier staged leaf. Direct mode2 intentionally delays leaf authentication
until its final whole-tree check; it preserves the identical publication rule.
This makes it a legitimate full-leaf baseline, unlike an unnecessarily expensive
all-node walk alone.

The actual parser handles arbitrary chunk boundaries and rejects incomplete EOF.
Protocol fixed8-byte frames, exact raw length, nonce/range echo and zero padding
are additional constraints. A checksum/root does not authorize a peer or its
chosen domain. An attacker who supplies the trusted root is outside the stated
anchor trust; correctly verifying that root says nothing about authorization.

Client hash counts122/64/48 for sparse/all-node/direct modes expose repeated
local hashing. CPU samples include the synchronous peer, while hash counters
exclude its internal computations. Fewer bytes may be slower locally; require
all three measurements and the same final-root contract. Do not infer WAN
latency, constant-time behavior or collision resistance from these runs.

Grade early publication, partial proofs, wrong index/count/domain, callback
retention or overstated security as failures irrespective of timing. Ten normal
semantic controls and compiling learner rejections are required. The independent
finite oracle and agent source review are not human-independent assessment or
the learner's delayed transfer evidence.
