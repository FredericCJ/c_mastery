"""Bind each scene criterion and changed condition to actual scoped evidence."""
import copy
import json
import sys
from common import ROOT, sha
sys.path.insert(0, str(ROOT))
from cmastery.exercises import parse_brief
from check_corpus_scene import validate

HERE = ROOT / 'research/v5/expansion/corpus_scene'
REPORTS = [f'reviews/v5/expansion/corpus_scene/{name}.json' for name in
           ['current_validation', 'support_validation', 'export_validation', 'cost_validation']]
VARIANTS = {
    1: ['Actual3D reflection/shear and keep-local structural reparent, compared with uncached homogeneous matrix products.',
        'Keep-world under a signed-permutation parent, scaled/singular refusal, complete range rollback and finite slot retirement.'],
    2: ['160 bent convex channels under rotation/reflection/scaling and both traversal directions:1280 independent visibility shortest-path comparisons per native profile.',
        'Typed scratch overwrite, copied session ownership, cancel/new-generation/stale-ticket rejection, both capacity budgets and MAX-generation cleanup.'],
}

def main():
    campaign, guard, export, cost = [json.loads((ROOT/p).read_text(encoding='utf8')) for p in REPORTS]
    assert not validate(campaign)
    assert guard['passed'] and len(guard['negative_controls']) == 13
    assert guard['report_sha256'] == sha(ROOT/REPORTS[0])
    assert export['passed'] and export['source_stable'] and not export['private_sources_copied']
    assert len(export['executions']) == 22 and len(export['exports']) == 2
    assert cost['passed'] and cost['source_stable'] and len(cost['executions']) == 6
    assert len(cost['observations']) == 4
    for report in [export, cost]:
        assert all(r['passed'] for r in report['executions'])
        for path, digest in report['input_sha256'].items():
            assert sha(ROOT/path) == digest, ('stale report', path)
    artifacts = ['labs/corpus_scene/CONTRACTS.md', 'labs/corpus_scene/scene.h',
                 'assessor/corpus_scene/ANALYSIS.md', 'assessor/corpus_scene/scene.c',
                 'labs/corpus_scene/tests.c', 'assessor/corpus_scene/oracle.py']
    rows=[]; briefs=[]
    for number,ident in [(1,'X-T14-011'),(2,'X-T16-004')]:
        brief=ROOT/'curriculum/v5/exercises'/f'{ident}.md'
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
                    '4096 independent uncached matrix products and1280 visibility shortest paths per native profile; finite coverage is not a proof of arbitrary mastery.',
                    'Four Cortex-M4 object/link and per-function frame audits, without target execution or whole-path stack measurement.',
                    'Matrix work and actual typed region sizes are recorded under equal contracts; no physical timing or universal speed claim.',
                    'Changed variants are author-supported, not independent delayed learner assessments.']))
    paths=set(campaign['inspected_paths'])|set(export['input_sha256'])|set(cost['input_sha256'])|set(REPORTS)|set(briefs)
    paths|={p.relative_to(ROOT).as_posix() for p in HERE.iterdir() if p.name in ['resources.json','taxonomy_extensions.json','research_gate.json','audit.json']}
    paths|={'tools/author_corpus_scene.py','tools/close_corpus_scene.py'}
    result=dict(schema_version=1,scope='Two distinct bounded scene/navigation mechanisms and changed conditions; author evidence only.',
                source_sha256={p:sha(ROOT/p) for p in sorted(paths)},current_reports=REPORTS,updates=rows)
    out=HERE/'completion_dispositions.json';out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(dict(passed=True,updates=len(rows),path=out.relative_to(ROOT).as_posix(),sha256=sha(out))))

if __name__=='__main__':main()
