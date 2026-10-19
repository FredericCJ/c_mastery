"""Bind the exact scanner criteria and implemented delayed memoization variant."""
import json,sys
from common import ROOT,sha,dump
from check_corpus_munch import validate
from corpus_export_bindings import errors as export_errors
sys.path.insert(0,str(ROOT))
from cmastery import exercises

def main():
    folder=ROOT/'research/v5/expansion/corpus_munch'
    reports=['reviews/v5/expansion/corpus_munch/'+x for x in ['current_validation.json','guard.json','export_validation.json']]
    campaign,guard,export=[json.loads((ROOT/p).read_text()) for p in reports]
    assert not validate(campaign)
    assert guard['passed'] and len(guard['negative_controls'])==16 and guard['report_sha256']==sha(ROOT/reports[0])
    assert export['passed'] and len(export['commands'])==12 and all(c['passed'] for c in export['commands'])
    assert not export_errors(export,['X-T15-020','X-T15-010'])
    assert len(export['exports'])==1 and export['exports'][0]['assessor_files_absent']
    for p,h in export['source_sha256'].items():assert sha(ROOT/p)==h,('stale export',p)
    artifacts=['labs/corpus_munch/CONTRACTS.md','labs/corpus_munch/munch.h','assessor/corpus_munch/ANALYSIS.md','assessor/corpus_munch/munch.c','assessor/corpus_munch/oracle.py','labs/corpus_munch/tests.c','research/v5/expansion/corpus_munch/validation.md']
    meta=exercises.exercise('X-T15-020');scopes={meta['id']:'Exact bounded longest-prefix scanner, failed-configuration memoization and actual prior regex-DFA composition.'}
    scopes.update({r['id']:r['criterion'] for r in meta['rubric']})
    scopes[meta['id']+'-DELAYED']='Implemented memoized variant, identical-token comparison, changed input/acceptance cache invalidation, real prerequisite adapter and equal-contract counter/cost comparison; independent delayed learner attempt pending.'
    rows=[]
    for ident,scope in scopes.items():
        rows.append(dict(id=ident,parent_exercise=meta['id'],parent_project=meta['project'],dimensions={
            'teaching':dict(state='complete-for-stated-contract',evidence=artifacts[:3]),
            'implementation':dict(state='complete-including-delayed-reference',evidence=artifacts),
            'author_validation':dict(state='passed-bounded-native-oracle-cost-and-core-link-audits',evidence=reports),
            'external_review':dict(state='not-run',evidence=[]),
            'independent_learner':dict(state='not-assessed',evidence=[]),
            'physical_observation':dict(state='not-observed;core-cross-build-only',evidence=[])},closure_rationale=scope,artifacts=artifacts,remaining_author_work=[],minimum_days=7 if ident.endswith('DELAYED') else None,evidence_limits=campaign['limitations']+['No unbounded lexicon/input-refill mechanism, target pipeline execution, thread-safety or independent delayed learner claim.']))
    paths=set(campaign['inspected_paths'])|set(export['source_sha256'])|set(reports)|set(artifacts)
    paths|={p.relative_to(ROOT).as_posix() for p in folder.iterdir() if p.name in ['resources.json','taxonomy_extensions.json','research_gate.json','validation.md']}
    paths.update(['tools/close_corpus_munch.py','tools/author_corpus_munch.py'])
    dump(folder/'completion_dispositions.json',dict(schema_version=1,scope='Required longest-prefix/memoization refinement with separate external dimensions.',source_sha256={p:sha(ROOT/p) for p in sorted(paths)},current_reports=reports,selected_catalog_sha256=export['selected_catalog_sha256'],updates=rows))
    print(json.dumps(dict(passed=True,updates=len(rows),sha256=sha(folder/'completion_dispositions.json'))))
if __name__=='__main__':main()
