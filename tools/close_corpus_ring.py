"""Bind consistent ring operations and delayed variants to actual evidence."""
import json
import sys
from common import ROOT, sha, dump
from check_corpus_ring import validate
sys.path.insert(0, str(ROOT))
from cmastery import exercises, learning

IDS = ['X-T76-010']


def main():
    folder = ROOT/'research/v5/expansion/corpus_ring'
    reports = ['reviews/v5/expansion/corpus_ring/'+p for p in
               ['current_validation.json', 'guard.json', 'export_validation.json']]
    campaign, guard, export = [json.loads((ROOT/p).read_text()) for p in reports]
    assert not validate(campaign)
    assert guard['passed'] and len(guard['negative_controls']) == 17
    assert guard['report_sha256'] == sha(ROOT/reports[0])
    assert export['passed'] and export['source_stable']
    assert len(export['executions']) == 11 and all(r['passed'] for r in export['executions'])
    assert len(export['exports']) == 1 and all(not r['private_sources_copied'] for r in export['exports'])
    for p, h in export['source_sha256'].items():
        assert sha(ROOT/p) == h, ('stale export', p)
    artifacts = ['labs/corpus_ring/CONTRACTS.md', 'labs/corpus_ring/ring.h',
                 'assessor/corpus_ring/ANALYSIS.md', 'assessor/corpus_ring/ring.c',
                 'assessor/corpus_ring/oracle.py', 'labs/corpus_ring/tests.c',
                 'research/v5/expansion/corpus_ring/validation.md']
    rows = []
    for ident in IDS:
        meta = exercises.exercise(ident)
        assert learning.digest(meta) == export['selected_catalog_sha256'][ident]
        scopes = {ident: 'All four exact rubric mechanisms and the implemented changed-condition variant.'}
        scopes.update({r['id']: r['criterion'] for r in meta['rubric']})
        scopes[ident+'-DELAYED'] = 'Implemented degraded distinct replicas and adjacent-epoch migration plans using actual prior hash/ring interfaces; capacity, stale view and equal-position changes, with unchanged failure outputs.'
        for key, scope in scopes.items():
            rows.append(dict(id=key, parent_exercise=ident, parent_project=meta['project'],
                dimensions={
                    'teaching': dict(state='complete-for-stated-contract', evidence=artifacts[:3]),
                    'implementation': dict(state='complete-including-delayed-reference', evidence=artifacts),
                    'author_validation': dict(state='passed-bounded-native-oracles-cost-and-object-link-audits', evidence=reports),
                    'external_review': dict(state='not-run', evidence=[]),
                    'independent_learner': dict(state='not-assessed', evidence=[]),
                    'physical_observation': dict(state='not-observed;cross-build-only', evidence=[])},
                closure_rationale=scope, artifacts=artifacts, remaining_author_work=[],
                minimum_days=7 if key.endswith('DELAYED') else None,
                evidence_limits=campaign['limitations']+[
                    'No concurrency, persistence, target execution or independent delayed learner claim.']))
    paths = set(campaign['inspected_paths']) | set(export['source_sha256']) | set(reports) | set(artifacts)
    paths |= {p.relative_to(ROOT).as_posix() for p in folder.iterdir()
              if p.is_file() and p.name in ['resources.json', 'taxonomy_extensions.json', 'research_gate.json', 'validation.md']}
    paths.add('tools/close_corpus_ring.py')
    result = dict(schema_version=1, scope='Required consistent ring refinement with separate external evidence.',
                  source_sha256={p: sha(ROOT/p) for p in sorted(paths)},
                  selected_catalog_sha256=export['selected_catalog_sha256'], current_reports=reports, updates=rows)
    dump(folder/'completion_dispositions.json', result)
    print(json.dumps(dict(passed=True, updates=len(rows), sha256=sha(folder/'completion_dispositions.json'))))


if __name__ == '__main__':
    main()
