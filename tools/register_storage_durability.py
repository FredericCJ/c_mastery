"""Attach exact recovery/medium support without closing unrelated delayed tasks."""
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/legacy_storage_followthrough';PUBLIC='labs/storage_followthrough/durability'
EXCLUDED={'AX-PR-T47-07-T47.07.b-DELAYED','AX-PR-T47-09-T47.09.b-DELAYED'}
LOCATORS=['Prerequisites and representation','Stable and pending model','Complete validation before home publication','Commit, checkpoint and recovery experiments','Finite model and security scope','Deliverables, assessment and hints']

def ids(project):
 return [identity for c in read_json(ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json'))['criteria'] for identity in [c['id'],c['id']+'-DELAYED'] if identity not in EXCLUDED]

def main():
 path=PUBLIC+'/CONTRACTS.md'
 resource=dict(id='R-STORAGE-FOLLOW-DURABILITY',title='Persistence authority, structural preflight and repeatable recovery cuts',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T47'],role='primary',reason='Actual original source comparison, bounded read-only recovery repair, persistence medium, three-slot profile, explicit-state checker and CRC collision.',prerequisites=['PR-T47-01/03/04/05/06'],assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read complete original contract and actual original install/open/commit/checkpoint/checker bodies; inspect complete original adapter and model',scope='; '.join(LOCATORS),limits='No newly inspected external storage theorem, published ISO edition or physical persistence authority.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'durability_resources.json',dict(resources=[resource]));paths=[path,'assessor/storage/store.c','labs/storage/FORMAT.md','labs/storage_followthrough/durability/instrument_store.py','assessor/storage_followthrough/durability/guard.c','assessor/storage_followthrough/durability/medium.c','tools/model_storage_protocol.py']
 dump(ROOT/BASE/'durability_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in paths},resources=BASE+'/durability_resources.json',inspection=dict(retained='Actual complete original source previously read; specific install/open/commit/checkpoint/decoder/checker contracts and current successor/model inspected.',limits='Original accepted source preserved; no new external or physical evidence inferred.')))
 assets=[PUBLIC+'/'+p for p in ['medium.h','medium.c','guard.h','guard.c','tests.c','CMakeLists.txt','expect_failure.py','instrument_store.py','CONTRACTS.md','third/include/store.h','third/store.c','third/tests.c']]+['labs/storage/include/store.h','labs/storage/src/store.c','labs/storage/FORMAT.md','labs/storage/INTEGRATION.md']
 for number in range(7,11):
  project=f'PR-T47-{number:02}';p=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');d=read_json(p);identities=ids(project)
  d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]));source=dict(id=resource['id'],path=BASE+'/durability_resources.json',passages=LOCATORS);d['source_records']=[r for r in d['source_records'] if r['id']!=source['id']]+[source]
  binding=dict(path=BASE+'/durability_completion_dispositions.json',criterion_ids=identities,scope='Only listed original ordinary/delayed obligations; application operation-ID and logical-redo support are separate. Physical durability and independent learner evidence remain unobserved.')
  d['execution_support']['storage_durability_successor']=binding
  for c in d['criteria']:c['durability_support']=dict(binding,criterion_ids=[i for i in identities if i in [c['id'],c['id']+'-DELAYED']])
  dump(p,d)
 print(dict(attached=['PR-T47-07','PR-T47-08','PR-T47-09','PR-T47-10'],ordinary=12,delayed=10,baseline_observations='preserved'))
if __name__=='__main__':main()
