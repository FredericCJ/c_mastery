"""Bind each exact sketch root/rubric/transfer to current separate author evidence."""
import json,sys
from common import ROOT,sha,dump
from check_corpus_sketch import validate
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,validate as brief_valid,catalog
BASE='research/v5/expansion/corpus_sketch'
def main():
    reports=['reviews/v5/expansion/corpus_sketch/'+name for name in ['current_validation.json','guard.json','export_validation.json']]
    campaign,guard,export=[json.loads((ROOT/p).read_text()) for p in reports]
    assert not validate(campaign);assert guard['passed'] and len(guard['negative_controls'])==17 and guard['report_sha256']==sha(ROOT/reports[0])
    assert export['passed'] and export['source_stable'] and len(export['executions'])==33 and all(r['passed'] for r in export['executions'])
    for p,h in export['input_sha256'].items():assert sha(ROOT/p)==h,('stale export',p)
    for k,v in export['catalog_selected'].items():assert catalog()['exercises'][k]==v,('stale selected exercise',k)
    assert len(export['exports'])==3 and all(not e['private_sources_copied'] and e['unsolved_sources_unchanged'] for e in export['exports'])
    gate=dict(status='passage-inspected',resources=BASE+'/resources.json',research_input_sha256={BASE+'/'+p:sha(ROOT/BASE/p) for p in ['resources.json','taxonomy_extensions.json']},inspection=dict(date='2026-10-18',method='Read actual named primary PDF passages and visually inspect HLL practical algorithm and MinHash theorem formulas.',scope='Exact source register locators and original finite refinements',limits='No whole-paper proof reproduction, production hash-quality evidence or independent human inspection inferred.'))
    dump(ROOT/BASE/'research_gate.json',gate)
    rows=[];paths=set(campaign['inspected_paths'])|set(export['input_sha256'])|set(reports)|{BASE+'/research_gate.json',BASE+'/validation.md','tools/close_corpus_sketch.py'}
    for n in (5,6,7):
        identifier=f'X-T15-{n:03}';brief=f'curriculum/v5/exercises/{identifier}.md';meta,body=parse_brief(ROOT/brief);brief_valid(meta,body)
        mechanism=meta['lab']['tests'][0].split('.')[-1]
        artifacts=[brief,'labs/corpus_sketch/CONTRACTS.md','labs/corpus_sketch/sketch.h','assessor/corpus_sketch/'+mechanism+'.c','assessor/corpus_sketch/ANALYSIS.md','assessor/corpus_sketch/oracle.py','labs/corpus_sketch/tests.c']
        scopes=[(identifier,'Exact '+mechanism+' ordinary mechanism, all three rubric claims and separately named transfer reference.')]+[(r['id'],r['criterion']) for r in meta['rubric']]+[(identifier+'-DELAYED',{'count_min':'Actual checked componentwise partition merge, equal complete active hash configuration, direct weighted-stream oracle and unchanged-on-refusal tests.','hyperloglog':'Actual precision folding with extra index bits reinterpreted as suffix prefix, every supported precision pair compared against direct coarse bitstring ranks.','minhash':'Actual all-row band matching, independent explicit-set oracle and concrete false-positive and false-negative candidate examples.'}[mechanism])]
        for key,scope in scopes:
            rows.append(dict(id=key,parent_exercise=identifier,parent_project=meta['project'],dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete-for-stated-contract',artifacts[:3]),('implementation','complete-including-delayed-reference',artifacts),('author_validation','passed-six-host-profiles-and-independent-finite-oracles',reports),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-observed',[])]},closure_rationale=scope,artifacts=artifacts,remaining_author_work=[],minimum_days=7 if key.endswith('DELAYED') else None,evidence_limits=['Exact finite domains and hosted C17 executions only.','Probability statements depend on stated uniformity/independence/nonadaptive premises, not deterministic test success.','No elapsed delayed learner attempt, independent human review or physical observation.']))
        paths|=set(artifacts)
    dump(ROOT/BASE/'completion_dispositions.json',dict(schema_version=1,scope='Three exact new required sketch mechanisms, nine rubric criteria and three implemented changed-condition references.',source_sha256={p:sha(ROOT/p) for p in sorted(paths)},current_reports=reports,updates=rows))
    print(json.dumps(dict(passed=True,updates=len(rows),sha256=sha(ROOT/BASE/'completion_dispositions.json'))))
if __name__=='__main__':main()
