# Exact free-effect transfer analysis

The original tests cover the 64-node shared DAG for terminal 0/1, but their
full-capacity check calls `ho_pure`, not the required identity `ho_bind`. The
companion executes the latter. No provider algorithm or original stub changed.

In the full arena, bind descends through at most 63 Choose nodes. Its first Pure
invokes the identity callback; that callback's constructor consumes attempt 65
and returns HO_FULL. No copied Choose is allocated. The outer bind restores mark
64, does not publish the local result, and retains the callback count. The old
root still interprets with 63 answer bytes. The abstract identity law compares
meanings only when both constructions succeed; it does not promise allocation
trace equality or success under insufficient capacity.

The -1 terminal is important: 2^63 paths sum to INT64_MIN, which is admitted.
Replacing a signed overflow test with a positive-magnitude bound can incorrectly
reject that case. The +1 sum exceeds INT64_MAX and leaves all three summary fields
unchanged. Memoization operates over 64 physical nodes while multiplicities are
added across both edges; counting a shared child only once would be incorrect.

In the three-node source, traversal allocates Pure, Pure, Choose, Emit. Failure
cuts 1..4 consume respectively 1..4 attempts and invoke the callback 1,2,2,2
times. Old nodes and root outputs are unchanged. A callback that first allocates
and then fails, or supplies invalid root zero, also rolls back the arena mark.
Its external call counter remains changed. The contract forbids retaining any
new root from a failed bind; rollback is not destruction of arbitrary external
effects or a transactional C execution facility.

Three actual compiled controls target the new boundary: identity shortcut at full
capacity, omitted bind rollback, and leaked summary publication on overflow. The
runner checks intended MF_CHECK diagnostics, separately from compiler/sanitizer
failures. Finite native observations do not prove all callbacks or delayed learner
mastery. Existing independent 119-trace/free-final and cost reports retain their
separate scopes.
