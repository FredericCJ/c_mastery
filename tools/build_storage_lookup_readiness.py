"""Bind exact original lookup tasks and changed lifetime/collation mechanisms."""
from common import ROOT,dump,read_json,sha
from check_storage_lookup import REPORT,EXPORT,check,check_export
BASE='research/v5/expansion/legacy_storage_followthrough'
def main():
    d,e=read_json(ROOT/REPORT),read_json(ROOT/EXPORT);check(d);check_export(e)
    project='curriculum/v5/expansion/legacy_assessments/PR-T47-03.json'
    teaching=[project,'labs/storage/FORMAT.md','labs/storage_followthrough/lookup/CONTRACTS.md','assessor/storage_followthrough/lookup/ANALYSIS.md']
    implementation=['assessor/storage/store.c','assessor/storage_followthrough/lookup/lookup.c','labs/storage_followthrough/lookup/tests.c','tools/verify_storage_lookup.py']
    claims={
      'a':'Actual retained tree inserts92 binary records under a changed permutation, validates after each update, retrieves every record and queries every internal separator equality. Embedded zeros, prefix pairs and high-bit unsigned ordering match an independent Python byte map; the actual retained wrong-equality control is rejected.',
      'a-DELAYED':'The original printable-ASCII normalization facade stages bytes/length, folds only A..Z, rejects noncanonical preexisting stores, and calls real bind/put/get/scan operations. Case collisions replace one canonical key, invalid input preserves output and overlap normalization is idempotent. All65793 empty/one/two-octet inputs are compared independently; length16/17 bounds are separately exercised. No locale/Unicode equivalence is claimed.',
      'b':'Actual scans probe before/at/between/after keys and exact binary prefixes, with zero/one/three accepted-record stops and complete map comparison. Actual get NOT_FOUND/FULL/invalid-key outcomes preserve caller bytes/length. Invocation count is distinguished from accepted records; transient callback borrows require copying.',
      'b-DELAYED':'An actual owned217-record inline snapshot first validates the source then finishes copying before client callbacks. The reentry fixture deletes92 source keys, validates each update, captures a separate91-record snapshot in the first callback, and still receives all92 original outer records. Source generation/staged status are descriptive; copying supplies lifetime independence without claiming concurrent shared-page readers.',
      'c':'The actual tree checker cross-validates tree traversal, leaf links and allocation after each insert/delete. Independently injected out-of-range child, leaf cycle and unreachable allocated page reject boundedly before copied-snapshot publication. Actual dangling/cycle scan errors are checked; active typed fixtures require no backend reads.',
      'c-DELAYED':'A valid tree is changed into an acyclic shared-subtree graph by duplicating a root child. The actual visited-page ownership check rejects reuse of the child and snapshot capture preserves prior output. This explicitly distinguishes exclusive tree ownership from absence of cycles and from a superficially ordered leaf-chain scan.'}
    criteria=[];updates=[]
    for suffix,claim in claims.items():
        identity='AX-PR-T47-03-T47.03.'+suffix
        dims=dict(teaching=dict(state='complete',evidence=teaching),implementation=dict(state='complete',evidence=implementation),author_validation=dict(state='passed',evidence=[REPORT,EXPORT,BASE+'/lookup_readiness.json']),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-applicable',evidence=['labs/storage_followthrough/lookup/CONTRACTS.md']))
        criteria.append(dict(id=identity,author_readiness='ready-bounded',demonstration=claim,teaching=teaching,implementation=implementation,validation=[REPORT,EXPORT],external_review='not-run',independent_learner='not-assessed',physical_observation='not-applicable'))
        updates.append(dict(id=identity,dimensions=dims,closure_rationale=claim+' Bounded author support only; independent learner transfer remains unobserved.'))
    inputs={**d['input_sha256'],**e['input_sha256']}
    for p in teaching+implementation+[REPORT,EXPORT,BASE+'/lookup_resources.json',BASE+'/lookup_research_gate.json','tools/check_storage_lookup.py','tools/register_storage_lookup.py','tools/build_storage_lookup_readiness.py','tools/check_replicated_replay_depth.py']:inputs[p]=sha(ROOT/p)
    ready=BASE+'/lookup_readiness.json';dump(ROOT/ready,dict(schema_version=1,criteria=criteria,input_sha256=inputs,current_reports={p:sha(ROOT/p) for p in [REPORT,EXPORT]},limits=['Exact ASCII changed-domain contract, not Unicode case folding.','Snapshots own copied bytes and do not implement concurrent version/reclamation machinery.','No human-independent review, delayed learner administration or physical behavior inferred.']))
    inputs[ready]=sha(ROOT/ready);dump(ROOT/BASE/'lookup_completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=[REPORT,EXPORT],input_sha256=inputs));print(dict(passed=True,updates=6))
if __name__=='__main__':main()
