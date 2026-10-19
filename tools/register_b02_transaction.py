"""Attach counted-transaction variants and concrete exported support to original IDs."""
import json
from common import ROOT,dump,sha
BASE='research/v5/expansion/b02_followthrough';PUBLIC='labs/b02_followthrough/transaction';BRIEF='curriculum/v5/exercises/X-T21-002.md'
LOCATORS=['Objectives, prerequisites and environment','Three control-flow forms and original provider','Fallible cleanup state machine','Delayed external registration','Verification, deliverables and rubric']

def main():
 path=PUBLIC+'/CONTRACTS.md'
 record=dict(id='R-B02-FOLLOW-TRANSACTION',title='Acquisition styles and retained compensation',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T21'],role='primary',reason='Actual acquisition-prefix styles, separate primary/cleanup causes and dependent registration retention across failed compensation.',prerequisites=['PR-T21-01','X-T21-002'],assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read full new implementation, tests, list oracle and contract; actual b2_transaction; exact original exercise and withheld analysis',scope='; '.join(LOCATORS),limits='Selected synchronous retain-on-cleanup-failure model; no arbitrary external rollback, durable transaction or learner guarantee.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'transaction_resources.json',dict(resources=[record]))
 passages={path:LOCATORS,'assessor/b02_followthrough/transaction/transaction.c':['Complete three styles, begin, cleanup and close bodies'],'labs/b02_followthrough/transaction/tests.c':['Actual malloc-backed registration and all failure-prefix/recovery cases'],'assessor/b02_depth/depth.c':['b2_transaction and release-failed-slot mutation'],'labs/b02_followthrough/map/abi_helpers.c':['Explicit selected Arm aggregate helper bodies'],'labs/containers_depth/byte_helpers.c':['Explicit byte-copy and zero helper bodies']}
 dump(ROOT/BASE/'transaction_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in passages},passages=passages,resources=BASE+'/transaction_resources.json',inspection=dict(date='2026-10-18',scope='Only explicitly named sources/passages',limits='Finite source inspection, no broad library or hardware claim.')))
 p=ROOT/BRIEF;s=p.read_text(encoding='utf8');head,rest=s.split('```json\n',1);encoded,tail=rest.split('\n```',1);d=json.loads(encoded)
 assets=[p.relative_to(ROOT).as_posix() for p in (ROOT/PUBLIC).iterdir() if p.is_file() and p.name!='campaign.json']+['labs/b02_depth/include/depth.h','labs/b02_depth/src/depth.c','labs/b02_followthrough/map/abi_helpers.c','labs/containers_depth/byte_helpers.c']
 d['fixtures']=list(dict.fromkeys(d['fixtures']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]))
 d['source_records']=[r for r in d['source_records'] if r['id']!=record['id']]+[dict(id=record['id'],path=BASE+'/transaction_resources.json',passages=LOCATORS)]
 d['status']['required_followthrough']=dict(contract=path,readiness=BASE+'/transaction_readiness.json',report=BASE+'/transaction_validation.json',target_report=BASE+'/transaction_target_validation.json',export_report=BASE+'/transaction_export_validation.json',scope='Exact root/R1-R4, cleanup-failure immediate and external-registration compensation delayed; author support only.')
 marker='\n## Required compensation follow-through\n'
 if marker in tail:tail=tail.split(marker,1)[0]
 tail+=marker+'\nComplete the supplied [acquisition styles and retained compensation contract](../../../labs/b02_followthrough/transaction/CONTRACTS.md), including the explicit B02 provider, three source styles and actual allocated synthetic registry. Original rubric and changed-condition tasks remain required; separate assessor implementations and author campaigns are not learner assessment.\n'
 p.write_text(head+'```json\n'+json.dumps(d,ensure_ascii=False,indent=2)+'\n```'+tail,encoding='utf8');print(dict(attached='X-T21-002',assets=len(assets)))
if __name__=='__main__':main()
