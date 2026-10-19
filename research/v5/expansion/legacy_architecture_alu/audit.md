# PR-T37-01 actual mechanism audit

Read actual original project, fixture and all three supplemental criteria/V1
tasks. The retained algo_alu4 is a real four-slice adder with value/carry/overflow
and masks its incoming carry. It has no operation/status contract, subtraction,
Z/N fields, explicit invalid-output preservation or width5 mechanism. The new
component preserves that producer and imports its exact body for comparison.
It adds the missing ordinary and changed mechanisms, not another topic heading.

Actual primary PDFs were retrieved at their original registered URLs and hashes
verified: Arm DUI0553B, DDI0403E.e and Intel volume2A revision093. Read DUI
printed3-19/20,3-41/42,3-49; DDI179 and205/206, including a rendered visual
inspection of the T1 branch bitfield; Intel printed3-499 and3-502. Initial web
screenshots timed out and Intel web extraction failed; successful private PDF
retrieval/local rendering supplied the actual inspection, not the failed calls.
No private source extracts are published. N1570 section6.5.7 paragraphs3–5 was
read from accessible primary text; C11 draft is not relabeled final C17/C23.

DUI0553B Table3-4 LE prints AND. DDI ConditionPassed instead derives OR as the
inverse of GT. The successor uses that architecture definition, checks equality
and documents this precise disagreement without changing old evidence. No new
Plantz whole-book or held-page inspection is claimed. The gate derivation is
original, and prior assigned reading remains separately bounded.

The campaign executes both32-bit ISAs on high-aligned finite operands; it does
not confuse four/five-bit logical registers with physical register widths.
Private implementation and oracle stay under assessor; learner exports contain
contracts, original test targets, assembly placeholders and producer stubs only.
The first optimized mutant build produced a may-be-uninitialized diagnostic;
its73-command failed receipt is retained. Reading a checked slice output now
requires success, with no weakened warning flags. All later T37 work remains
required, as do external review, independent delayed learning and physical work.
