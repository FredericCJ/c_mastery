"""Attach stable-arena variants and concrete exported support to original IDs."""
import json
from common import ROOT,dump,sha
BASE='research/v5/expansion/b02_followthrough';PUBLIC='labs/b02_followthrough/arena';BRIEF='curriculum/v5/exercises/X-T16-002.md'
LOCATORS=['Original objectives and admission','Representations and operations','Reasoning and costs','Verification, failure and transfer','Deliverables, hints, rubric and export']
def main():
 path=PUBLIC+'/CONTRACTS.md'
 record=dict(id='R-B02-FOLLOW-ARENA',title='Nested byte scopes, retained cleanup and checked borrows',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T16'],role='primary',reason='Concrete mark, byte authority, fallible cleanup retry, reentry and nested commit variants.',prerequisites=['PR-T16-02','X-T16-002'],assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read full new core, tests, independent oracle and contract; original B02 reserve/mark/rollback and retained struct al_arena pop/commit',scope='; '.join(LOCATORS),limits='Exact bounded byte scopes and cleanup variants; no arbitrary typed-storage, raw-pointer revocation, published C17/C23 access or physical claim.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'arena_resources.json',dict(resources=[record]))
 passages={path:LOCATORS,'assessor/b02_followthrough/arena/arena.c':['Complete checked borrow, retained cleanup and inner commit implementation'],'assessor/b02_depth/depth.c':['b2_arena_init, b2_reserve, b2_mark, b2_rollback'],'assessor/allocation_depth/arena.c':['pop and al_arena_commit: contrasting reclaim-before-callback failure policy'],'labs/b02_followthrough/map/abi_helpers.c':['Explicit selected Arm aggregate helper bodies'],'labs/containers_depth/byte_helpers.c':['Explicit byte-copy and zero helper bodies']}
 dump(ROOT/BASE/'arena_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in passages},passages=passages,resources=BASE+'/arena_resources.json',inspection=dict(date='2026-10-18',scope='Only explicitly named sources/passages',limits='Finite source inspection, no broad library or hardware claim.')))
 p=ROOT/BRIEF;s=p.read_text(encoding='utf8');head,rest=s.split('```json\n',1);encoded,tail=rest.split('\n```',1);d=json.loads(encoded)
 assets=[p.relative_to(ROOT).as_posix() for p in (ROOT/PUBLIC).iterdir() if p.is_file() and p.name!='campaign.json']+['labs/b02_depth/include/depth.h','labs/b02_depth/src/depth.c','labs/b02_followthrough/map/abi_helpers.c','labs/containers_depth/byte_helpers.c']
 d['fixtures']=list(dict.fromkeys(d['fixtures']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]))
 d['source_records']=[r for r in d['source_records'] if r['id']!=record['id']]+[dict(id=record['id'],path=BASE+'/arena_resources.json',passages=LOCATORS)]
 d['status']['required_followthrough']=dict(contract=path,readiness=BASE+'/arena_readiness.json',report=BASE+'/arena_validation.json',target_report=BASE+'/arena_target_validation.json',export_report=BASE+'/arena_export_validation.json',scope='Exact root/R1-R3, reverse-cleanup immediate and inner-commit delayed variant; author support only.')
 marker='\n## Required nested-arena follow-through\n'
 if marker in tail:tail=tail.split(marker,1)[0]
 tail+=marker+'\nComplete the supplied [nested byte scope and retained cleanup contract](../../../labs/b02_followthrough/arena/CONTRACTS.md), including its explicit companion build and original B02 provider snapshot. Original rubric and changed-condition tasks remain required; separate assessor implementations and author campaigns are not learner assessment.\n'
 p.write_text(head+'```json\n'+json.dumps(d,ensure_ascii=False,indent=2)+'\n```'+tail,encoding='utf8');print(dict(attached='X-T16-002',assets=len(assets)))
if __name__=='__main__':main()
