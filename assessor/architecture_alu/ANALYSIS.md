# Withheld ALU and branch analysis

## T37.02.a

Exactly one or three asserted input bits gives sum1; at least two gives carry1.
The weighted slice equations telescope to the full sum with final carry. The
selected complemented subtrahend plus initial1−borrow represents the required
difference modulo2^width. Width5 introduces1024 operand pairs per operation/input
but no new Boolean equation. Reject width before computing a shift; unsigned
arithmetic alone does not authorize shifting by its width. The scalar gate
routine's returned status is checked before reading its output even though the
inductive bit invariant establishes valid inputs internally.

## T37.02.b

For N values, a+b≥N has N(N−1)/2 pairs. Extra1 includes the N pairs whose sum
is N−1, yielding N(N+1)/2. Borrow is symmetric under exchanging operands, with
equality added when borrow-in1. Signed overflow count is N²/4 for each input
mode; enumerate signed ranges independently to verify it, rather than assuming
carry counts apply. At width4 ordinary result flags for7+1 and15+1 distinguish
V from C;8−1 distinguishes N from corrected signed order. Z implies result0,
so N must be0. The whole staged output, including unused trace entries, is
unchanged on unsupported operation/width/out-of-range input.

The actual retained `algo_alu4` body is extracted without rewriting and compared
for all width4 addition pairs and both carry inputs. Its masking behavior is
not used as a public admission guarantee. The new component adds subtraction,
explicit flags, complete traces and rejected-input publication semantics.

## T37.02.c

Unsigned less after subtraction is x86 CF or inverse Arm C. Signed less is
N xor V, not N alone. Equal implies neither greater nor less. For small-width
signed patterns, zero-extension before32-bit CMP changes the interpretation.
High alignment preserves both signed and unsigned order and aligns the sign
and overflow tests. Native assembly saves the original flags before composing
its result mask and repeats CMP before each branch, because flag-changing mask
instructions must not contaminate later conditions. Thumb retains APSR in a
caller-saved register and uses short conditional branches outside IT.

DUI0553B's printed LE AND condition is falsified by equal operands: Z1,N=V
should take LE. DDI's base GT condition is `!Z && N==V`; inversion gives the
required OR. The actual JLE/BLE behavior is independently observed here. Exact
short encodings are inspected, assembled, dumped and checked; target address
calculation remains bounded to the selected forms, not a general decoder.

The four mutants are swapped carry/V wires, ignored borrow-in, masking an
unsupported operation into a supported one, and using N alone for signed less.
Each target run selects its intended case and must fail that assertion. Preserve
the first optimized-mutant compiler diagnostic: it was not a mutation pass.
The code now checks slice status before consuming output, with no diagnostic
suppression. Tests cannot establish physical timing or independent learning.
