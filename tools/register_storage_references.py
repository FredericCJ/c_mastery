"""Attach persistent allocation references to the original exact delayed task."""
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/legacy_storage_followthrough';PUBLIC='labs/storage_followthrough/references'
def main():
 path=PUBLIC+'/CONTRACTS.md';locators=['Inputs, prerequisites and execution profile','Three different identities','CST5 byte contract and persistence','Aborts, publication and finite resources','Concrete implementation and falsification']
 resource=dict(id='R-STORAGE-FOLLOW-REFERENCES',title='Allocation incarnations in a versioned tree format',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T47'],role='primary',reason='Concrete persistent allocation identities, moved reference pairs, retirement and unpublished rollback profile.',prerequisites=['PR-T47-01/02/03/04/05/06/07/08/09','Required capacity/path-repair successor'],assignable=True,locators=locators,inspection=dict(date='2026-10-18',method='Read complete local contract, original/fork codec and tree mutation, and complete generated source diff',scope='; '.join(locators),limits='Original technical specification, not an external standard or independent proof.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'references_resources.json',dict(resources=[resource]));paths=[path,'assessor/storage/store.c','assessor/storage_followthrough/tree/tree.c','labs/storage_followthrough/tree/include/store.h','tools/author_storage_references.py']
 dump(ROOT/BASE/'references_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in paths},resources=BASE+'/references_resources.json',inspection=dict(retained='Complete parent node codec, metadata, begin/abort, allocation, split/borrow/merge/root changes, checker and page IO; exact source delta inspected.',limits='No fresh published ISO/ARIES inspection, physical persistence or independent-human assessment')))
 p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T47-05.json';d=read_json(p)
 assets=[PUBLIC+'/'+name for name in ['include/refs.h','include/store.h','include/tree.h','refs.c','tests.c','tree_cases.c','CMakeLists.txt','expect_failure.py','CONTRACTS.md']]+['labs/storage/FORMAT.md']
 d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]));source=dict(id=resource['id'],path=BASE+'/references_resources.json',passages=locators);d['source_records']=[r for r in d['source_records'] if r['id']!=source['id']]+[source]
 binding=dict(path=BASE+'/references_completion_dispositions.json',criterion_ids=['AX-PR-T47-05-T47.05.b-DELAYED'],scope='Only persistent generation-tagged reference changed condition: actual CST5 engine, allocation retirement, exact moved pairs and unpublished-only rollback; ordinary ownership remains separately bound.')
 d['execution_support']['storage_references_successor']=binding
 for c in d['criteria']:
  if c['objective']=='T47.05.b':c['changed_condition_support']=binding
 dump(p,d);print(dict(attached='PR-T47-05',ordinary=0,delayed=1,baseline_observations='preserved'))
if __name__=='__main__':main()
