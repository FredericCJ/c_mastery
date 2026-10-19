# T26 histories and progress source audit

Read original PR-T26-01 T26.01.a–c/T26.02.a–c ordinary/delayed tasks and their
project/module contracts. The retained CMX complete-history checker is not the
required changed SET/pending policy; the T25 history successor has a different
capacity3 contract. This packet implements a separate bounded capacity1–4
checker and compares it to an independent oracle instead of silently reusing
the wrong interface. It imports actual CMX queue operations unchanged for Tail
helping and retired-node admission experiments.

Direct primary inspection2026-10-17: Herlihy/Wing section2.1–2.2, printed467–469,
defines histories, well-formed participants, real-time order and pending
extension; source-paper FIFO empty behavior differs from this deliberately
totalized bounded object. Michael–Scott corrected HTML complete nonblocking
pseudocode E1–E17,D1–D20 plus introductory allocator premise; original PODC1996
paper sections2,3.1–3.3 including liveness argument and ABA premise. The corrected
HTML names two repaired typos and the type-preserving allocator limitation.
These are inspected passages, not inferred bibliographic coverage. No original
source text or private corpus extracts are republished.

Actual code inspection: assessor/execution/concurrent.c complete SPSC/HP/epoch
implementation, cmx.h types/operations, execution README queue contract; and
advanced.c complete-history search/reservation implementation. In particular,
CMX has an explicit collector and a bounded per-slot allocation scan. Failed
allocation is not an atomic global full-pool snapshot. The successor requires
that distinction; original source/acceptance is unchanged.

This wave covers six ordinary and six delayed criterion IDs only. It does not
close LRUN-18's remaining reclamation/construction scope, LRUN-19's kernel scope,
final C17/C23 source access or independent/physical evidence.
