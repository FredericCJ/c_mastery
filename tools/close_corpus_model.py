"""Bind exact semantic-model root, three rubric rows and changed-condition task."""
import json,sys
from common import ROOT,sha,dump
from check_corpus_model import validate
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,validate as brief_valid,catalog
BASE='research/v5/expansion/corpus_model'
def main():
    reports=['reviews/v5/expansion/corpus_model/'+n for n in ['current_validation.json','guard.json','export_validation.json']]
    campaign,guard,export=[json.loads((ROOT/p).read_text()) for p in reports]
    assert not validate(campaign)
    assert guard['passed'] and len(guard['negative_controls'])==18
    assert guard['report_sha256']==sha(ROOT/reports[0])
    assert export['passed'] and export['source_stable'] and len(export['executions'])==14
    assert all(row['passed'] for row in export['executions'])
    for p,h in export['input_sha256'].items():assert sha(ROOT/p)==h,('stale export',p)
    for key,value in export['catalog_selected'].items():assert catalog()['exercises'][key]==value
    assert len(export['exports'])==1 and all(not r['private_sources_copied'] and r['unsolved_sources_unchanged'] for r in export['exports'])
    gate=dict(status='passage-inspected',resources=BASE+'/resources.json',
        research_input_sha256={BASE+'/'+n:sha(ROOT/BASE/n) for n in ['resources.json','taxonomy_extensions.json']},
        inspection=dict(date='2026-10-18',method='Read actual named primary online sections; retain exact HTML captures.',
            scope='Fowler brief semantic-model catalog and selected Single/TwoPass Builder sections; Scheme REPL section and index edition footer.',
            limits='No referenced book chapter, full Scheme implementation, production framework, independent human or physical claim.'))
    dump(ROOT/BASE/'research_gate.json',gate)
    brief='curriculum/v5/exercises/X-T15-021.md';meta,body=parse_brief(ROOT/brief);brief_valid(meta,body)
    artifacts=[brief,'labs/corpus_model/CONTRACTS.md','labs/corpus_model/model.h',
        'assessor/corpus_model/model.c','assessor/corpus_model/ANALYSIS.md','assessor/corpus_model/oracle.py',
        'labs/corpus_model/tests.c','labs/corpus_model/repl.c','labs/corpus_model/session_driver.c',
        'assessor/parsing_depth/pratt.c','labs/parsing_depth/parsing.h']
    scopes=[(meta['id'],'Original persistent stable-ID signal model, two configuration frontends/direct API, actual frozen Pratt queries and persistent read/eval/print; exact bounded criteria below.')]
    scopes += [(r['id'],r['criterion']) for r in meta['rubric']]
    scopes += [(meta['id']+'-DELAYED','Implemented chunked feed/EOF route and prefix-retaining writer; actual484 stream cases/profile include79 failed-output prefixes, changing write/chunk boundaries, pending-admission rejection, oversized recovery and missing final newline. Revision count and exact independent transcript prevent duplicate effects; five actual stdio processes are separately executed.')]
    updates=[]
    for key,scope in scopes:
        dimensions={name:dict(state=state,evidence=evidence) for name,state,evidence in [
          ('teaching','complete-for-stated-contract',artifacts[:3]),
          ('implementation','complete-including-delayed-reference',artifacts),
          ('author_validation','passed-six-host-profiles-and-independent-finite-oracles',reports),
          ('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-observed',[])]}
        updates.append(dict(id=key,parent_exercise=meta['id'],parent_project=meta['project'],dimensions=dimensions,
            closure_rationale=scope,artifacts=artifacts,remaining_author_work=[],minimum_days=7 if key.endswith('DELAYED') else None,
            evidence_limits=['Finite sixteen-slot model and original languages; not full Scheme, XML, Fowler application or production DSL equivalence.',
              'Actual accepted Pratt provider used; original parser/tokenizer criteria remain separately assessed.',
              'Live serialized session and truthful nonreentrant prefix writer premises; no durable exactly-once output or concurrent access guarantee.',
              'Bounded author-team source review only; no external human, independent delayed learner or physical observation.']))
    paths=set(campaign['inspected_paths'])|set(export['input_sha256'])|set(reports)|set(artifacts)
    paths|={BASE+'/'+n for n in ['research_gate.json','validation.md','screening_resolution.json']}
    paths|={'tools/close_corpus_model.py','tools/check_expansion_campaigns.py'}
    dump(ROOT/BASE/'completion_dispositions.json',dict(schema_version=1,scope='Exact semantic-model root, three rubric criteria and implemented changed-condition reference.',
        source_sha256={p:sha(ROOT/p) for p in sorted(paths)},current_reports=reports,updates=updates))
    print(json.dumps(dict(passed=True,updates=len(updates),sha256=sha(ROOT/BASE/'completion_dispositions.json'))))
if __name__=='__main__':main()
