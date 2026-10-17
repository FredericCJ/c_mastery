# T67/T68 author validation

This batch authors 20 projects, 80 leaf objectives, 40 units, 20 full exercise
briefs, 40 glossary entries and 60 recall variants. Its 20 public C anchors have
40 compiled semantic mutants. Source inspection is the author's bounded reading
of the registered primary passages, not an independent review. The published
C17 and final C23 source gaps remain open.

Exact completed commands, run from the repository under Ubuntu WSL:

```text
python3 tools/run_expansion_family.py adt --report reviews/v5/expansion/b07/adt_campaign.json
python3 tools/run_expansion_family.py metaprog --report reviews/v5/expansion/b07/metaprog_campaign.json
python3 tools/run_ad_mp_experiments.py
```

Each family passed GCC debug/release/ASan+UBSan and Clang
debug/release/ASan+UBSan: 30 CTests per profile, comprising ten positive anchors
and twenty expected-rejection mutation controls. Each default learner build
compiled and its ten named anchors failed with the declared semantic assertion
marker. Each campaign records 43 exact subprocess commands and their complete
logs, compiler/build inputs and source hashes. Current source pins were checked
after the final run and none were stale.

The independent ADT oracle compared 67,954 actual driver results per profile:
all one- and two-byte UTF-8 strings plus empty and named longer boundaries,
small strided coordinate/capacity combinations, and packet tag/length cases.
The metaprogramming oracle compared 2,092 actual driver results per profile:
semantic-key hashes, decimal formatting capacities and dependency-derived build
keys. Both also recomputed ten concrete fixture expectations. These are bounded
independent implementations, not proofs of all representation or lifetime claims.

The separate experiment receipt records 124 successful checks. Forty deliberately
faulty implementations were each compiled with strict C17 warnings and observed
to fail their intended named assertion; an unrelated crash or compiler error
was not accepted as a mutation kill. GCC and Clang diagnostic probes checked
missing enum cases, incompatible wrapper/handle categories, constant capacity,
unsupported generic arguments and const writes. Both accepted the separate
C2x `__VA_OPT__` comparison probe. That observation does not establish final
C23 wording. Generated C tables were compiled and their exact 64-bit values
executed. Object sizes, undefined symbols and compiler stack-use reports were
recorded for freestanding compilation of the mechanism source; this does not
claim a complete linked target runtime or a physical measurement.

The canonical exercise validator was invoked directly for all twenty briefs,
with their real assets, source passage locators, family tests, objective facets
and rubrics. All passed, and their exercise prerequisite graph was acyclic.
Whole-edition integration and publication are separate root-owned checks.

The depth audit maps all 80 leaves through source, teaching, exercise,
learner deliverable and criterion. `ad_mp_gap_ledger.json` retains 46 required
open records: twenty delayed attempts, twenty further written investigations,
four unavailable-reference records and two unfinished measurement campaigns.
Actual learner prerequisite-component composition is explicitly included among
the remaining investigations. No learner mastery, outside reviewer independence,
production applicability or comprehensive curriculum completion is inferred.
