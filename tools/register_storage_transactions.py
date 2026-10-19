"""Attach only the transaction/split obligations this successor actually supplies."""
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/legacy_storage_followthrough';PUBLIC='labs/storage_followthrough/transactions'
SELECTED={'PR-T47-04':['AX-PR-T47-04-T47.04.b'],'PR-T47-06':['AX-PR-T47-06-T47.06.'+c for c in 'abc']}
def main():
    path=PUBLIC+'/CONTRACTS.md';locators=['Original mechanism and an explicit verification port','Changed contract: operation rollback','One savepoint with finite identity','Required evidence and assessment','Verification and limitations']
    resource=dict(id='R-STORAGE-FOLLOW-TRANSACTIONS',title='Actual split-admission failure, operation undo and finite savepoint contract',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T47'],role='primary',reason='Explicit workspace ownership, changed rollback semantics and verification-port authority.',prerequisites=['PR-T47-01..06','PR-T47-07/09 for publication and recovery'],assignable=True,locators=locators,inspection=dict(date='2026-10-18',method='Read complete local contract and actual retained mutation/abort/commit implementation',scope='; '.join(locators),limits='Original author contract; not independent assessment or exact published C17.',local_sha256=sha(ROOT/path)))
    dump(ROOT/BASE/'transactions_resources.json',dict(resources=[resource]));dump(ROOT/BASE/'transactions_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={path:sha(ROOT/path),'assessor/storage/store.c':sha(ROOT/'assessor/storage/store.c')},resources=BASE+'/transactions_resources.json',inspection=dict(retained='alloc_node/insert/refresh/store_put/store_delete/store_begin/store_abort/store_commit; full actual typed representation',limits='No new complete external paper/book or normative ISO text inspection')))
    for project,ids in SELECTED.items():
        p=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');d=read_json(p)
        assets=[PUBLIC+'/'+name for name in ['transactions.h','transactions.c','fault_port.h','instrument_store.py','tests.c','CMakeLists.txt','expect_failure.py','CONTRACTS.md']]+['labs/storage/include/store.h','labs/storage/src/store.c','labs/storage/FORMAT.md']
        d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]));source=dict(id=resource['id'],path=BASE+'/transactions_resources.json',passages=locators);d['source_records']=[r for r in d['source_records'] if r['id']!=source['id']]+[source]
        binding=dict(path=BASE+'/transactions_completion_dispositions.json',criterion_ids=ids,scope='Actual C17 split failure/whole-abort comparison, operation rollback, one finite savepoint and no-steal/uncertain-commit boundaries. Only named ordinary tasks plus T47.04.b and T47.06.b delayed variants; remaining variants are distinct.')
        d['execution_support']['storage_transactions_successor']=binding
        for c in d['criteria']:
            if c['id'] in ids:c['successor_support']=dict(binding,criterion_ids=[c['id']])
        dump(p,d)
    print(dict(attached=list(SELECTED),ordinary=4,delayed=2,baseline_observations='preserved'))
if __name__=='__main__':main()
