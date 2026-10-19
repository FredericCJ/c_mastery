"""Bind only the original changed-page redo delayed obligation."""
from common import ROOT,dump,read_json,sha
from check_storage_delta import REPORT,EXPORT,check,check_export
BASE='research/v5/expansion/legacy_storage_followthrough'
def main():
 d,e=read_json(ROOT/REPORT),read_json(ROOT/EXPORT);check(d);check_export(e)
 identity='AX-PR-T47-11-T47.11.c-DELAYED'
 claim='Implemented distinct SDW1 full/sparse redo with the same32raw-page atomic API, capacity, generation and failure semantics.315commit cuts,315checkpoint cuts,1433repeated interruptions and complete32-page recovery checks per profile; candidate-corruption/read-failure preflight; maximum-generation disk fixture; four semantic mutants. Independent Python checks315phase outcomes,two4096-byte markers and equal-contract costs: for3useful bytes sparse commit0/5/3 and checkpoint36/5/3 versus full0/34/3 and65/34/3 reads/writes/flushes. This is a new protocol, not CST2 compatibility or physical-I/O equivalence.'
 teaching=['curriculum/v5/expansion/legacy_assessments/PR-T47-11.json','labs/storage_followthrough/delta/CONTRACTS.md','assessor/storage_followthrough/delta/ANALYSIS.md']
 implementation=['assessor/storage_followthrough/delta/delta.c','labs/storage_followthrough/delta/delta.h','labs/storage_followthrough/delta/tests.c','tools/verify_storage_delta.py','assessor/storage_followthrough/durability/medium.c','labs/storage/include/store.h']
 dims=dict(teaching=dict(state='complete',evidence=teaching),implementation=dict(state='complete',evidence=implementation),author_validation=dict(state='passed',evidence=[REPORT,EXPORT,BASE+'/delta_readiness.json']),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-applicable',evidence=['labs/storage_followthrough/delta/CONTRACTS.md']))
 row=dict(id=identity,author_readiness='ready-bounded',demonstration=claim,teaching=teaching,implementation=implementation,validation=[REPORT,EXPORT],external_review='not-run',independent_learner='not-assessed',physical_observation='not-applicable')
 inputs={**d['input_sha256'],**e['input_sha256']}
 for p in teaching+implementation+[REPORT,EXPORT,BASE+'/delta_resources.json',BASE+'/delta_research_gate.json','tools/check_storage_delta.py','tools/register_storage_delta.py','tools/build_storage_delta_readiness.py','tools/check_replicated_replay_depth.py']:inputs[p]=sha(ROOT/p)
 ready=BASE+'/delta_readiness.json';dump(ROOT/ready,dict(schema_version=1,criteria=[row],input_sha256=inputs,current_reports={p:sha(ROOT/p) for p in [REPORT,EXPORT]},limits=['Only one exact delayed obligation.','Finite synthetic persistence choices; no physical latency/durability or CST2 format compatibility.','No human-independent assessment or learner mastery.']))
 inputs[ready]=sha(ROOT/ready);dump(ROOT/BASE/'delta_completion_dispositions.json',dict(schema_version=1,updates=[dict(id=identity,dimensions=dims,closure_rationale=claim+' Bounded author support only; no physical persistence, human-independent review or learner mastery.')],current_reports=[REPORT,EXPORT],input_sha256=inputs));print(dict(passed=True,updates=1))
if __name__=='__main__':main()
