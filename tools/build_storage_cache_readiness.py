"""Map original cache criteria and exact changed tasks to bounded author evidence."""
from common import ROOT,dump,read_json,sha
from check_storage_cache import REPORT,EXPORT,check,check_export
BASE='research/v5/expansion/legacy_storage_followthrough'
def main():
    d,e=read_json(ROOT/REPORT),read_json(ROOT/EXPORT);check(d);check_export(e)
    project='curriculum/v5/expansion/legacy_assessments/PR-T47-02.json'
    teaching=[project,'labs/storage/FORMAT.md','labs/storage_followthrough/cache/CONTRACTS.md','assessor/storage_followthrough/cache/ANALYSIS.md']
    implementation=['assessor/storage/store.c','assessor/storage_followthrough/cache/cache.c','labs/storage_followthrough/cache/tests.c','labs/storage_followthrough/cache/mutate_store.py','tools/verify_storage_cache.py']
    claims={
      'a':'Actual retained physical-page-tag cache and successor direct policy execute1,1,5,1 with three backend reads and one hit. Actual transactional scratch remains separate; scratch lookup induces no backend write. Native cache/entry/scratch sizes and actual compiler stack/import closure are recorded.',
      'a-DELAYED':'A real four-entry two-way LRU policy replaces only the least recent way, updates recency on successful hit and preserves ownership. The original parent pinned-root variant also uses exactly four payload pages with three conflict slots, not an extra page. Exact five-read traces and the independent policy model distinguish replacements.',
      'b':'Actual store commit/checkpoint are exercised, and each separately omitted invalidation is rejected by its explicit empty-cache postcondition. The companion comparisons expose that physical tags plus the other invalidation mask stale bytes for the selected valid trace. Combined omission demonstrates old raw generation0 at current generation1; actual store_get contains it as STORE_CORRUPT with unchanged value/length. This is an explicit bounded correction to overstated individual stale-value wording, not invented execution.',
      'b-DELAYED':'Actual20-key insert, full deletion and another20-key insertion free and reallocate page2 while generation advances1 to3. Exact owner/generation/physical tuple prevents the repeated WAL slot from hitting old bytes. Separate image and home/WAL changes are tested; supplied current authority remains a trusted caller premise.',
      'c':'The independent Python cache/tag/replacement model checks7203 four-operation histories and28812transitions across direct, two-way and root-pinned policies, including changed image/generation/view and partial-read failures. Observed cache attempts/hits, fixed memory and copied pages remain distinct from unmeasured filesystem/device traffic.',
      'c-DELAYED':'The actual eight-page double scan produces16reads/0hits for direct/two-way and15reads/1hit with the root pinned; sixteen repeated root point reads produce1read/15hits under every policy. Each workload begins with a cold cache and the same four-page payload budget. No physical latency conclusion is inferred.'}
    criteria=[];updates=[]
    for suffix,claim in claims.items():
        identity='AX-PR-T47-02-T47.02.'+suffix
        dims=dict(teaching=dict(state='complete',evidence=teaching),implementation=dict(state='complete',evidence=implementation),author_validation=dict(state='passed',evidence=[REPORT,EXPORT,BASE+'/cache_readiness.json']),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-applicable',evidence=['labs/storage_followthrough/cache/CONTRACTS.md']))
        criteria.append(dict(id=identity,author_readiness='ready-bounded',demonstration=claim,teaching=teaching,implementation=implementation,validation=[REPORT,EXPORT],external_review='not-run',independent_learner='not-assessed',physical_observation='not-applicable'))
        updates.append(dict(id=identity,dimensions=dims,closure_rationale=claim+' Bounded author support; independent learner/human evidence remains unobserved.'))
    inputs={**d['input_sha256'],**e['input_sha256']}
    for p in teaching+implementation+[REPORT,EXPORT,BASE+'/cache_resources.json',BASE+'/cache_research_gate.json','tools/check_storage_cache.py','tools/register_storage_cache.py','tools/build_storage_cache_readiness.py','tools/check_replicated_replay_depth.py']:inputs[p]=sha(ROOT/p)
    ready=BASE+'/cache_readiness.json';dump(ROOT/ready,dict(schema_version=1,criteria=criteria,input_sha256=inputs,current_reports={p:sha(ROOT/p) for p in [REPORT,EXPORT]},limits=['Preserved original task text is supplemented by the exact physical-tag masking correction.','Generated controls are reproduced byte-for-byte, archived and bound to the unchanged actual store source.','Finite serialized traces do not establish arbitrary concurrent cache behavior or physical performance.']))
    inputs[ready]=sha(ROOT/ready);dump(ROOT/BASE/'cache_completion_dispositions.json',dict(schema_version=1,updates=updates,current_reports=[REPORT,EXPORT],input_sha256=inputs));print(dict(passed=True,updates=6))
if __name__=='__main__':main()
