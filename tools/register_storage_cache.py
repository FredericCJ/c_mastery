"""Attach cache successor support without changing retained assessments."""
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/legacy_storage_followthrough'
PUBLIC='labs/storage_followthrough/cache'
def main():
    path=PUBLIC+'/CONTRACTS.md';locators=['Clean storage, identity and admission','Publication, invalidation and a falsifiable correction','Objective evidence and rubric','Verification and limits']
    resource=dict(id='R-STORAGE-FOLLOW-CACHE',title='Clean cache identity and replacement successor contract',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T47'],role='primary',reason='Explicit policy, generation, ownership and bounded correction to the original invalidation task.',prerequisites=['PR-T47-01','PR-T47-02','PR-T47-05/07/08 for actual publication and reuse integration'],assignable=True,locators=locators,inspection=dict(date='2026-10-18',method='Read the actual complete local contract and named retained implementation',scope='; '.join(locators),limits='Original author contract; does not replace exact published C17, device measurements or independent assessment.',local_sha256=sha(ROOT/path)))
    dump(ROOT/BASE/'cache_resources.json',dict(resources=[resource]));dump(ROOT/BASE/'cache_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={path:sha(ROOT/path),'assessor/storage/store.c':sha(ROOT/'assessor/storage/store.c')},resources=BASE+'/cache_resources.json',inspection=dict(retained='cached(), invalidate(), store_begin(), store_commit() and install(); actual physical tags and invalidation ordering inspected',limits='No fresh complete external paper/book or published C17 inspection')))
    p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T47-02.json';d=read_json(p)
    assets=[PUBLIC+'/'+name for name in ['cache.h','cache.c','tests.c','CMakeLists.txt','expect_failure.py','mutate_store.py','CONTRACTS.md']]+['labs/storage/include/store.h','labs/storage/src/store.c','labs/storage/FORMAT.md']
    d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]))
    source=dict(id=resource['id'],path=BASE+'/cache_resources.json',passages=locators);d['source_records']=[r for r in d['source_records'] if r['id']!=source['id']]+[source]
    binding=dict(path=BASE+'/cache_completion_dispositions.json',criterion_ids=[c['id'] for c in d['criteria']],scope='Actual C17 four-entry policies, original store transitions/page reuse, independent trace oracle and compiled invalidation counterexamples; exact original ordinary/delayed tasks with explicit masking correction. No physical/human/learner inference.')
    d['execution_support']['storage_cache_successor']=binding
    for c in d['criteria']:c['successor_support']=dict(binding,criterion_ids=[c['id']])
    dump(p,d);print(dict(attached='PR-T47-02',ordinary=3,delayed=3,baseline_observations='preserved'))
if __name__=='__main__':main()
