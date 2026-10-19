"""Attach exact original B01 companion without changing accepted identities."""
import json
from common import ROOT,dump,sha
BASE='research/v5/expansion/b01_followthrough'
IDS=['X-T02-001','X-T03-001','X-T03-002','X-T03-003','X-T03-004','X-T03-005','X-T04-001','X-T35-001']
PUBLIC='labs/foundation_followthrough'
LOCATORS=['Interval construction: X-T04-001-R3 and original interval variant','Source character copying: X-T03-003-R2','Symmetric codec defect and object observations: X-T03-002','Cursor algebra and pool histories: X-T03-001/X-T03-005','Semantic and code-generation comparisons','Independent local state oracle: X-T35-001','Deliverables, assessment and hints']

def main():
    contract=PUBLIC+'/CONTRACTS.md'
    record=dict(id='R-B01-FOLLOW-CONTRACT',title='Original B01 verification completion',url=contract,authors=['Curriculum authors'],edition='Original required companion 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T02','T03','T04','T35'],role='primary',reason='Concrete completion of original interval, typed-copy, paired-codec, interleaved-history and counter-oracle obligations.',prerequisites=IDS,assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read complete original briefs, public/private provider, transfer assignments/reference/tests/analysis, companion and actual draft paragraph.',scope='; '.join(LOCATORS),limits='Bounded author reading; exact published C17/C23 unavailable; no learner or physical evidence.',local_sha256=sha(ROOT/contract)))
    dump(ROOT/BASE/'resources.json',dict(resources=[record]))
    passages={contract:LOCATORS,'assessor/foundation_followthrough/follow.c':['Complete interval, typed-source copying and codec adapters'],'assessor/foundation_followthrough/ANALYSIS.md':['Complete technical analysis and limits'],'assessor/foundation_depth/depth.c':['Complete unchanged provider'],'assessor/foundation_depth/transfer/reference.c':['Step,layout,owner/cursor,least codec,typed storage,move,stride,registry,query,frame'],'assessor/foundation_depth/transfer/tests.c':['All eight test functions;three distinct registry owner lifetimes'],'assessor/foundation_depth/transfer/ASSIGNMENTS.md':['All prerequisite and withheld changed-condition assignments'],'assessor/foundation_depth/transfer/SEMANTIC_CASES.md':['Four prompts and separate analysis'],'research/v5/expansion/b01/b01_resources.json':['Existing exact registered source locators; no new whole-document claim']}
    passages['assessor/foundation_followthrough/REASONING.md']=['Exact ordinary and level3 changed-premise arguments for all38original rubric criteria']
    dump(ROOT/BASE/'research_gate.json',dict(status='passage-inspected',research_input_sha256={BASE+'/resources.json':sha(ROOT/BASE/'resources.json')},resources=BASE+'/resources.json',input_sha256={p:sha(ROOT/p) for p in passages},passages=passages,external_source_sha256={'build/portability-sources/n1570.pdf':sha(ROOT/'build/portability-sources/n1570.pdf')},inspection=dict(date='2026-10-18',method='pymupdf actual PDF page95 printed77,paragraph6 and adjacent paragraph7;bounded source-code reading',scope='N1570 6.5p6-7 effective type and character access; reuses original registered authority for other claims',limits='Draft identity preserved; exact C17/C23 access gaps remain.')))
    assets=[p.relative_to(ROOT).as_posix() for p in (ROOT/PUBLIC).iterdir() if p.is_file()]+['labs/foundation_depth/include/depth.h','labs/foundation_depth/src/depth.c','labs/smp_depth/acpi/big_endian.S']
    for id in IDS:
        p=ROOT/'curriculum/v5/exercises'/f'{id}.md';s=p.read_text(encoding='utf8');head,rest=s.split('```json\n',1);encoded,tail=rest.split('\n```',1);d=json.loads(encoded)
        d['fixtures']=list(dict.fromkeys(d['fixtures']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[contract]))
        d['source_records']=[r for r in d['source_records'] if r['id']!=record['id']]+[dict(id=record['id'],path=BASE+'/resources.json',passages=LOCATORS)]
        d['status']['required_followthrough']=dict(contract=contract,readiness=BASE+'/readiness.json',report=BASE+'/validation.json',transfer_report='reviews/v5/expansion/b01_followthrough/foundation_transfer_successor.json',export_report=BASE+'/export_validation.json',scope='Exact original root, rubric and changed-condition support. External and independent learner assessment remain separate.')
        marker='\n## Required original verification follow-through\n'
        if marker in tail:tail=tail.split(marker,1)[0]
        tail+=marker+'\nComplete the applicable [original verification companion](../../../labs/foundation_followthrough/CONTRACTS.md), retaining the original foundation gate and withheld transfer protocol. The companion supplies missing interval/typed-copy construction, symmetric codec falsification, independent histories and local counter-oracle support. Author reference execution does not award learner mastery.\n'
        p.write_text(head+'```json\n'+json.dumps(d,ensure_ascii=False,indent=2)+'\n```'+tail,encoding='utf8')
    print(dict(attached=IDS,public_assets=len(assets)))
if __name__=='__main__':main()
