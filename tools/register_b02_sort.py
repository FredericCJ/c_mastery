"""Attach stable-sort variants and concrete exported support to original IDs."""
import json
from common import ROOT,dump,sha
BASE='research/v5/expansion/b02_followthrough';PUBLIC='labs/b02_followthrough/sort';BRIEF='curriculum/v5/exercises/X-T15-002.md'
LOCATORS=['Original objectives and admission','Representations and operations','Reasoning and costs','Verification, failure and transfer','Deliverables, hints, rubric and export']
def main():
 path=PUBLIC+'/CONTRACTS.md'
 record=dict(id='R-B02-FOLLOW-SORT',title='Stable values, bottom-up passes and an owning-record permutation',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T15'],role='primary',reason='Concrete stable-order, failure, width, indirect ownership and traffic variants.',prerequisites=['PR-T15-01','X-T15-002'],assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read full new core, tests, independent oracle, contract; original B02 merge/b2_sort and ODS11.1.1 code and recurrence discussion',scope='; '.join(LOCATORS),limits='Exact bounded sort variants; no parent heap execution, universal performance or published C17/C23 access claim.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'sort_resources.json',dict(resources=[record]))
 passages={path:LOCATORS,'assessor/b02_followthrough/sort/sort.c':['Complete width, bottom-up and indirect implementation'],'assessor/b02_depth/depth.c':['LEFT_WINS, merge, b2_sort'],'research/v5/expansion/b02/b02_resources.json':['R-B02-ODS-MERGE: 11.1.1 Merge-Sort: merge code and recursion-tree analysis'],'labs/b02_followthrough/map/abi_helpers.c':['Explicit selected Arm aggregate helper bodies'],'labs/containers_depth/byte_helpers.c':['Explicit byte-copy and zero helper bodies']}
 dump(ROOT/BASE/'sort_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in passages},passages=passages,resources=BASE+'/sort_resources.json',inspection=dict(date='2026-10-18',scope='Only explicitly named sources/passages',limits='Finite source inspection, no broad library or hardware claim.')))
 p=ROOT/BRIEF;s=p.read_text(encoding='utf8');head,rest=s.split('```json\n',1);encoded,tail=rest.split('\n```',1);d=json.loads(encoded)
 assets=[p.relative_to(ROOT).as_posix() for p in (ROOT/PUBLIC).iterdir() if p.is_file() and p.name!='campaign.json']+['labs/b02_depth/include/depth.h','labs/b02_depth/src/depth.c','labs/b02_followthrough/map/abi_helpers.c','labs/containers_depth/byte_helpers.c']
 d['fixtures']=list(dict.fromkeys(d['fixtures']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]))
 d['source_records']=[r for r in d['source_records'] if r['id']!=record['id']]+[dict(id=record['id'],path=BASE+'/sort_resources.json',passages=LOCATORS)]
 d['status']['required_followthrough']=dict(contract=path,readiness=BASE+'/sort_readiness.json',report=BASE+'/sort_validation.json',target_report=BASE+'/sort_target_validation.json',export_report=BASE+'/sort_export_validation.json',scope='Exact root/R1-R2, bottom-up immediate and named indirect delayed variant; author support only.')
 marker='\n## Required stable-sort follow-through\n'
 if marker in tail:tail=tail.split(marker,1)[0]
 tail+=marker+'\nComplete the supplied [bottom-up and owning-record permutation contract](../../../labs/b02_followthrough/sort/CONTRACTS.md), including its explicit companion build and original B02 provider snapshot. Original rubric and changed-condition tasks remain required; separate assessor implementations and author campaigns are not learner assessment.\n'
 p.write_text(head+'```json\n'+json.dumps(d,ensure_ascii=False,indent=2)+'\n```'+tail,encoding='utf8');print(dict(attached='X-T15-002',assets=len(assets)))
if __name__=='__main__':main()
