"""Attach counted-result variants and concrete exported support to original IDs."""
import json
from common import ROOT,dump,sha
BASE='research/v5/expansion/b02_followthrough';PUBLIC='labs/b02_followthrough/result';BRIEF='curriculum/v5/exercises/X-T19-002.md'
LOCATORS=['Objectives, prerequisites and environment','Constructors and counted rendering','Owning bytes, move and take','Delayed wire version and old consumer','Diagnostics and enforcement','Verification, deliverables and assessment']

def main():
 path=PUBLIC+'/CONTRACTS.md'
 record=dict(id='R-B02-FOLLOW-RESULT',title='Counted tagged results, ownership and wire compatibility',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T19'],role='primary',reason='Actual result constructors, counted rendering, unique byte ownership, old-object version rejection and compiler diagnostic limits.',prerequisites=['PR-T19-02','X-T19-002'],assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read complete new implementation, old consumer, tests, independent oracle and contract; actual b2_render; exact original exercise and withheld analysis',scope='; '.join(LOCATORS),limits='Bounded valid-call source inspection; no arbitrary ABI, language, dangling-pointer or independent learner guarantee.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'result_resources.json',dict(resources=[record]))
 passages={path:LOCATORS,'assessor/b02_followthrough/result/result.c':['Complete constructors, ownership transfer/reset and versioned wire bodies'],'labs/b02_followthrough/result/old_consumer.c':['Complete frozen version1 consumer without new producer header'],'labs/b02_followthrough/result/switch.c':['Enum omission/default, complete positive and unsigned-tag switches'],'assessor/b02_depth/depth.c':['b2_render and original partial-publication mutation'],'labs/b02_followthrough/map/abi_helpers.c':['Explicit selected Arm aggregate helper bodies'],'labs/containers_depth/byte_helpers.c':['Explicit byte-copy and zero helper bodies']}
 dump(ROOT/BASE/'result_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in passages},passages=passages,resources=BASE+'/result_resources.json',inspection=dict(date='2026-10-18',scope='Only explicitly named sources/passages',limits='Finite source inspection, no broad library or hardware claim.')))
 p=ROOT/BRIEF;s=p.read_text(encoding='utf8');head,rest=s.split('```json\n',1);encoded,tail=rest.split('\n```',1);d=json.loads(encoded)
 assets=[p.relative_to(ROOT).as_posix() for p in (ROOT/PUBLIC).iterdir() if p.is_file() and p.name!='campaign.json']+['labs/b02_depth/include/depth.h','labs/b02_depth/src/depth.c','labs/b02_followthrough/map/abi_helpers.c','labs/containers_depth/byte_helpers.c']
 d['fixtures']=list(dict.fromkeys(d['fixtures']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]))
 d['source_records']=[r for r in d['source_records'] if r['id']!=record['id']]+[dict(id=record['id'],path=BASE+'/result_resources.json',passages=LOCATORS)]
 d['status']['required_followthrough']=dict(contract=path,readiness=BASE+'/result_readiness.json',report=BASE+'/result_validation.json',target_report=BASE+'/result_target_validation.json',export_report=BASE+'/result_export_validation.json',diagnostics_report=BASE+'/result_warnings_validation.json',scope='Exact root/R1-R3, owning-byte immediate and old-object version delayed; author support only.')
 marker='\n## Required counted-result follow-through\n'
 if marker in tail:tail=tail.split(marker,1)[0]
 tail+=marker+'\nComplete the supplied [counted result, ownership and wire contract](../../../labs/b02_followthrough/result/CONTRACTS.md), including the explicit B02 provider, independently compiled old consumer and diagnostic-policy experiment. Original rubric and changed-condition tasks remain required; separate assessor implementations and author campaigns are not learner assessment.\n'
 p.write_text(head+'```json\n'+json.dumps(d,ensure_ascii=False,indent=2)+'\n```'+tail,encoding='utf8');print(dict(attached='X-T19-002',assets=len(assets)))
if __name__=='__main__':main()
