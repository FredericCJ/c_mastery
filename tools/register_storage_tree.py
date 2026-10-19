"""Attach only actual split/merge/path-repair obligations; preserve baseline."""
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/legacy_storage_followthrough';PUBLIC='labs/storage_followthrough/tree'
CRITERIA={'PR-T47-04':['a','c'],'PR-T47-05':['a','b','c']}
def main():
 path=PUBLIC+'/CONTRACTS.md';locators=['Representation and dependency boundary','Derive the split and deletion rules','Changed separator repair and its costs','Concrete workloads and independent verification','Deliverables, rubric and delayed transfer']
 resource=dict(id='R-STORAGE-FOLLOW-TREE',title='Two B+tree capacities and affected-node separator repair',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T47'],role='primary',reason='Explicit capacity, fork ABI/format, ownership and local repair contract with separate full-refresh comparison.',prerequisites=['PR-T47-01/02/03/04/05','PR-T47-06/07/08/09 for persistence integration'],assignable=True,locators=locators,inspection=dict(date='2026-10-18',method='Read complete original contract and actual retained mutation/checker bodies; derive and inspect the separate implementation',scope='; '.join(locators),limits='This original contract is not external authority or an independently reviewed correctness theorem.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'tree_resources.json',dict(resources=[resource]));dump(ROOT/BASE/'tree_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={path:sha(ROOT/path),'assessor/storage/store.c':sha(ROOT/'assessor/storage/store.c'),'labs/storage/include/store.h':sha(ROOT/'labs/storage/include/store.h')},resources=BASE+'/tree_resources.json',inspection=dict(retained='Complete alloc_node/entry/choose/insert/refresh/store_put/remove_parent/store_delete/check_node/store_check and original codec/persistence; complete parent T47.04/05 tasks.',limits='No new complete ARIES/book or published ISO inspection')))
 assets=[PUBLIC+'/'+p for p in ['include/tree.h','include/store.h','tree.c','tests.c','CMakeLists.txt','expect_failure.py','CONTRACTS.md']]+['labs/storage/FORMAT.md']
 for project,suffixes in CRITERIA.items():
  p=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');d=read_json(p)
  d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]));source=dict(id=resource['id'],path=BASE+'/tree_resources.json',passages=locators);d['source_records']=[r for r in d['source_records'] if r['id']!=source['id']]+[source]
  ids=[c['id'] for c in d['criteria'] if c['objective'][-1] in suffixes]
  binding=dict(path=BASE+'/tree_completion_dispositions.json',criterion_ids=ids,scope='Capacity7/3 split/merge/ownership, exact affected-node repair versus full refresh, independent prefix map and format oracles; T47.05.b generation-tagged reference variant remains separate.')
  d['execution_support']['storage_tree_successor']=binding
  for c in d['criteria']:
   if c['id'] in ids:c['successor_support']=dict(binding,criterion_ids=[c['id']])
  dump(p,d)
 print(dict(attached=list(CRITERIA),ordinary=5,delayed=4,baseline_observations='preserved'))
if __name__=='__main__':main()
