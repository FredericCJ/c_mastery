"""Bind only the two exact retained-reader/shared-cache delayed obligations."""
from common import ROOT,dump,read_json,sha
from check_storage_readers import REPORT,EXPORT,check,check_export
BASE='research/v5/expansion/legacy_storage_followthrough'
def main():
 d,e=read_json(ROOT/REPORT),read_json(ROOT/EXPORT);check(d);check_export(e)
 teaching=['curriculum/v5/expansion/legacy_assessments/PR-T47-06.json','labs/storage_followthrough/readers/CONTRACTS.md','assessor/storage_followthrough/readers/ANALYSIS.md']
 implementation=['assessor/storage_followthrough/readers/readers.c','labs/storage_followthrough/readers/readers.h','labs/storage_followthrough/readers/tests.c','tools/verify_storage_readers.py','assessor/storage/store.c','assessor/storage_followthrough/cache/cache.c','assessor/storage_followthrough/lookup/lookup.c']
 claims={
 'AX-PR-T47-06-T47.06.a-DELAYED':'Actual two-slot copied snapshot retention composes SlSnapshot under a hub mutex, publishes exact owner/slot/nonwrapping serial and preserves versions across later commits, source poison and close. MAX retires only after cleanup; stale/double release cannot consume a reused slot. Close retains readers until release, with joined callers required before mutex destruction. The contract/analysis explicitly derives additional root/page-version/reader-registration/reclamation machinery a zero-copy design would need. This is implemented full-copy retention, not a claim of page-level MVCC.',
 'AX-PR-T47-06-T47.06.c-DELAYED':'Two actual original store handles share the same ScCache through their real page IO callbacks under one POSIX mutex. Publication advances trusted cache revision before checkpoint and reconstructs the reader metadata/private cache. Exact shared-hit/physical-page checks and compiled omitted-revision/omitted-refresh controls distinguish cache-byte identity from handle authority. Native competing writer/reader threads, retained copied reads, IO failure poison and drain execute in eight C17 profiles including both thread sanitizers; finite216-history/648-transition oracle is separately bounded.'}
 criteria=[];updates=[]
 for identity,claim in claims.items():
  dims=dict(teaching=dict(state='complete',evidence=teaching),implementation=dict(state='complete',evidence=implementation),author_validation=dict(state='passed',evidence=[REPORT,EXPORT,BASE+'/readers_readiness.json']),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-applicable',evidence=['labs/storage_followthrough/readers/CONTRACTS.md']))
  criteria.append(dict(id=identity,author_readiness='ready-bounded',demonstration=claim,teaching=teaching,implementation=implementation,validation=[REPORT,EXPORT],external_review='not-run',independent_learner='not-assessed',physical_observation='not-applicable'))
  updates.append(dict(id=identity,dimensions=dims,closure_rationale=claim+' Bounded author support only; external human/learner evidence is not observed.'))
 inputs={**d['input_sha256'],**e['input_sha256']}
 for p in teaching+implementation+[REPORT,EXPORT,BASE+'/readers_resources.json',BASE+'/readers_research_gate.json','tools/check_storage_readers.py','tools/register_storage_readers.py','tools/build_storage_readers_readiness.py','tools/check_replicated_replay_depth.py']:inputs[p]=sha(ROOT/p)
 ready=BASE+'/readers_readiness.json';dump(ROOT/ready,dict(schema_version=1,criteria=criteria,input_sha256=inputs,current_reports={p:sha(ROOT/p) for p in [REPORT,EXPORT]},limits=['Only two named delayed obligations; original tasks/other variants remain in their own packets.','Mutex serialization and copied versions have explicit costs; no lock-free or arbitrary-interleaving claim.','No physical durability, human-independent review or learner mastery claim.']))
 inputs[ready]=sha(ROOT/ready);dump(ROOT/BASE/'readers_completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=[REPORT,EXPORT],input_sha256=inputs));print(dict(passed=True,updates=2))
if __name__=='__main__':main()
