# T12.10: progress is not record commitment

Required PR-T12-03 T12.10a/b/c and their original delayed variants, families
A/B/C/D/F/I/J. C17 hosted profile, synthetic local streams only. Prerequisites:
counted slices, integer range, variadic pointer types, actual FCursor/f_take and
f_parse_i32 learner artifacts, and the preceding lc_unsigned conversion adapter.
Read R-LLIB-N1570-SCANNER; it is a precisely identified C11 draft passage.
Complete scanner.c from scanner.h; reference/oracle material remains withheld.

First use %3s to read ABC from ABC! into char dst[4]. Record the assignment count,
the terminator and %n consumption; %n does not itself add an assignment. A field
width bounds characters consumed for that conversion, not the entire record.
The matching pointer type is mandatory. Analyze `%d` with a long* and numeric
input outside int's range: neither is a safe run-to-reject experiment. Compile
the wrong-pointer translation separately for the intended diagnostic; do not
execute it. Numeric nonrepresentability is an undefined-behavior premise under
the inspected rule, not an expected scanf failure return.

Build a cursor parser for the exact complete record
`count=[0-9]+;name=[A-Za-z0-9_]+`. Count must fit nonnegative int32_t; the name is
1..32 decoded bytes. No whitespace, sign, NUL or trailing bytes are admitted.
Use the actual prior cursor and signed parser, not sscanf or a cloned provider.
Output is a disjoint live struct ls_record; all rejection paths preserve its
entire previous value. Stage fields, then commit once. Report LS_RANGE for a
well-delimited overflowing count, LS_LONG for an otherwise decoded name beyond32,
and LS_SYNTAX for grammar failure. Multiple defects may expose the first checked
failure; only acceptance and unchanged-output guarantees are global.

The changed name grammar adds `\;` for a literal semicolon and `\\` for a literal
backslash. Draw ordinary/escape states and the end-of-input transitions first.
An ordinary unescaped semicolon, unknown escape, dangling backslash or empty
name is invalid. Limit decoded length, not raw escape length. For example
`count=12;name=A\;B\\C` publishes count12 and name `A;B\C`. Ordinary mode rejects
it. An arbitrary suffix such as ! must never silently end a successful record.
State the cursor invariant0<=position<=length and prove every decode iteration
advances; staging is bounded33 bytes and parsing is O(input length), no allocation.

Compare a scanf pattern with explicit total-consumption checking on a missing
delimiter, an empty name and trailing !. A successful count assignment can occur
before a later literal or name fails. That observation cannot authorize partial
publication into the public record. A valid alternative transactional parser is
acceptable if its grammar, independent expected results and rollback agree.

For each of12x,x,empty and12 at EOF, create a finite controlled FILE stream.
Record the initial EOF indicator, scanf assignment count, assigned value, the
next unread byte and flags after the attempted read. Expected assignments are
1,0,EOF,1; the unread suffixes are x,x,EOF,EOF. Explain matching failure versus
input failure and why feof cannot predict an operation that has not happened.
Only these representable numeric tokens are passed to scanf.

Implement ls_line for finite byte-oriented streams. It consumes one full line
including newline, returns the payload without newline and NUL terminates it.
Capacity is1..129; successful payload length is at mostcap-1. Empty lines succeed;
EOF without any new byte returns LS_END; an unterminated final nonempty line
succeeds. Oversized lines are completely drained and return LS_LONG without
publishing partial output/length. Read error returns LS_IO with output unchanged.
Input consumption cannot be rolled back. A live unbounded producer requires a
separate timeout/budget/cancellation contract; this finite-file exercise makes
no such liveness promise.

Run the recovery sequence oversized line,12x,42,empty line,unterminated7. The next
attempt must begin at the next record, not in the oversized suffix. On each
finite-file iteration either bytes are consumed or EOF/error terminates the
loop. Feed the complete token to the actual lc_unsigned strtoumax adapter with
explicit target limit. Include128-byte zero-padded9 and128 nines: the former
succeeds, the latter fails range without changing the previous result. Arbitrary
bounded length means the declared128-byte policy, not an unbounded allocation.

Deliver the state machine, cursor/commit reasoning, traces, implementation,
source manifest and discriminating cases. Six GCC/Clang O0/O2/sanitizer profiles
must pass. The assessor compares both escape policies with an independent
regular-language and unbounded-integer oracle over all five-byte strings from
the declared four-character alphabet plus length/NUL boundaries. Compiled
premature-commit, accepted-suffix and incomplete-drain mutants must fail their
intended assertions. Finite oracle coverage is bounded evidence.

Rubric:0 trusts scanf as range validation or publishes failed fields;1 solves
the ordinary example;2 derives all grammar/progress/ownership obligations and
diagnoses each defect;3 independently implements the original bounded-token,
escaped-name and newline-recovery transfers after at least7days. Hints: separate
assignment from commitment; track decoded length; consume through the recovery
delimiter. Author runs do not supply delayed learner or non-author review.
