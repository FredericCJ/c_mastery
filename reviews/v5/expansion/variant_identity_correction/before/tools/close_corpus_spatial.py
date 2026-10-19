"""Bind each spatial criterion and changed condition to actual scoped evidence."""
import copy
import json
import sys
from common import ROOT, sha
sys.path.insert(0, str(ROOT))
from cmastery.exercises import parse_brief
from check_corpus_spatial import validate

HERE = ROOT / 'research/v5/expansion/corpus_spatial'
REPORTS = [f'reviews/v5/expansion/corpus_spatial/{name}.json' for name in
           ['current_validation', 'support_validation', 'export_validation', 'cost_validation']]
VARIANTS = {
    6: ['Actual three-dimensional weighted nearest distance with ties and duplicate coordinates.',
        'Full32 collinear cloud, staged updates and linear-oracle query comparison; no logarithmic update claim.'],
    7: ['Moving boxes refit all ancestors while preserving topology, then compare32 exact results and visits with rebuilding.',
        'Three-dimensional degenerate slabs and touching boxes; a concrete overlapping-box/disjoint-segment counterexample separates broad from narrow phase.'],
    8: ['Actual three-dimensional octree with midpoint ownership, unit duplicate buckets and all query boundaries.',
        'Deletion, reinsertion and exact node-budget refusal preserve the old tree; compare the same admitted points with the k-d implementation.'],
    9: ['Changed insertion/removal order exercises splits, underfill condensation, orphan leaf-ID reinsertion and root collapse.',
        'Three-dimensional zero-volume and overlapping boxes, exact capacity refusal and a declared departure from level-preserving internal-entry reinsertion.'],
    10: ['Actual three-dimensional negative cells and widths1/2/4, full-cell collision equality and unique exact pairs.',
         'Coarse width16 admits a large box, while width1 exhausts references transactionally; the previous query result remains available.'],
}

def main():
    campaign, guard, export, cost = [json.loads((ROOT/p).read_text(encoding='utf8')) for p in REPORTS]
    assert not validate(campaign)
    assert guard['passed'] and len(guard['negative_controls']) == 13
    assert guard['report_sha256'] == sha(ROOT/REPORTS[0])
    assert export['passed'] and export['source_stable'] and not export['private_sources_copied']
    assert len(export['executions']) == 55 and len(export['exports']) == 5
    assert cost['passed'] and cost['source_stable'] and len(cost['executions']) == 16
    assert len(cost['observations']) == 36
    for report in [export, cost]:
        assert all(r['passed'] for r in report['executions'])
        for path, digest in report['input_sha256'].items():
            assert sha(ROOT/path) == digest, ('stale report', path)
    artifacts = ['labs/corpus_spatial/CONTRACTS.md', 'labs/corpus_spatial/spatial.h',
                 'assessor/corpus_spatial/ANALYSIS.md', 'assessor/corpus_spatial/spatial.c',
                 'labs/corpus_spatial/tests.c', 'assessor/corpus_spatial/oracle.py']
    rows=[]; briefs=[]
    for number in range(6,11):
        ident=f'X-T14-{number:03}'; brief=ROOT/'curriculum/v5/exercises'/f'{ident}.md'
        meta,_=parse_brief(brief); briefs.append(brief.relative_to(ROOT).as_posix())
        scopes={ident: 'All four mechanism-specific criteria and both implemented changed conditions.'}
        scopes.update({r['id']:r['criterion'] for r in meta['rubric']})
        scopes.update({f'{ident}:V{i+1}':v for i,v in enumerate(VARIANTS[number])})
        for key, scope in scopes.items():
            rows.append(dict(id=key,parent_exercise=ident,parent_project=meta['project'],
                dimensions={
                    'teaching':dict(state='complete-for-declared-bounded-contract',evidence=artifacts[:3]),
                    'implementation':dict(state='complete-for-declared-reference-and-changed-conditions',evidence=artifacts),
                    'author_validation':dict(state='passed-source-bound-native-oracle-export-and-target-audit',evidence=REPORTS),
                    'external_review':dict(state='not-run',evidence=[]),
                    'independent_learner':dict(state='not-assessed',evidence=[]),
                    'physical_observation':dict(state='not-observed;integer-geometry-model-only',evidence=[])},
                artifacts=artifacts,closure_rationale=scope,
                author_validation=dict(report=REPORTS[0],checks=meta['lab']['tests'],export=REPORTS[2],costs=REPORTS[3]),
                minimum_days=7 if ':V' in key else None,remaining_author_work=[],
                evidence_limits=copy.deepcopy(campaign['limitations'])+[
                    '6976 independent linear geometric comparisons per native profile; finite coverage is not a proof of arbitrary mastery.',
                    'Four Cortex-M4 object/link and per-function frame audits, without target execution or whole-path stack measurement.',
                    'Node/reference visits are measured under equal contracts; no physical timing or universal speed claim.',
                    'Changed variants are author-supported, not independent delayed learner assessments.']))
    paths=set(campaign['inspected_paths'])|set(export['input_sha256'])|set(cost['input_sha256'])|set(REPORTS)|set(briefs)
    paths|={p.relative_to(ROOT).as_posix() for p in HERE.iterdir() if p.name in ['resources.json','taxonomy_extensions.json','research_gate.json','audit.json']}
    paths|={'tools/author_corpus_spatial.py','tools/close_corpus_spatial.py'}
    result=dict(schema_version=1,scope='Five distinct bounded spatial mechanisms and changed conditions; author evidence only.',
                source_sha256={p:sha(ROOT/p) for p in sorted(paths)},current_reports=REPORTS,updates=rows)
    out=HERE/'completion_dispositions.json';out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(dict(passed=True,updates=len(rows),path=out.relative_to(ROOT).as_posix(),sha256=sha(out))))

if __name__=='__main__':main()
