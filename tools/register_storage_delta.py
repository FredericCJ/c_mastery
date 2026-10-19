"""Attach exactly the original changed-page redo obligation and public scaffolds."""
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/legacy_storage_followthrough';PUBLIC='labs/storage_followthrough/delta'
IDENTITY='AX-PR-T47-11-T47.11.c-DELAYED'
LOCATORS=['Prerequisites, imports and environment','Representation and publication protocol','Ownership, failures and costs','Concrete workload and falsification','Deliverables, rubric and transfer']
def main():
 path=PUBLIC+'/CONTRACTS.md'
 resource=dict(id='R-STORAGE-FOLLOW-DELTA',title='Changed-page redo under an equal transaction contract',url=path,authors=['Curriculum authors'],edition='Original repository protocol; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T47'],role='primary',reason='Complete new raw-page protocol and exact comparison; no CST2 format equivalence claim.',prerequisites=['PR-T47-01','PR-T47-06','PR-T47-07','PR-T47-08','PR-T47-09','PR-T47-10'],assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read complete original contract, implementation and tests plus actual imported medium and store_io declaration',scope='; '.join(LOCATORS),limits='Original bounded protocol; not a whole-database, physical-durability or external-standard inspection.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'delta_resources.json',dict(resources=[resource]))
 paths=[path,'assessor/storage_followthrough/delta/delta.c','labs/storage_followthrough/delta/tests.c','labs/storage/include/store.h','labs/storage/FORMAT.md','assessor/storage_followthrough/durability/medium.c','labs/storage_followthrough/durability/medium.h']
 dump(ROOT/BASE/'delta_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in paths},resources=BASE+'/delta_resources.json',inspection=dict(scope='Complete new SDW1 full/sparse protocol and actual selected imports.',limits='No new published ISO C17/C23 or physical device inspection.')))
 p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T47-11.json';d=read_json(p)
 assets=[PUBLIC+'/'+n for n in ['delta.h','delta.c','tests.c','CMakeLists.txt','expect_failure.py','CONTRACTS.md']]+['labs/storage/include/store.h','labs/storage/FORMAT.md','labs/storage_followthrough/durability/medium.h','labs/storage_followthrough/durability/medium.c','labs/storage_followthrough/durability/CONTRACTS.md']
 d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]));d['source_records']=[r for r in d['source_records'] if r['id']!=resource['id']]+[dict(id=resource['id'],path=BASE+'/delta_resources.json',passages=LOCATORS)]
 binding=dict(path=BASE+'/delta_completion_dispositions.json',criterion_ids=[IDENTITY],scope='Only the exact changed-page-redo variant; same raw-page capacity/atomicity/failure contract across full and sparse policies, not CST2 format compatibility.')
 d['execution_support']['storage_delta_successor']=binding
 for c in d['criteria']:
  if c['id']+'-DELAYED'==IDENTITY:c['delta_support']=binding
 dump(p,d);print(dict(attached='PR-T47-11',ordinary=0,delayed=1,historical_baseline='preserved'))
if __name__=='__main__':main()
