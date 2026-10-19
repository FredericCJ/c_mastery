"""Bind exact streaming-event criteria and changed conditions."""
import json
import sys
from common import ROOT,sha
from check_corpus_events import validate
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief
BASE='research/v5/expansion/corpus_events'
REPORTS=['reviews/v5/expansion/corpus_events/'+n+'.json' for n in
         ['current_validation','support_validation','export_validation','cost_validation']]
def main():
    campaign,guard,export,cost=[json.loads((ROOT/p).read_text()) for p in REPORTS]
    assert not validate(campaign)
    assert guard['passed'] and len(guard['negative_controls'])==14 and guard['report_sha256']==sha(ROOT/REPORTS[0])
    assert export['passed'] and export['source_stable'] and not export['private_sources_copied']
    assert len(export['exports'])==1 and len(export['executions'])==11 and all(r['passed'] for r in export['executions'])
    assert all(sha(ROOT/p)==h for p,h in export['input_sha256'].items())
    assert cost['passed'] and cost['source_stable'] and len(cost['executions'])==4
    assert len(cost['observations'])==2 and all(r['passed'] for r in cost['executions'])
    assert all(sha(ROOT/p)==h for p,h in cost['input_sha256'].items())
    brief='curriculum/v5/exercises/X-T15-019.md';meta,_=parse_brief(ROOT/brief)
    code=['assessor/corpus_events/events.c','labs/corpus_events/events.h','labs/corpus_events/tests.c','assessor/corpus_events/oracle.py']
    teaching=[brief,'labs/corpus_events/CONTRACTS.md','assessor/corpus_events/ANALYSIS.md']
    claims={meta['id']:'All four exact criteria and both required implemented changed conditions, limited to declared storage, serialization and callback premises.'}
    claims.update({r['id']:r['criterion'] for r in meta['rubric']})
    claims[meta['id']+'-V1']='Actual text capacities16 and1,1024 independent Expat documents per capacity/profile, changed feed and callback schedules, and eight strict target object/link audits.'
    claims[meta['id']+'-V2']='Actual copied two-event queue and separate checked decimal consumer, deferred drains, WAIT retries, overwritten consumed input, pending cancel, malformed/overflowing field refusals and complete-document publication authority.'
    rows=[]
    for ident,claim in claims.items():
        rows.append(dict(id=ident,parent_exercise=meta['id'],parent_project=meta['project'],dimensions=dict(
            teaching=dict(state='complete-for-declared-contract',evidence=teaching),implementation=dict(state='complete-nested-events-and-changed-conditions',evidence=code),author_validation=dict(state='passed-source-bound-host-oracle-export-target-audit',evidence=REPORTS),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-run;target-link-only',evidence=[])),closure_rationale=claim,artifacts=teaching+code,remaining_author_work=[],evidence_limits=[
            'Configured depth/name/text limits are explicit; finite documents do not prove every possible stream.',
            'Original bounded grammar is XML-derived but not XML/SAX conformance; raw ]]>, attributes, Unicode and entity rules differ explicitly.',
            'Exact published C17/C23 remain uninspected; actual W3C and Oracle SAX primary clauses are the evidence, not inaccessible direct SAX host metadata.',
            'Target sections and emitted frames are actual; physical execution/latency and whole-system stack are not claimed. Direct total-overflow injection is not a reachable transcript under the current uint32 event bound.',
            'Reference execution is not independent delayed learner assessment or human-independent review.']))
    paths=set(campaign['inspected_paths'])|set(export['input_sha256'])|set(cost['input_sha256'])|set(REPORTS)|set(teaching+code)
    paths|={BASE+'/'+p for p in ['resources.json','taxonomy_extensions.json','research_gate.json','audit.json']}
    paths|={'tools/author_corpus_events.py','tools/close_corpus_events.py'}
    out=ROOT/BASE/'completion_dispositions.json'
    result=dict(schema_version=1,scope='Exact original nested streaming events and consumer author support only.',source_sha256={p:sha(ROOT/p) for p in sorted(paths)},current_reports=REPORTS,updates=rows)
    out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(dict(passed=True,updates=len(rows),path=out.relative_to(ROOT).as_posix(),sha256=sha(out))))
if __name__=='__main__':main()
