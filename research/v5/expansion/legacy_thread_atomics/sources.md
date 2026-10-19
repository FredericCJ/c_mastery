# Actual source inspection and source-access limits

The author read the listed primary passages directly during this T25 wave;
no title, search result, census or previous agent's description is treated as
passage inspection. These records supplement rather than broaden accepted
resource IDs. The original project, all twelve criterion objects and their
changed tasks were read before implementation. The original synthetic fixture
was inspected: it is an abstract dependency closure, not an atomic execution
or emitted-helper experiment. It is retained and separately checked.

* WG14 N1570, C11 committee draft,3.14 definition/example;5.1.2.4 paragraphs1–28;
  7.17.2.2;7.17.3 paragraphs1–10 and7.17.3.1;
  7.17.4 through7.17.4.2;7.17.5/.1;7.17.7.1 through7.17.7.4.
  The reads include the memory-location/adjacent-bit-field examples, dependency
  exclusions, data-race definition, atomic initialization, order/fence rules,
  lock-free query description and CAS expected/spurious behavior.
* WG14 N3096, April2023 working draft, cover/status;7.17.2.2 paragraphs2–6;
  7.17.3 paragraphs1–11 and7.17.3.1;7.17.4 through7.17.4.2;
  7.17.7.1 through7.17.7.5. The latter passage was read directly, including
  failure-order constraints and the unchanged-versus-mismatch expected cases.
* GCC primary `__atomic` builtin manual, memory-order introduction and
  consume implementation note; target fallback/runtime-order handling;
  load/store/exchange/CAS; old-versus-new fetch arithmetic; thread/signal fences
  and lock-free query sections. This is a compiler interface description,
  not the ISO C language definition. Actual installed compiler versions and
  generated code are recorded separately by the campaign.

Exact final C17 and published C23 text was not accessible in this work. N2176
is encrypted in the available cache and no final C23 text was obtained. These
are reference gaps, not inspected sources. No result renames N1570 C17 or
N3096 a published standard. T25.08.c and its delayed source-comparison packet
retain a partial status for that precise remaining obligation. The practical
baseline remains C17 and probes additionally use actual compiler C23 modes.

The research changed implementation decisions: the publication test distinguishes
initial reads from release reads; the reusable slot needs a reverse retirement
edge; the compound-state test uses atomic fields so the forced90 is defined;
SC schedule enumeration is limited to the all-SC fragment; weak CAS ownership
failure is separated from failed expected-value replacement. Fences and
consume lowering receive separate code/semantic evidence. Host aggregate
libatomic is declared and excluded from the strict target image.

Finite tests and source reads do not establish independent review or learner
mastery. The exact source versions, inspection bounds and generated hashes
appear in resources.json and research_gate.json. Private source extracts are
not published. Target instruction/link observations remain distinct from
target execution, physical ISR behavior and timing guarantees.
