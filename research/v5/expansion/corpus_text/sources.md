# Inspected sources and adaptations

Bison3.8.1 Chapter6 was read from the original captured HTML. It distinguishes stack/context discard from token discard, statement and close-delimiter synchronization, possible cascades and reclamation of discarded semantic objects. Our original adapter has typed automatic temporaries and an explicit statement boundary; it does not claim a Bison-generated compiler or generic context preservation.

Wadler's author PDF Sections1–3 was read for the algebra, group/flatten, continuation list and representation derivation. The original lazy selection and complexity arguments do not automatically transfer to this bounded eager C probe. The contract states a deterministic greedy continuation rule, counts repeated probes and exposes DAG expansion/fuel limits. Oppen1980 is a metadata discovery lead and a related-work citation in Wadler, **not an independently inspected paper**.

Parr2004 selected Sections6–7 was read for restricted constructs, list application, presence tests and formal separation. The C instruction set is an original bounded adaptation; model construction and stable aliases remain caller obligations. No full formal separation proof is claimed.

OWASP's current original HTML-context encoding and adjacent context contrast were read through web rendering. A direct urllib request returned403, so no local byte capture is asserted. The laboratory demonstrates only a synthetic string-property violation, with no browser, server or real data. Context-specific encoding advice is not a proof of universal injection safety.

Private original PDF/HTML captures are under ignored build/corpus_text_sources. Published records retain identities and locators, not book/paper extracts. Exact published ISO C17 text remains a separately recorded source gap; these sources are not substitutes for it.
