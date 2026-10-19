"""Attach exactly the original logical-redo obligation and public scaffolds."""
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/legacy_storage_followthrough';PUBLIC='labs/storage_followthrough/logical'
IDENTITY='AX-PR-T47-09-T47.09.b-DELAYED'
LOCATORS=['Prerequisites, imports and environment','Representation and ordering','Ownership, failure and resource contracts','Workloads, independent checks and controls','Deliverables, rubric and changed assumptions']
def main():
 path=PUBLIC+'/CONTRACTS.md'
 resource=dict(id='R-STORAGE-FOLLOW-LOGICAL',title='Logical redo and applied-operation authority',url=path,authors=['Curriculum authors'],edition='Original repository protocol; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T47'],role='primary',reason='Logical ADD replay and durable applied-ID authority; not a physical after-image WAL or general database protocol.',prerequisites=['PR-T47-01','PR-T47-06','PR-T47-07','PR-T47-08','PR-T47-09','PR-T47-10'],assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read complete original contract, implementation and tests plus actual imported medium and store_io declaration',scope='; '.join(LOCATORS),limits='Original bounded protocol; not a whole-database, physical-durability or external-standard inspection.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'logical_resources.json',dict(resources=[resource]))
 paths=[path,'assessor/storage_followthrough/logical/logical.c','labs/storage_followthrough/logical/tests.c','labs/storage/include/store.h','labs/storage/FORMAT.md','assessor/storage_followthrough/durability/medium.c','labs/storage_followthrough/durability/medium.h']
 dump(ROOT/BASE/'logical_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in paths},resources=BASE+'/logical_resources.json',inspection=dict(scope='Complete new SLR1 logical-command protocol and actual selected imports.',limits='No new published ISO C17/C23 or physical device inspection.')))
 p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T47-09.json';d=read_json(p)
 assets=[PUBLIC+'/'+n for n in ['logical.h','logical.c','tests.c','CMakeLists.txt','expect_failure.py','CONTRACTS.md']]+['labs/storage/include/store.h','labs/storage/FORMAT.md','labs/storage_followthrough/durability/medium.h','labs/storage_followthrough/durability/medium.c','labs/storage_followthrough/durability/CONTRACTS.md']
 d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]));d['source_records']=[r for r in d['source_records'] if r['id']!=resource['id']]+[dict(id=resource['id'],path=BASE+'/logical_resources.json',passages=LOCATORS)]
 binding=dict(path=BASE+'/logical_completion_dispositions.json',criterion_ids=[IDENTITY],scope='Only the exact logical-redo variant; logical ADD with predecessor/applied sequence and latest-ID receipt, not arbitrary B+tree redo.')
 d['execution_support']['storage_logical_successor']=binding
 for c in d['criteria']:
  if c['id']+'-DELAYED'==IDENTITY:c['logical_support']=binding
 dump(p,d);print(dict(attached='PR-T47-09',ordinary=0,delayed=1,historical_baseline='preserved'))
if __name__=='__main__':main()
