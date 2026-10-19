# Assessor analysis: numeric grammar and library boundaries

The reference validates the full ASCII grammar before invoking any converter.
This makes syntax errors win over range errors in a malformed long numeric token.
The signed adapter strips only an admitted plus and imports f_parse_i32 unchanged.
The unsigned adapter stages a bounded NUL-terminated token, resets errno and
requires full consumption before limit admission; it never narrows first.

The independent oracle uses Python regex and arbitrary-size integers for9350
tokens: all strings over `01+- x` of lengths0..5 plus signed/range/prefix/embedded-NUL
and length boundaries. It explicitly models C17 octal because Python's base0
policy differs on010. The compiled model probe confirms uintmax_t64 rather than
silently assuming it from the host name. Both compilers run this oracle. These
bounded cases do not prove all possible byte histories or conversion libraries.

The float validator normalizes decimal-rational syntax before a separately owned
locale conversion. It does not prove decimal-to-binary rounding or canonicalize
the resulting double's identity. In particular, two distinct canonical decimal
rationals can round to the same binary value. A false equivalence here loses the
reasoning criterion even if every test passes. A callback that reports partial
consumption or range failure cannot commit; syntax failure never calls it.

Withhold original DELAYED packages:08a adds a leading plus while rejecting both
whitespace and double signs;08b changes the grammar to explicit C17 base0 forms,
including the010 reinterpretation and08/0b10 rejection;08c supplies the canonical
wire contract and adverse backend results. Require implementation, causal
explanation and an independent changed case after the original7-day delay.
Alternative normalized grammars require an explicit approved contract change;
silently accepting generic strtod syntax does not satisfy this package.

The native notebook executes only representable atoi examples, records errno/end
pointer separately, and compares C/C.utf8 with FE_TONEAREST and the actual binary
model. It claims no decimal-comma locale, foreign library execution, portable
undefined output, or independent high-precision rounding evidence. The latter
belongs to T12.13's separate numerical analysis obligation, which remains required.
