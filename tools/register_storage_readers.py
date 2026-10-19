"""Attach the two exact snapshot/shared-cache changed-condition obligations."""
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/legacy_storage_followthrough';PUBLIC='labs/storage_followthrough/readers'
def main():
 path=PUBLIC+'/CONTRACTS.md';locators=['Prerequisites and representation','Publication and shared cache authority','Snapshot versions, retention and reclamation','Concrete investigation and evidence']
 resource=dict(id='R-STORAGE-FOLLOW-READERS',title='Retained copied readers and one cache across two actual handles',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T47'],role='primary',reason='Actual two-handle cache composition, copied snapshot lease and serialization/reclamation boundary.',prerequisites=['PR-T47-01/02/03/06/07/08/09'],assignable=True,locators=locators,inspection=dict(date='2026-10-18',method='Read complete original contract, actual imported store/cache/snapshot bodies and adapter',scope='; '.join(locators),limits='No new external POSIX/ISO authority inspection or independent-human proof.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'readers_resources.json',dict(resources=[resource]));paths=[path,'assessor/storage/store.c','assessor/storage_followthrough/cache/cache.c','assessor/storage_followthrough/lookup/lookup.c']
 dump(ROOT/BASE/'readers_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in paths},resources=BASE+'/readers_resources.json',inspection=dict(retained='Complete original store IO/cache/open/checkpoint/transaction/get bodies, ScCache implementation and SlSnapshot capture/scan bodies.',limits='No independently assessed concurrency or physical persistence claim')))
 p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T47-06.json';d=read_json(p)
 assets=[PUBLIC+'/'+name for name in ['readers.h','readers.c','tests.c','CMakeLists.txt','expect_failure.py','CONTRACTS.md']]+['labs/storage/include/store.h','labs/storage/src/store.c','labs/storage/FORMAT.md']
 for family in ['cache','lookup']:assets+=['labs/storage_followthrough/'+family+'/'+name for name in [family+'.h',family+'.c','CONTRACTS.md']]
 d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]));source=dict(id=resource['id'],path=BASE+'/readers_resources.json',passages=locators);d['source_records']=[r for r in d['source_records'] if r['id']!=source['id']]+[source]
 ids=['AX-PR-T47-06-T47.06.a-DELAYED','AX-PR-T47-06-T47.06.c-DELAYED'];binding=dict(path=BASE+'/readers_completion_dispositions.json',criterion_ids=ids,scope='Only the exact two changed-condition tasks: copied snapshot lifetime/version retention and actual two-store shared-cache publication/serialization; ordinary support remains separately bound.')
 d['execution_support']['storage_readers_successor']=binding
 for c in d['criteria']:
  if c['objective'][-1] in ['a','c']:c['changed_condition_support']=dict(binding,criterion_ids=[c['id']+'-DELAYED'])
 dump(p,d);print(dict(attached='PR-T47-06',ordinary=0,delayed=2,baseline_observations='preserved'))
if __name__=='__main__':main()
