"""Attach only the original queued-I/O changed-condition obligation."""
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/legacy_storage_followthrough';PUBLIC='labs/storage_followthrough/async';ID='AX-PR-T47-12-T47.12.b-DELAYED'
LOCATORS=['Exact obligation and prerequisites','Representation, operations and authority','Actual CMK2 boundary and local containment','Workload, falsification and resource argument','Deliverables, hints and assessment']
def main():
 path=PUBLIC+'/CONTRACTS.md'
 resource=dict(id='R-STORAGE-FOLLOW-ASYNC',title='Asynchronous read abandonment and completion authority',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T47'],role='primary',reason='Actual queued read composition, separate abandonment and buffer ownership, explicit unknown-outcome quarantine.',prerequisites=['PR-T47-01','PR-T47-02','PR-T47-12','PR-T48-09','CMK2 block ABI'],assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read complete new contract and core; complete original CMK2 async.inc, ABI and user_v2; inspected guest build/command/test implementation',scope='; '.join(LOCATORS),limits='Selected original source and adapter only; no device timeout/quiescence or physical observation claim.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'async_resources.json',dict(resources=[resource]))
 passages={path:LOCATORS,'assessor/storage_followthrough/async/async.c':['Complete core'],'labs/storage_followthrough/async/guest.c':['Complete read adapter and workload'],'assessor/kernel_smp/async.inc':['Complete queued block implementation'],'labs/kernel_smp/include/cmk2_abi.h':['Complete syscall ABI'],'labs/kernel_smp/tests/user_v2.c':['Complete original user workload preserved in composition'],'labs/kernel_smp/tools/guest.py':['build','command','test']}
 dump(ROOT/BASE/'async_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in passages},passages=passages,resources=BASE+'/async_resources.json',inspection=dict(scope='Only explicitly listed inspected passages',limits='No exact published ISO C17/C23 access, whole OS audit or physical observation inferred.')))
 assets=[p.relative_to(ROOT).as_posix() for p in (ROOT/PUBLIC).iterdir() if p.is_file() and p.name!='campaign.json']
 assets += [p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/kernel_smp').rglob('*') if p.is_file() and not any(x in p.parts for x in ['__pycache__','build','.cache'])]
 p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T47-12.json';d=read_json(p);d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]));d['source_records']=[r for r in d['source_records'] if r['id']!=resource['id']]+[dict(id=resource['id'],path=BASE+'/async_resources.json',passages=LOCATORS)]
 binding=dict(path=BASE+'/async_completion_dispositions.json',criterion_ids=[ID],scope='Actual queued CMK2 read with retained borrow through positive completion; native uncertain-result quarantine and no invented cancellation acknowledgement.')
 d['execution_support']['storage_async_successor']=binding
 for c in d['criteria']:
  if c['id']+'-DELAYED'==ID:c['async_support']=binding
 dump(p,d);print(dict(attached=['PR-T47-12'],exact_ids=1,historical_baseline='preserved'))
if __name__=='__main__':main()
