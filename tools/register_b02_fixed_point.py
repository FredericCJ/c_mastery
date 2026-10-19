"""Attach stable-arena variants and concrete exported support to original IDs."""
import json
from common import ROOT,dump,sha
BASE='research/v5/expansion/b02_followthrough';PUBLIC='labs/b02_followthrough/fixed_point';BRIEF='curriculum/v5/exercises/X-T17-002.md'
LOCATORS=['Objectives, prerequisites and environment','Q7 and signal units','Delayed Q11 width and resources','Exact scale16 legacy tasks','Verification, failure and mutations','Deliverables, rubric and publication']
def main():
 path=PUBLIC+'/CONTRACTS.md'
 record=dict(id='R-B02-FOLLOW-FIXED',title='Exact fixed-point accumulation and distinct scales',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T17','T04'],role='primary',reason='Exact Q7/Q11 and distinct scale16 range, tie, clipping, unit and propagated-error contracts.',prerequisites=['PR-T17-02','X-T17-002'],assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read full new core, tests, Fraction oracle and contract; actual B02 Q7; exact X-T17-002 and all six PR-T04-03 tasks',scope='; '.join(LOCATORS),limits='Exact bounded numeric contracts; no16-bit-int execution, published C17/C23 access or physical sensor claim.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'fixed_point_resources.json',dict(resources=[record]))
 passages={path:LOCATORS,'assessor/b02_followthrough/fixed_point/fixed_point.c':['Complete exact accumulation, changed rounding/scales and signal/error adapters'],'assessor/b02_depth/depth.c':['b2_q7 including original per-term mutation'],'curriculum/projects/PR-T04-03.md':['Contract, Inputs, Steps, Failure, Acceptance and Transfer'], 'fixtures/language/PR-T04-03.json':['Exact scale16 four pairs and clipping flags'],'labs/b02_followthrough/map/abi_helpers.c':['Explicit selected Arm aggregate helper bodies'],'labs/containers_depth/byte_helpers.c':['Explicit byte-copy and zero helper bodies']}
 dump(ROOT/BASE/'fixed_point_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in passages},passages=passages,resources=BASE+'/fixed_point_resources.json',inspection=dict(date='2026-10-18',scope='Only explicitly named sources/passages',limits='Finite source inspection, no broad library or hardware claim.')))
 p=ROOT/BRIEF;s=p.read_text(encoding='utf8');head,rest=s.split('```json\n',1);encoded,tail=rest.split('\n```',1);d=json.loads(encoded)
 assets=[p.relative_to(ROOT).as_posix() for p in (ROOT/PUBLIC).iterdir() if p.is_file() and p.name!='campaign.json']+['labs/b02_depth/include/depth.h','labs/b02_depth/src/depth.c','labs/b02_followthrough/map/abi_helpers.c','labs/containers_depth/byte_helpers.c']
 d['fixtures']=list(dict.fromkeys(d['fixtures']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]))
 d['source_records']=[r for r in d['source_records'] if r['id']!=record['id']]+[dict(id=record['id'],path=BASE+'/fixed_point_resources.json',passages=LOCATORS)]
 d['status']['required_followthrough']=dict(contract=path,readiness=BASE+'/fixed_point_readiness.json',report=BASE+'/fixed_point_validation.json',target_report=BASE+'/fixed_point_target_validation.json',export_report=BASE+'/fixed_point_export_validation.json',scope='Exact root/R1-R3, nearest-even immediate and Q11 delayed; separate six scale16 legacy tasks; author support only.')
 marker='\n## Required exact fixed-point follow-through\n'
 if marker in tail:tail=tail.split(marker,1)[0]
 tail+=marker+'\nComplete the supplied [Q7, Q11 and distinct scale16 contract](../../../labs/b02_followthrough/fixed_point/CONTRACTS.md), including its explicit companion build and original B02 provider snapshot. Original rubric and changed-condition tasks remain required; separate assessor implementations and author campaigns are not learner assessment.\n'
 p.write_text(head+'```json\n'+json.dumps(d,ensure_ascii=False,indent=2)+'\n```'+tail,encoding='utf8');print(dict(attached='X-T17-002',assets=len(assets)))
 legacy=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T04-03.json';old=json.loads(legacy.read_text(encoding='utf8'));binding=dict(path=BASE+'/fixed_point_completion_dispositions.json',scope='Six exact scale16 ordinary/delayed tasks; separate from Q7. Source-bound implementation, rational oracle, target and exported public support; independent learner/human/physical evidence remains separate.')
 old['successor_support']=binding
 old['supplied_artifacts']=list(dict.fromkeys(old['supplied_artifacts']+assets))
 old['shared_contracts']=list(dict.fromkeys(old['shared_contracts']+[path]))
 for criterion in old['criteria']:criterion['successor_support']={**binding,'criterion_ids':[criterion['id'],criterion['id']+'-DELAYED']}
 dump(legacy,old)

if __name__=='__main__':main()
