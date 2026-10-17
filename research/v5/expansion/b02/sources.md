# B02 inspected sources and author reflection

On 2026-10-17 the author opened N1570 and read the exact clauses registered in b02_resources.json, using bounded in-memory pdftotext passages for preprocessing, generic selection, union access and strto conversion. No private extract is published. The C17 baseline is preserved; N1570 is explicitly a C11 draft. Access to exact published C17 text is still a gap, and existing B01 correction records remain separate.

The ODS linear-probing page was reached from chapter contents after two guessed URLs failed. The inspected URL ends in 5_2_Linear_Probing.html. Its half-empty invariant cannot be copied unchanged into a full bounded table: probing must terminate without an empty sentinel, and insertion must search past tombstones for duplicate keys. The inspected analysis states probabilistic hash assumptions; our modulo-hash inputs deliberately violate them.

ODS 11.1.1 was read through its recursion-tree argument. The displayed merge selects the right item on equal comparison. This is correct sorting, but fails this exercise's added stability contract. The exercise requires evidence of that distinction and caller scratch admission; it does not label the source algorithm wrong under its own contract.

Existing PR-T06-01, PR-T12-03, PR-T14-04, PR-T15-01, PR-T16-02, PR-T17-02, PR-T19-02, PR-T21-01 and PR-T21-02 were read. Their contracts already contain important semantics. New APIs, independent oracle obligations and explicit rubric criteria deepen selected claims without replacing the rest of those projects. Historical hours and acceptance records stay unchanged. References not opened in this pass remain uninspected for this pass.

Open gaps include exact published C17 wording, physical target behavior, learner performance, independent human assessment, dynamic rehash/chaining, generic erased owning containers, failure-capable destructors and durable transactions. The per-leaf disposition ledger tracks other B02 claims without inferring coverage from titles.
