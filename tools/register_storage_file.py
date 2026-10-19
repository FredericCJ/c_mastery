"""Register exact file-transfer and new-name support with inspected primary passages."""
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/legacy_storage_followthrough';PUBLIC='labs/storage_followthrough/file'
IDS=['AX-PR-T47-11-T47.11.a','AX-PR-T47-11-T47.11.a-DELAYED','AX-PR-T47-11-T47.11.b','AX-PR-T47-11-T47.11.b-DELAYED','AX-PR-T47-11-T47.11.c']
LOCATORS=['Prerequisites, imports and environment','Original positioned-transfer investigation','A staged nonblocking operation','Existing file versus newly published name','Costs, falsification and assessment']
def main():
 path=PUBLIC+'/CONTRACTS.md';resources=[]
 for r in read_json(ROOT/BASE/'file_source_access.json')['resources']:
  resources.append(dict(id=r['id'],title=r['id'].split('-')[-1].lower()+'(2), Linux manual page',url=r['url'],authors=['Linux man-pages project'],edition=r['edition'],year=2026,kind='manual',access='Open HTML; selected passages inspected',domains=['T47'],role='primary',reason='Selected actual file/readiness/synchronization/close semantics; separate from ISO C and physical measurements.',prerequisites=['PR-T47-01..10'],assignable=True,locators=r['inspection']['scope'].split('; '),inspection=r['inspection'],retrieved_sha256=r['retrieved_sha256']))
 resources.append(dict(id='R-STORAGE-FOLLOW-FILE',title='Exact file transfers, readiness and a new image name',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T47'],role='primary',reason='Actual original syscall-wrapper experiments and separate nonblocking/new-name implementations.',prerequisites=['PR-T47-01..10'],assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read complete authored contract and original file adapter plus actual successor/callback code',scope='; '.join(LOCATORS),limits='No physical device or whole POSIX/ISO authority claim.',local_sha256=sha(ROOT/path))))
 dump(ROOT/BASE/'file_resources.json',dict(resources=resources))
 paths=[path,'labs/storage/src/store_file.c','labs/storage/include/store_file.h','assessor/storage_followthrough/file/file.c',BASE+'/file_source_access.json']
 dump(ROOT/BASE/'file_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in paths},resources=BASE+'/file_resources.json',inspection=dict(scope='Explicit selected primary-manual passages and complete actual original/successor file adapters.',limits='Exact published ISO C17/C23 remain unavailable; no physical persistence inference.')))
 p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T47-11.json';d=read_json(p)
 assets=[PUBLIC+'/'+n for n in ['file.h','file.c','tests.c','CMakeLists.txt','expect_failure.py','CONTRACTS.md']]+['labs/storage/include/store.h','labs/storage/src/store.c','labs/storage/include/store_file.h','labs/storage/src/store_file.c','labs/storage/FORMAT.md','labs/storage/INTEGRATION.md']
 d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]));added={r['id'] for r in resources};d['source_records']=[r for r in d['source_records'] if r['id'] not in added]+[dict(id=r['id'],path=BASE+'/file_resources.json',passages=r['locators']) for r in resources]
 binding=dict(path=BASE+'/file_completion_dispositions.json',criterion_ids=IDS,scope='Only five exact ordinary/delayed obligations; changed-page redo remains separate. Linux observations are distinct from physical durability.')
 d['execution_support']['storage_file_successor']=binding
 for c in d['criteria']:c['file_support']=dict(binding,criterion_ids=[i for i in IDS if i in [c['id'],c['id']+'-DELAYED']])
 dump(p,d);print(dict(attached='PR-T47-11',ordinary=3,delayed=2,baseline_observations='preserved'))
if __name__=='__main__':main()
