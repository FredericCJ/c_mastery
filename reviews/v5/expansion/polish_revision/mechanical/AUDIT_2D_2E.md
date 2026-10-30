# Object-level audit of the format (2d) and brace (2e) steps

## Why

The 2d and 2e proofs were token-level. A token proof cannot see three
effects, and the language_portability rewrite found all three in
lab sources:
- `__LINE__` changing when a line moves;
- a stringified argument (`#x`) whose spacing changes;
- braces wrapping a macro invocation that expands to several statements.

## Method

`tools/source_equivalence.py objects --neutral-line --squash-strings`:
- each changed unit is compiled before and after the step, with
  `-D__LINE__=0`, at `-g0`;
- the code and section bytes are compared;
- strings that differ only in whitespace get their own class.

Audit runs cannot be recorded as proofs (`record` refuses them).

| Step | Base -> commit | Identical | String whitespace only | Different | Not compiled |
| --- | --- | --- | --- | --- | --- |
| 2e braces | 7004a7e3b -> e70609563 | 1309 | 0 | 4 compile mismatches | 851 |
| 2d format | f6f8d8c07 -> 7004a7e3b | 1666 | 293 (+3, see below) | 3 | 1187 |

## Findings

- **2e:** the 4 compile mismatches (reclamation.c, ecc.c, concurrent.c,
  cas.c) are the files misbraced at e70609563 and repaired in 6c5fc73ee and
  3434dbdf5. Rechecked against the pre-brace base at the current tree, all 4
  are identical (`audit_2e_repaired_four.json`).
- **2d:** the 3 "different" units are these:
  - consume_index_depth/tests.c;
  - ownership/references/tests/fence.c;
  - verification_fuzzing/targets/transfer_cbmc.c.

  In each, the code is identical up to objdump label annotations, and only
  stringified CHECK/assert expressions changed spacing. GCC's string tail
  merging hid this from the whitespace class. So 296 units changed only the
  spacing of printed stringified expressions. The checkers compare such
  markers ignoring whitespace (commits 1f13170b6, 3e9fe307b, 0ceead76f).
- **Not covered by this audit:**
  - units the heuristics could not compile (851 and 1187);
  - preprocessor branches not selected by default (probe CASE values,
    mutant macros);
  - behaviour that depends on `__LINE__` itself, which is neutralised here
    by design.

  These rely on their producers. The language_portability probes (wrappers
  cases 6 and 7, lexical spellings) are known instances and are repaired in
  that family's batch. A full producer sweep (617 producers) is running.
