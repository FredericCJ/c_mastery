"""Exact Treiber-stack readiness, preserving external assessment boundaries."""
import json,sys
from common import ROOT,sha
from check_corpus_treiber import validate
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief
BASE='research/v5/expansion/corpus_treiber'
REPORTS=['reviews/v5/expansion/corpus_treiber/'+n+'.json' for n in ['current_validation','support_validation','export_validation','cost_validation']]
def main():
 campaign,guard,export,cost=[json.loads((ROOT/p).read_text()) for p in REPORTS]
 assert not validate(campaign)
 assert guard['passed'] and len(guard['negative_controls'])==19 and guard['report_sha256']==sha(ROOT/REPORTS[0])
 assert export['passed'] and export['source_stable'] and not export['private_sources_copied']
 assert len(export['exports'])==1 and len(export['executions'])==11 and all(r['passed'] for r in export['executions'])
 assert all(sha(ROOT/p)==h for p,h in export['input_sha256'].items())
 assert cost['passed'] and cost['source_stable'] and len(cost['executions'])==4 and all(r['passed'] for r in cost['executions'])
 assert all(sha(ROOT/p)==h for p,h in cost['input_sha256'].items())
 brief='curriculum/v5/exercises/X-T55-004.md';meta,_=parse_brief(ROOT/brief)
 code=['assessor/corpus_treiber/treiber.c','labs/corpus_treiber/treiber.h','labs/corpus_treiber/tests.c','labs/corpus_treiber/model.c','assessor/corpus_treiber/oracle.py']
 teaching=[brief,'labs/corpus_treiber/CONTRACTS.md','assessor/corpus_treiber/ANALYSIS.md']
 claims={meta['id']:'Actual Treiber push/pop and exact protected reclamation, all four rubric criteria and both changed conditions under the declared finite/caller premises.'}
 claims.update({r['id']:r['criterion'] for r in meta['rubric']})
 claims[meta['id']+'-V1']='Executed one-slot stalled reader, competing pop, cancelled hazard and delayed collector generation mismatch; generation3 admits exactly four incarnations then permanently refuses reuse.'
 claims[meta['id']+'-V2']='Executed four native participants,256 independent real-time histories/profile and actual-code RC11356/30 finite executions; actual relaxed-publication and no-revalidation data-race counterexamples retained.'
 rows=[]
 for ident,claim in claims.items():
  rows.append(dict(id=ident,parent_exercise=meta['id'],parent_project=meta['project'],dimensions=dict(teaching=dict(state='complete-for-declared-contract',evidence=teaching),implementation=dict(state='complete-actual-stack-protection-and-finite-reuse',evidence=code),author_validation=dict(state='passed-source-bound-native-model-oracle-export-target-audit',evidence=REPORTS),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-run;target-object-link-only',evidence=[])),closure_rationale=claim,artifacts=teaching+code,remaining_author_work=[],evidence_limits=['Fixed4participants/8slots and no live-node reinsertion; static addresses do not themselves prevent logical ABA or races on reused payload.','Six C17 profiles plus GCC/Clang ThreadSanitizer and two finite RC11 models do not prove arbitrary histories, wait-free progress, scheduling fairness or learner mastery.','Strict M4 images have actual atomic instruction/helper closure; no target execution, silicon latency or exclusive-monitor progress claim.','Original IBM1986 and exact published C17/C23 remain source gaps; Rochester author push/pop, Michael reclamation and retained N1570 draft clauses are accurately identified.','Reference executions and author-team source review are not independent human assessment or delayed learner evidence.']))
 paths=set(campaign['inspected_paths'])|set(export['input_sha256'])|set(cost['input_sha256'])|set(REPORTS)|set(teaching+code)
 paths|={BASE+'/'+p for p in ['resources.json','taxonomy_extensions.json','research_gate.json','audit.json']}
 paths|={'tools/author_corpus_treiber.py','tools/close_corpus_treiber.py','research/v5/expansion/legacy_advanced_reclamation/resources.json'}
 out=ROOT/BASE/'completion_dispositions.json';result=dict(schema_version=1,scope='Required finite84 lock-free-stack-treiber actual implementation, not tagged-pool substitution.',source_sha256={p:sha(ROOT/p) for p in sorted(paths)},current_reports=REPORTS,updates=rows)
 out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=True,updates=len(rows),path=out.relative_to(ROOT).as_posix(),sha256=sha(out))))
if __name__=='__main__':main()
