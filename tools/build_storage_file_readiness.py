"""Bind only the five exact original and changed file-adapter obligations."""
from common import ROOT,dump,read_json,sha
from check_storage_file import REPORT,EXPORT,check,check_export
BASE='research/v5/expansion/legacy_storage_followthrough'
CLAIMS={
 'a':'Actual unchanged original store_file.c is linked with syscall wrappers;40954histories cover every positive first short1..4095, EINTR before/after progress, zero/error at every prefix0..4095 and both directions. Offsets reach logicalUINT32_MAX-1 under the explicit64-bit-off_t profile. Failed raw reads retain their accepted prefix rather than falsely promising unchanged output.',
 'a-DELAYED':'Implemented staged SfiOp carries exact-owner/nonwrapping generation, copied writes, unpublished read scratch, prefix count, terminal/release state and per-step callback budget. AGAIN requires fresh readiness; cancellation retains write progress and read output. Actual O_NONBLOCK pipe/poll executes EAGAIN,13/4083-byte progress and hangup. This is a single ordered page operation, not a seekable network block service.',
 'b':'Actual original preallocated Linux file adapter formats, commits, closes and reopens with complete value checks. Real fsync plus injected EINTR and persisted-before-EIO final-sync outcome execute poisoning/reconciliation. Kernel/filesystem/off_t and accepted-byte observations are recorded separately from physical durability.',
 'b-DELAYED':'Actual Linux constructor creates only a new simple component in an exclusive private directory, preallocates, formats through the retained adapter and performs file then directory fsync before output publication. Failed directory synchronization retains named evidence, consumes the descriptor once and preserves output. A post-close EINTR control verifies no retry by descriptor number; Linux versus POSIX.1-2024 scope is explicit.',
 'c':'The correctness-checked one-byte hot-root update derives and observes commit0reads/34writes/3flushes and checkpoint65reads/34writes/3flushes. Independent oracle checks139264commit write bytes and266240read/139264write checkpoint bytes against one useful byte, with actual original provider/syscall counters. Filesystem/device traffic and timing are explicitly outside these logical measurements.'}
def main():
 d,e=read_json(ROOT/REPORT),read_json(ROOT/EXPORT);check(d);check_export(e)
 teaching=['curriculum/v5/expansion/legacy_assessments/PR-T47-11.json','labs/storage_followthrough/file/CONTRACTS.md','assessor/storage_followthrough/file/ANALYSIS.md']
 implementation=['assessor/storage_followthrough/file/file.c','labs/storage_followthrough/file/file.h','labs/storage_followthrough/file/tests.c','tools/verify_storage_file.py','assessor/storage/store.c','labs/storage/src/store_file.c','labs/storage/include/store_file.h']
 criteria=[];updates=[]
 for suffix,claim in CLAIMS.items():
  identity='AX-PR-T47-11-T47.11.'+suffix
  dims=dict(teaching=dict(state='complete',evidence=teaching),implementation=dict(state='complete',evidence=implementation),author_validation=dict(state='passed',evidence=[REPORT,EXPORT,BASE+'/file_readiness.json']),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-applicable',evidence=['labs/storage_followthrough/file/CONTRACTS.md']))
  criteria.append(dict(id=identity,author_readiness='ready-bounded',demonstration=claim,teaching=teaching,implementation=implementation,validation=[REPORT,EXPORT],external_review='not-run',independent_learner='not-assessed',physical_observation='not-applicable'))
  updates.append(dict(id=identity,dimensions=dims,closure_rationale=claim+' Bounded author support only; no physical persistence, human-independent review or learner mastery is inferred.'))
 inputs={**d['input_sha256'],**e['input_sha256']}
 for p in teaching+implementation+[REPORT,EXPORT,BASE+'/file_source_access.json',BASE+'/file_resources.json',BASE+'/file_research_gate.json','tools/check_storage_file.py','tools/register_storage_file.py','tools/build_storage_file_readiness.py','tools/check_replicated_replay_depth.py']:inputs[p]=sha(ROOT/p)
 ready=BASE+'/file_readiness.json';dump(ROOT/ready,dict(schema_version=1,criteria=criteria,input_sha256=inputs,current_reports={p:sha(ROOT/p) for p in [REPORT,EXPORT]},limits=['Only five exact ordinary/delayed obligations; changed-page redo remains separate.','Actual Linux file/poll behavior does not establish physical persistence or arbitrary-backend equivalence.','No human-independent assessment or learner mastery.']))
 inputs[ready]=sha(ROOT/ready);dump(ROOT/BASE/'file_completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=[REPORT,EXPORT],input_sha256=inputs));print(dict(passed=True,updates=len(updates)))
if __name__=='__main__':main()
