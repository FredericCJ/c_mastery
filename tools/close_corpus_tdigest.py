"""Bind the exact t-digest root, three rubric rows and changed-condition task."""
import json
import sys
from common import ROOT, sha, dump
from check_corpus_tdigest import validate
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief, validate as brief_valid, catalog

BASE = 'research/v5/expansion/corpus_tdigest'


def main():
    reports = ['reviews/v5/expansion/corpus_tdigest/'+name for name in
               ['current_validation.json','guard.json','export_validation.json']]
    campaign,guard,export = [json.loads((ROOT/path).read_text()) for path in reports]
    assert not validate(campaign)
    assert guard['passed'] and len(guard['negative_controls']) == 15
    assert guard['report_sha256'] == sha(ROOT/reports[0])
    assert export['passed'] and export['source_stable']
    assert len(export['executions']) == 11 and all(row['passed'] for row in export['executions'])
    for path,digest in export['input_sha256'].items():
        assert sha(ROOT/path) == digest,('stale export',path)
    for key,value in export['catalog_selected'].items():
        assert catalog()['exercises'][key] == value,('stale selected exercise',key)
    assert len(export['exports']) == 1
    assert all(not row['private_sources_copied'] and row['unsolved_sources_unchanged']
               for row in export['exports'])
    gate = dict(status='passage-inspected',resources=BASE+'/resources.json',
        research_input_sha256={BASE+'/'+name:sha(ROOT/BASE/name)
                               for name in ['resources.json','taxonomy_extensions.json']},
        inspection=dict(date='2026-10-18',
            method='Read the actual named versioned primary PDF passages, equations and algorithm.',
            scope='Exact source-register locators for centroid construction, normalized k2 and its real-arithmetic preservation.',
            limits='No universal quantile bound, full reference implementation equivalence or independent human review inferred.'))
    dump(ROOT/BASE/'research_gate.json',gate)
    brief = 'curriculum/v5/exercises/X-T15-009.md'
    meta,body = parse_brief(ROOT/brief)
    brief_valid(meta,body)
    artifacts = [brief,'labs/corpus_tdigest/CONTRACTS.md','labs/corpus_tdigest/tdigest.h',
                 'assessor/corpus_tdigest/tdigest.c','assessor/corpus_tdigest/ANALYSIS.md',
                 'assessor/corpus_tdigest/oracle.py','labs/corpus_tdigest/tests.c']
    scopes = [('X-T15-009','Original bounded normalized-k2 merge, explicit singleton fitted distribution and all three criterion-specific derivations/checks.')]
    scopes += [(row['id'],row['criterion']) for row in meta['rubric']]
    scopes += [('X-T15-009-DELAYED','Implemented coarser recompression and actual leaf8-to8, leaf32-to8 and leaf32-to32 partition routes across three batch sizes, three input orders and left/reverse/balanced merge trees. Count/scale/mixture/error checks retain the concrete merge-order counterexample.')]
    rows = []
    for key,scope in scopes:
        dimensions = {name:dict(state=state,evidence=evidence) for name,state,evidence in [
            ('teaching','complete-for-stated-contract',artifacts[:3]),
            ('implementation','complete-including-delayed-reference',artifacts),
            ('author_validation','passed-six-host-profiles-and-independent-finite-oracles',reports),
            ('external_review','not-run',[]),('independent_learner','not-assessed',[]),
            ('physical_observation','not-observed',[])]}
        rows.append(dict(id=key,parent_exercise=meta['id'],parent_project=meta['project'],
            dimensions=dimensions,closure_rationale=scope,artifacts=artifacts,
            remaining_author_work=[],minimum_days=7 if key.endswith('DELAYED') else None,
            evidence_limits=['570 bounded histories/probes per hosted profile, including 30 independently constructed fresh batches.',
                'Observed rank error 0.625 and same-C merge-order variation are preserved; no universal accuracy or associativity claim.',
                'Cross-arithmetic history divergences are retained; independent constraints and mixture checks do not establish bitwise equality to every model.',
                'No elapsed delayed learner attempt, independent human review or physical observation.']))
    paths = set(campaign['inspected_paths']) | set(export['input_sha256']) | set(reports) | set(artifacts)
    paths |= {BASE+'/'+name for name in ['research_gate.json','validation.md','screening_resolution.json']}
    paths.add('tools/close_corpus_tdigest.py')
    dump(ROOT/BASE/'completion_dispositions.json',dict(schema_version=1,
        scope='Exact t-digest root, three rubric criteria and implemented changed-condition reference.',
        source_sha256={path:sha(ROOT/path) for path in sorted(paths)},current_reports=reports,updates=rows))
    print(json.dumps(dict(passed=True,updates=len(rows),sha256=sha(ROOT/BASE/'completion_dispositions.json'))))


if __name__ == '__main__':
    main()
