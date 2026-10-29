# Operator stacks and transactional token admission

This required PR-T15-09 refinement follows X-T15-012. Retain the learner's actual
`parsing.h`, lexer, tree and checked evaluator. Author runs explicitly substitute
`assessor/parsing_depth/pratt.c`; those runs are not learner integration evidence.
The parser itself must use an operator stack and operand/node-index stack, not
call the Pratt parser or a parser library. Pratt is a separate differential check.

## Representation and grammar

Use C17, arrays of 64 operators/node indices and the existing 64-node `struct pd_tree`.
Input is `struct pd_tokens`, at most 64 non-EOF tokens followed by exactly one final EOF.
Integer token values are nonnegative int32. The original lexer accepts at most
256 bytes; offsets are byte offsets and tokens borrow no input text. Token kinds
are integer0, addition1, multiplication2, open3, close4, minus5, power6, EOF7.
Metadata lengths/lines/columns come from the lexer; this parser uses kind, value
and offset only. A manually supplied stream must meet the same token contract.

Binary + and - have precedence10 and associate left, * has20 and associates
left, prefix minus has30, and power has40. `right_power` selects left(0) or
right(1) associativity for power. This is an original small expression language:
`^` denotes exponentiation here, not the C XOR operator. Prefix minus always
starts an operand and does not reduce an awaiting binary operator. Therefore
`-2^2` means `-(2^2)`, `2^-3^2` means `2^(-(3^2))`, and `2^--3` is valid.
Parentheses group without emitting a node. No implicit multiplication, unary
plus, floating literals, comments, identifiers or function calls are accepted.

Leaves have kind0, value and source offset, zero child indices. Operator nodes
have kind1/add,2/multiply,5/negate,6/power,8/subtract, value0 and operator offset.
Children precede their parent; unary uses left and has right0. Emission order is
postorder, root is the only remaining value-stack entry, and every emitted node
is reachable exactly once. The existing evaluator checks signed int32 arithmetic,
permits exponents0..31 and defines 0^0=1. A syntax-valid tree may return PD_RANGE
or PD_DOMAIN when evaluated; that is not a parser rejection.

## Operations, ownership and failure

`sy_begin(mode, operator_capacity, node_capacity, out)` admits mode0/1 and
capacities0..64, creating an empty prefix that expects an operand. Zero capacities
are meaningful: literal-only input needs no operator slot but does need one node.
PD_INVALID means a null required pointer, invalid configuration, malformed token
or syntax. PD_RANGE means an admitted finite budget would be exceeded. Whole
`sy_parse` requires a final EOF and returns PD_OK only for a complete expression.
Missing/early EOF, empty expressions, unmatched parentheses, adjacent operands,
and trailing operators are invalid. On failure, tree and cost outputs remain
unchanged. Diagnostics may change: offset is the rejected token or EOF position,
expected is a bit mask of helpful token kinds (zero for capacity/configuration),
steps counts consumed token attempts. It is not a guaranteed minimal edit repair.
Budget failure may precede a later syntax error. State the admission order when
comparing two implementations; do not require one global error for mixed faults.

`sy_push` consumes one token into a successfully initialized valid `struct sy_state`.
PD_PENDING admits a non-EOF token, PD_OK admits a completing EOF, any other result
leaves the entire prior state unchanged. It rejects all tokens after completion.
PD_RANGE also applies to a 65th non-EOF token independently of other capacities.
`sy_result` returns PD_PENDING without changing outputs until EOF is accepted.
It then copies the tree/cost without consuming state. No external pointer is
retained; input buffers may be released after the call. The state is single-owner,
serialized and caller-owned. Caller corruption of internal fields, overlapping
live arguments, dangling pointers and concurrent mutation violate preconditions;
these C structs are not opaque capabilities or data-race prevention mechanisms.
Neither API allocates or frees. Diagnostics may be null. Other null pointer
arguments return PD_INVALID. On all configuration failures out is unchanged.

## Construction and worked trace

Keep an explicit `want_operand` state. In operand position accept a literal,
opening delimiter or prefix minus; in operator position accept a binary
operator, closing delimiter or EOF. Before an incoming binary operator, reduce
stack tops with greater precedence, or equal precedence when incoming is left
associative. Stop at an opening marker. A prefix minus is pushed without this
binary admission reduction. Close reduces to its matching marker then discards
the marker; EOF must find none. Pop right then left for a binary reduction.

For `1+2*3`, emit leaf1; hold +; emit leaf2; hold *; emit leaf3; EOF emits * over
leaves2/3, then + over leaf1/the new subtree. The node count is5, operator pushes2,
reductions2, peak operator depth2, peak value depth3. Compare `9-4-2`: equal
precedence causes the first subtraction before the second is pushed. The modes
give `2^3^2` values64/512; a rule that always pops equal precedence cannot implement
both. Parent markers shield outer operators but do not themselves produce values.

Dijkstra's actual 1961 ALGOL Bulletin Supplement10, second-part pp2–3 and7–10,
describes translation-time precedence, the operator-stack junction and bracket
shielding. Its printed priority chart groups unary negation with multiplication
and its example pop rule uses greater-or-equal. Our grammar, prefix rules,
right-associative variant, budgets and transactional API are explicit curriculum
extensions; do not attribute their exact semantics to the historical translator.

## Costs and enforcement

Each admitted operator is pushed once and reduced once; each delimiter is pushed
and removed once. Whole parsing uses O(n) stack actions and O(n) live storage for
an n-token generalized implementation; these arrays impose a finite64 bound.
Whole parsing uses one private staging state. The streaming strong-failure
guarantee uses one full private state copy per admitted or rejected token. With
capacity C that adds O(C) copy work per token and O(n*C) total copy work; fixed64
does not justify hiding this cost when generalizing. `struct sy_cost` counts stack work,
not copying, instructions, cache misses or elapsed time. Measure both paths under
the same accepted-expression and rollback contract, retaining source identities.

C17 type/array rules enforce neither valid typestate nor syntax. Explicit runtime
checks enforce admitted budgets and transitions; assertions, the independent
grammar model and generated-code audits provide bounded evidence. A language with
algebraic data types or affine ownership can express additional constraints, but
does not automatically prove this precedence table, failure atomicity or costs.
No C23 language feature is required; no unexecuted C23 comparison changes these
claims. Strict target core excludes libc, allocator and compiler runtime archives.
Any generated memory helper must resolve to the repository's original audited
support, with actual undefined-symbol/link-map checks. Host stdio, clock, Python
and CMake are verification tools, not target dependencies.

## Experimental and transfer obligations

Compare all tree fields against the actual earlier Pratt implementation and
evaluate using its actual checked evaluator. Also use an independent recursive
grammar/exact-integer oracle, since two C parsers can share a misconception.
Exercise malformed streams, maximum prefix depth, zero/exact/insufficient budgets,
left/right associativity, unary interactions, arithmetic overflow/domain errors,
and every incomplete streaming prefix. Keep syntax errors distinct from evaluator
errors. Inject rejection after private reductions (an unmatched opening marker at
EOF), then complete that same prefix with a close and EOF. Prove no half-built
tree is published. Run the named compiled defects and require their intended
semantic assertion, not a compiler failure or sanitizer crash, to reject them.

The delayed package changes whole-input admission to resumable tokens with a
strong rollback guarantee. It is implemented in the separate assessor provider
today, but learner performance must be obtained at least seven days later without
disclosing that implementation. Require the learner's actual prior lexer and
evaluator, exact budgets, a resumable rejection demonstration and a copy-cost
analysis. There is no external system or hardware in this experiment.
