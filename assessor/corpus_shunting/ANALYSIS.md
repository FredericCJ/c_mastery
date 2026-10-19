# Withheld shunting-yard solution and assessment

The reference implementation in shunting.c is separate from the refusing learner
provider. Public tests use manually constructed literal tokens before invoking
the prerequisite lexer so a scaffold refusal cannot be blamed on an incomplete
prerequisite. Oracle and full solution must never enter learner/source packages.

## Invariants and correctness argument

Induct on admitted tokens. The value stack names complete disjoint subtrees;
pending operators separate those subtrees and an optional expected operand.
Every tree node emitted so far belongs to one value subtree. Opening markers
partition pending operators into nesting regions. Within a region, binary
admission has reduced exactly the operators that must precede the incoming one.
Prefix minus is special: it begins an operand and must not pop the binary operator
awaiting that operand. For `2^-3`, treating prefix minus as an incoming ordinary
precedence30 operator would pop power40 before its right operand exists.

A binary reduction takes right from the top, then left, and emits their parent.
A unary reduction uses only left and writes right0, matching the actual prior
tree ABI. This preserves child-before-parent and reachability. Associativity is
determined by equality handling when admitting a binary operator: left reduces
equal; right retains it. Parentheses stop reductions at their own marker and
produce no node. Successful EOF removes all operators and requires one complete
subtree. Termination follows from finite tokens and decreasing operator count on
every reduction. Stack capacities and token bound are checked before storage.

The whole call operates on one private state and publishes tree/cost only after
EOF. Streaming stages a complete prior state before a token; any failed private
reduction is discarded. Diagnostics are separate output permitted to change.
This is RAM call atomicity, not thread safety, signal safety, crash recovery or
durability. An opaque handle with private state is another valid representation;
an undo log can replace full copying if every mutation and failed undo is handled.
Do not accept a claimed O(n) streaming cost that counts reductions but omits
O(C)-sized staging copies. Fixed-capacity stack size must be measured in actual
target generated objects; the core may use compiler-generated memory calls even
with no include or explicit source call to memcpy.

## Discriminating cases and common incorrect answers

`2^3^2`: right512, left64. `-2^2`: -4, not4. `(-2)^2`:4. `9-4-2`:3,
not7 or -3. `2^--3`:8. `2^-3^2` parses successfully but evaluator returns
PD_DOMAIN without changing its output. `0^0`:1 under the explicit inherited
contract. `2147483647+1` parses but evaluation returns PD_RANGE. `1 2`, `2(3)`,
`(1)(2)` and `()` are invalid; counting balanced delimiters alone is insufficient.
Literal1 succeeds with operator capacity0/node capacity1. `1+2*3` needs operator
capacity2 and five nodes; insufficient capacity must not expose emitted leaves.

Seven compiled defects are necessary discriminators here: always-left power,
unary-before-power, reversed subtraction operands, early publication of a tree,
publication of failed streaming state, accepting post-EOF tokens, and accepting
negative integer tokens outside the lexer contract. Each builds and fails its
named assertion; compilation failure is not a successful mutation experiment.
The independent Python oracle uses recursive grammar rules and exact integers,
not the stack algorithm. Its failure ordering evaluates left subtree before right
and checks each intermediate; an exact final result alone can conceal an earlier
overflow. Pratt comparison adds field/offset checking but shares the earlier
grammar contract and evaluator, so is not itself an independent mathematical proof.

## Exact rubric evidence

R1 / T15.14.a: derive precedence, unary and equal-priority rules, show a full trace
and a counterexample to blindly copying the historic greater-or-equal rule.
R2 / T15.14.b: implement AST emission and budgets with original stacks, prove
ownership/failure invariants, distinguish syntax from evaluation and use actual
prior interfaces. Reject superficial coverage from calling pd_pratt in sy_parse.
R3 / T15.14.c: independently falsify malformed input, rollback and changed
assumptions, inspect core generated dependencies and account for streaming copies.
Each criterion needs its own explanation and evidence, not a single green CTest.
Level0 contradicts the contract; level1 demonstrates examples only; level2 derives
and verifies the criterion on stated bounds; level3 independently transfers it to
the withheld variant with new expected results and honest enforcement limits.

## X-T15-018-DELAYED: resumable rejection after reductions

Release at least seven days after ordinary completion. Supply only the learner
API contract, their actual X-T15-012 artifacts and tokens for `(1+2`, capacities
operator3/node5, right-power1. Withhold reference code and this analysis. Ask for
token-at-a-time publication with immutable failure state, then supply EOF before
the close. The first four non-EOF tokens are PD_PENDING. EOF must return
PD_INVALID after a private + reduction discovers the unmatched opening marker;
the retained prefix still has exactly two
literal nodes, operators '(' and '+', and two value entries. A candidate retaining
the private reduced node has already violated rollback even if eventual value3
is recovered. Compare complete state bytes before/after rejection. Then admit ')'
and EOF; result is three nodes, root2, value3. Repeat the delayed condition with
`2^3^2` in both modes, null diagnostics, and a capacity exhaustion followed by a
legal alternative token. Reject all input after EOF; repeated result inspection
is allowed. Require independent expected trees, a source-current actual prior
dependency run and whole/stream copy-cost comparison.

The implemented public stream anchor and private oracle execute these reference
conditions now. Actual delayed learner records remain pending. Record when each
hint was released: (1) distinguish pending operators from completed subtrees;
(2) isolate the publication point; (3) stage one complete state or justify a
complete undo log. Alternative correct implementations may report a different
helpful expected-token mask, but must meet stated offsets, success/failure and
unchanged-output rules. Never infer independent mastery from author execution.
