# Independent communications and shared-tooling review

Accepted after fresh inspection and verification. Reviewer: `/root/alg_sim`, 2026-10-09.

The original 1,185 objectives, 207 projects and 194 fixtures pass preservation checks. The nine inserted module handoffs can be removed to recover exact previous lesson bytes after restoring their original CRLF line endings. Both communications modules and all 18 briefs were substantively inspected against their contract, source, profile, hardware and workload scopes.

## COMM-SHARED-01 — medium, resolved

Selected-prelude domains were omitted from validator cycle edges, and direct publication could consume stale order.

Paths: `tools/common.py`, `tools/validate.py`, `tools/write_program.py`, `tools/publish.py`, `tools/check_contract_guards.py`.

Fresh code uses common.domain_dependencies in both validator and route generator, and check_program_order in validate/publication. The same mutation now emits Prerequisite cycle at T02; invalid first T02 traversal is rejected. All 12 full-batch negative contract controls pass, including selected-prelude cycle and stale order.

## COMM-02 — medium, resolved

Full T28/T29/T31 prerequisites contradicted entry prose claiming selected T28 leaves avoid unrelated ADC/elective work; T29 also transitively requires full T28.

Paths: `curriculum/modules/T38.md`.

Fresh metadata has empty selected-prelude list and entry prose truthfully requires completed core T04/T18/T27/T28/T29/T31, reusing passed evidence; optional T28 work remains optional.

## COMM-03 — medium, resolved

T38.12.a and the project required FDF/BRS/ESI validation, but supplied cases/oracle omitted all three. An fdf:false property added to a successful case was ignored and still produced ok.

Paths: `curriculum/projects/PR-T38-10.md`, `fixtures/communications/PR-T38-10.json`, `tools/verify_communications_fixtures.py`.

Fresh contract/fixture/oracle require FDF=true, typed raw flags, nominal-only versus switched data and active/passive ESI. Valid no-BRS/passive examples and invalid FDF/flags are explicit. Independent fdf=false mutation now yields not_fd; all 10 T38 fixtures pass fresh recomputation.

## COMM-04 — medium, resolved

Python structural equality accepted boolean expected values as integers. Replacing expected.dlc_lengths[1]=1 with True still equaled recompute output.

Paths: `tools/verify_communications_fixtures.py`, `fixtures/communications/PR-T38-10.json`.

Fresh strict recursive comparison checks exact types and input guards distinguish booleans from bounded numbers. Independent expected DLC 1→True and input clock_hz→True probes are rejected; all 10 valid fixtures still pass.

## COMM-05 — low, resolved

Prose transformation split established IPv4/C13x spellings and two hexadecimal-address/fails boundaries.

Paths: `curriculum/modules/T39.md`, `curriculum/projects/PR-T39-04.md`, `curriculum/projects/PR-T39-05.md`, `curriculum/projects/PR-T39-06.md`, `curriculum/projects/PR-T39-07.md`, `curriculum/projects/PR-T39-08.md`.

Fresh canonical text restores IPv4/C13x and separates each address from fails. Author rechecked frozen metadata/oracles; no mechanism, source or budget change.

All four material findings and the small prose finding above are resolved after actual fresh inspection. All 18 supplied communications fixtures and 33 independent controls pass. The RX checks withhold publication/recycling while DMA owns storage; the TX checks reject an incomplete chain or reuse before completion. PHY identity, latch-low/current status, negotiated mode and retained-FCS policy are explicit. These static controls do not report completed learner C or wire observations.

The full route has each of 42 domains once, with both complete prerequisites and selected-prelude owners before dependents. Independent accounting gives 2,808 module hours + 160 new integration hours + 48 overhead = 3,016 hours. The 668-hour extension preserves the prior 2,348-hour catalog. T40/T41/T42 family tables add zero second budget. All 12 malformed-contract controls reject; isolated source-gate controls reject changed or unreviewed scope. Merged source limits/methods/dates and central contract download handling were checked freshly.

Scope also includes the changed nine base handoffs, shared generation/validation/acceptance tools, README and profiles/laboratory/reading/roadmap/workload/enhancement/communication-stacks/audit/assessment/integrations guides. Exact inspected artifact hashes and individual validation results are in the JSON receipt. Root still owns actual publication/browser/archive checks.
