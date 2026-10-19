"""Bind exact codec root, rubric and delayed obligations to actual author runs."""
import json
import sys
from common import ROOT,sha,dump
from check_corpus_codec import validate
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief
from cmastery import exercises,learning

def main():
    folder=ROOT/'research/v5/expansion/corpus_codec'
    reports=['reviews/v5/expansion/corpus_codec/'+p for p in ['current_validation.json','guard.json','export_validation.json']]
    campaign,guard,export=[json.loads((ROOT/p).read_text()) for p in reports]
    assert not validate(campaign)
    assert guard['passed'] and len(guard['negative_controls'])==15 and guard['report_sha256']==sha(ROOT/reports[0])
    assert export['passed'] and export['source_stable'] and not export['private_sources_copied']
    assert len(export['executions'])==11 and all(r['passed'] for r in export['executions'])
    assert export['selected_catalog_sha256']['X-T15-004']==learning.digest(exercises.exercise('X-T15-004'))
    for p,h in export['input_sha256'].items():assert sha(ROOT/p)==h,('stale export',p)
    meta,_=parse_brief(ROOT/'curriculum/v5/exercises/X-T15-004.md')
    scopes={meta['id']:'All three exact rubric mechanisms and implemented delayed optimal parser.'}
    scopes.update({r['id']:r['criterion'] for r in meta['rubric']})
    scopes['X-T15-004-DELAYED']='Actual backward optimal-token DP and composition, independently checked by forward shortest paths; depth-ceiling interview has separate worked reasoning.'
    artifacts=['labs/corpus_codec/CONTRACTS.md','labs/corpus_codec/codec.h','assessor/corpus_codec/ANALYSIS.md','assessor/corpus_codec/codec.c','assessor/corpus_codec/oracle.py','labs/corpus_codec/tests.c','research/v5/expansion/corpus_codec/validation.md']
    rows=[]
    for ident,scope in scopes.items():
        rows.append(dict(id=ident,parent_exercise=meta['id'],parent_project=meta['project'],dimensions={
            'teaching':dict(state='complete-for-stated-contract',evidence=artifacts[:3]),
            'implementation':dict(state='complete-including-delayed-reference',evidence=artifacts),
            'author_validation':dict(state='passed-bounded-native-oracle-cost-and-strict-object-link-audits',evidence=reports),
            'external_review':dict(state='not-run',evidence=[]),
            'independent_learner':dict(state='not-assessed',evidence=[]),
            'physical_observation':dict(state='not-observed;cross-build-only',evidence=[])},
            closure_rationale=scope,artifacts=artifacts,remaining_author_work=[],minimum_days=7 if ident.endswith('DELAYED') else None,
            evidence_limits=campaign['limitations']+['No DEFLATE interoperability, target execution or independent delayed learner claim.']))
    paths=set(campaign['inspected_paths'])|set(export['input_sha256'])|set(reports)|set(artifacts)
    paths|={p.relative_to(ROOT).as_posix() for p in folder.iterdir() if p.is_file() and p.name in ['resources.json','taxonomy_extensions.json','research_gate.json','validation.md']}
    paths.add('tools/close_corpus_codec.py')
    result=dict(schema_version=1,scope='Required compression refinement; separate author and external evidence.',source_sha256={p:sha(ROOT/p) for p in sorted(paths)},current_reports=reports,updates=rows)
    dump(folder/'completion_dispositions.json',result)
    print(json.dumps(dict(passed=True,updates=len(rows),sha256=sha(folder/'completion_dispositions.json'))))
if __name__=='__main__':main()
