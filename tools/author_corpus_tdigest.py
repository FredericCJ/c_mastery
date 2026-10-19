"""Write the additive t-digest brief and exact inspected primary registrations."""
import json
from common import ROOT, dump, sha

BASE = 'research/v5/expansion/corpus_tdigest'
IDENTIFIER = 'X-T15-009'
SOURCES = [
    ('paper', 'R-TDIGEST-2019', 'Computing Extremely Accurate Quantiles Using t-Digests',
     ['Ted Dunning', 'Otmar Ertl'], 'arXiv:1902.04023v1, 11 February 2019',
     'https://arxiv.org/pdf/1902.04023v1',
     ['Sections 2.1-2.3, PDF pages 4-5: centroid weight/mean, ordering and size invariant',
      'Algorithm 1, PDF page 9: sorted centroid/raw-point merging construction',
      'Section 2.8 equation 8, PDF page 12: normalized k2 scale',
      'Section 2.9, PDF pages 13-15: interpolation and singleton behavior'],
     'Original bounded normalized-k2 companion with its own explicitly specified fitted distribution. The paper title and empirical accuracy language are not adopted as a universal bound. No Java implementation or complete paper algorithm equivalence claim.'),
    ('invariant', 'R-TDIGEST-INVARIANT-2019', 'Conservation of the t-Digest Scale Invariant',
     ['Ted Dunning'], 'arXiv:1903.09919v1, 24 March 2019',
     'https://arxiv.org/pdf/1903.09919v1',
     ['Sections 1, 2.1 and 2.3, PDF pages 1-3: singleton-or-size invariant, added left/right weight and normalized k2 preservation'],
     'Real-arithmetic scale-invariant reasoning only; finite double rounding, the fitted CDF and quantile accuracy require separate analysis and observations.')
]


def main():
    resources = []
    for name, rid, title, authors, edition, url, locators, limits in SOURCES:
        capture = ROOT / f'build/corpus_tdigest_sources/{name}.pdf'
        resources.append(dict(id=rid, title=title, authors=authors, year=2019,
            edition=edition, kind='paper', url=url,
            access='Actual public versioned primary PDF downloaded and inspected',
            domains=['T15'], role='primary', assignable=True,
            reason='Required quantile-summary construction, merge and assumption/error investigation',
            prerequisites=['T14 probability and empirical distributions', 'T15 invariants and complexity'],
            locators=locators, inspection=dict(date='2026-10-18',
                method='Read the actual named versioned primary PDF passages, equations and algorithm.',
                scope='; '.join(locators), limits=limits,
                download_path=capture.relative_to(ROOT).as_posix(), download_sha256=sha(capture))))
    dump(ROOT / BASE / 'resources.json', dict(resources=resources))
    dump(ROOT / BASE / 'taxonomy_extensions.json', dict(schema_version=1,
        kind='required-competency-refinement-of-existing-leaf',
        source_records=[r['id'] for r in resources], objectives=['T15.15.a','T15.15.b','T15.15.c'],
        exercises=[IDENTIFIER],
        scope_rationale='The screened t-digest candidate explicitly requires approximate quantile behavior and merge-order/error limitations. Count-Min, HLL, MinHash and a generic probabilistic paragraph do not implement centroid quantile construction. This is an additive concrete refinement, preserving every existing T15.15 obligation.',
        preservation='No replacement of original leaves, criteria, hours, source identities or historical receipts.'))
    claims = [
        'Derive and implement bounded normalized-k2 centroid construction, checked staging and precisely defined singleton CDF/quantile behavior.',
        'Separate size-invariant, arithmetic, ordering and empirical-error claims; account for actual staging, sorting and validation cost.',
        'Use independent construction, mixture and exact raw-rank checks; implement and assess partition merging and coarser two-level recompression under changed orders.'
    ]
    rubric = []
    coverage = []
    for index, claim in enumerate(claims, 1):
        objective = 'T15.15.' + chr(ord('a') + index - 1)
        rid = IDENTIFIER + f'-R{index}'
        rubric.append(dict(id=rid, objective=objective, criterion=claim, levels={
            '0':'Missing mechanism or false deterministic/accuracy claim.',
            '1':'Selected examples work but a required boundary, premise or independent check is absent.',
            '2':claim+' D1-D3 establish the stated finite contract and preserve unfavorable observations.',
            '3':'Unaided D4 changes merge/compression conditions and defends the altered invariant, error and cost analysis.'}))
        coverage.append(dict(objective=objective, teaching=['#problem','#reasoning'],
            implementation=['D1','D2'], boundary=dict(applicable=True,evidence=['D3']),
            independent_assessment=[rid], assessed_claim=claim,
            scope='Required centroid quantile refinement; other parent algorithm mechanisms remain separately required.'))
    meta = dict(schema_version=1, id=IDENTIFIER,
        title='Merging t-digest, singleton quantiles and observed error limits',
        project='PR-T15-10', domain='T15', objectives=['T15.15.a','T15.15.b','T15.15.c'],
        families=list('ABCDEFGHIJ'), required=True,
        prerequisite_exercises=['X-T04-001','X-T15-002'], prerequisite_artifacts=[], profile='C',
        environment=dict(language='ISO C17, exact-width integers and double with at least 53 significand bits',
            target='Hosted Linux numerical experiments; no target runtime or physical claim',
            toolchain=['GCC','Clang','CMake','Python'],
            dependencies='Original allocation-free centroid, scale and interpolation mechanism. No third-party t-digest or math implementation. Hosted stdio/Python are verification support.'),
        source_records=[dict(id=r['id'],path=BASE+'/resources.json',passages=r['locators']) for r in resources],
        lab=dict(family='corpus_tdigest',scaffold='labs/corpus_tdigest',tests=['corpus_tdigest.contract']),
        assessor_analysis='assessor/corpus_tdigest/ANALYSIS.md',
        status=dict(content='authored',execution='source-bound campaign required',independent_review='not-run',learner_assessment='not-assessed',
            limitations=['The real-arithmetic k-size invariant is not a universal quantile-error bound.',
                        'Large finite error, merge-order variation and cross-arithmetic divergence must be retained.',
                        'Author execution supplies neither delayed learner evidence nor physical observation.'],
            campaign_receipt='reviews/v5/expansion/corpus_tdigest/current_validation.json'),
        fixtures=['fixtures/corpus_tdigest/cases.json'],shared_contracts=['labs/corpus_tdigest/CONTRACTS.md'],
        delayed_transfer=dict(minimum_days=7,variant_ids=[IDENTIFIER+'-DELAYED']),
        objective_coverage=coverage,rubric=rubric)
    body = '''# X-T15-009: Merging t-digest, singleton quantiles and observed error limits

## Prerequisites

Complete the named integer/algorithm exercises and review empirical distributions, weighted means and logarithms. Read the exact registered primary passages and CONTRACTS.md. Keep centroid ordering distinct from ordering of the raw observations it once represented. No private implementation is a learner prerequisite.

## Environment

Use the declared hosted C17 contract, without fast-math, external t-digest or math-library substitution for the studied mechanism. The original logarithm and fixed arrays have explicit finite domains. C23 supplies no automatic lifetime, numeric-error or statistical guarantee. Record the compiler/ABI and actual storage requirements before considering a target port.

## Problem

Implement td_init, td_valid, td_add, td_merge, td_quantile and td_cdf in src/tdigest.c. Maintain bounded positive-weight centroids, stable mean ordering, singleton extremes and normalized k2 size. Stage each change before publication and preserve all destination/output bytes on refusal. Implement the exact fitted distribution in CONTRACTS.md: singleton atoms, half-weight interpolation intervals and right-continuous CDF. Quantile is its generalized inverse, with explicit exact endpoints; it is not the type7 sample convention.

The ordinary route processes bounded batches and merges compatible digests. The separate required delayed route adds td_recompress and changes precision, partitioning, batch order and merge tree. Do not reconstruct missing raw observations or advertise merge as associative or idempotent.

## Necessity

Exact order statistics require retaining or externally organizing observations. A centroid summary saves bounded storage by losing information about within-cluster ranks and ranges. A size invariant controls grouping but does not prove a universal rank-error guarantee. Repeated values and sparse distant tails expose that distinction. A plausible smooth-data graph is insufficient evidence of a quality bound.

## Reasoning

Derive k-size from the normalized logit scale, including the singular endpoints and why a singleton is admitted separately. Explain the real-arithmetic preservation argument when weight is added to either side, and why reducing compression is permitted while increasing it cannot restore discarded data. Derive the weighted-mean recurrence and its bounded intermediates. Separate real truncation bounds for the logarithm series from floating rounding and the conservative admission/validation tolerances.

Derive total mass and generalized-inverse behavior for the specified fitted distribution, including repeated means and two singleton samples. Explain why fitted CDF agreement alone cannot establish accuracy against raw observations. Define a tie-aware empirical rank interval and a distinct value-error baseline. Account for fixed arrays, insertion sort, validation scans and scale evaluations. Give counterexamples to unconditional 1/delta rank accuracy and to equal answers across merge orders.

## Deliverables

- D1: Derivations, invariant and ownership tables, exact source locators, and at least two false explanations with concrete counterexamples.
- D2: Completed original C17 provider, build/dependency manifest, explicit refusal/publication policy and actual operation/storage accounting.
- D3: Independent construction, fitted-mixture and exact raw-data oracles, boundary snapshots, three compiling semantic controls and actual GCC/Clang receipts. Preserve the large-error and arithmetic/order disagreements.
- D4: Delayed recompression implementation and changed-condition experiments, with conservation, error and resource comparisons.

## Verification

Exercise empty and singleton state, duplicate values, endpoint queries, NaN/Infinity, output snapshots, compression mismatch, self-merge and total exhaustion. Independently select admissible prefixes for fresh batches; check every emitted centroid using higher precision, exact weight totals, endpoints and weighted sums. Compare the C traversal with a separately assembled atom/uniform mixture and retain exact sorted raw data for empirical error.

Run 570 bounded workload/order/compression/batch/tree cases, including 30 fresh construction probes. The reference fsum arithmetic need not make identical later groupings when tiny mean changes alter sorting; retain those disagreements as observations while checking each result independently. Capture a same-data, same-C merge-order witness and the sparse-tail rank-error case reaching 0.625. No universal accuracy claim follows from this campaign.

Configure labs/corpus_tdigest with CMake. Default src must compile and fail TD_CHECK. An explicit completed source is selected through TD_PROVIDER, or a private directory through CMASTERY_ASSESSOR_ROOT. The three compiled controls must fail the intended assertion rather than crash or fail compilation. Record inputs, source identities, compiler versions and raw outcomes.

## Rubric

Assess each JSON criterion independently. A false accuracy premise fails its reasoning criterion even if public tests pass. Level2 requires the stated bounded mechanism and independent evidence; level3 requires unaided changed-condition work and a defensible account of information loss. Retained author implementations do not establish learner mastery.

## Hints

1. First write the exact meaning of one centroid and what information it has lost.
2. Derive mass in the singleton and interval pieces before coding quantiles.
3. Keep scale validity, raw-data accuracy and arithmetic reproducibility in separate result columns.

## Variants

X-T15-009-DELAYED: at least seven days later, implement td_recompress and a two-level route using leaves at compression32 followed by a final compression8. Compare leaf8-to8, leaf32-to8 and leaf32-to32 across contiguous partitions, left/reverse folds and balanced trees, batches1/17/64, and ascending/descending/shuffled data. Preserve count and bounded k-size in every route; report both rank and value error. Include incompatible precision, repeated means, empty/self cases and unchanged-state refusal checks. Explain why leaf precision, merge order and retained raw-data baselines affect the interpretation. Do not label smaller observed error as a general improvement theorem.

## Security

Use only original local teaching providers and bounded synthetic inputs. Treat malformed state, extreme counts and adversarial value distributions as explicit cases. A minimized mutant failure and its repaired behavior demonstrate the selected invariant only. This mechanism is neither a cryptographic primitive nor a production quality/security certification. No external service is targeted.

## Validation

Source inspection, finite author execution, outside review, unaided learner work, elapsed delayed assessment and physical observation remain separate. Learner sources are intentionally incomplete. Only source-current guarded campaign and actual isolated export receipts support executed author claims.
'''
    (ROOT / f'curriculum/v5/exercises/{IDENTIFIER}.md').write_text(
        '```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf-8',newline='\n')
    dump(ROOT / BASE / 'screening_resolution.json', dict(schema_version=1,
        origin='research/v5/expansion/corpus_followthrough/screening_map.json', candidate='t-digest',
        original_disposition='specific-coverage-check-required', exercise=IDENTIFIER,
        mechanism='Actual original normalized-k2 centroid merge, singleton fitted distribution and two-level coarser recompression',
        source_register=BASE+'/resources.json', limitations='No universal accuracy guarantee or strong-order assumption; measured counterexamples are required.'))
    print('Authored one additive t-digest brief and two inspected primary registrations; no global writes')


if __name__ == '__main__':
    main()
