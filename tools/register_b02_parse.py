"""Attach the required parser variants without replacing the original B02 gate."""
import json
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/b02_followthrough';PUBLIC='labs/b02_followthrough/parse';BRIEF='curriculum/v5/exercises/X-T12-002.md'
LOCATORS=['Original obligations and representation','Grammar, operations and failures','Hosted comparison and exact source boundary','Evidence and assessment','Exported stages']

def main():
 path=PUBLIC+'/CONTRACTS.md'
 record=dict(id='R-B02-FOLLOW-PARSE',title='Counted integer parsing: radix, streaming and hosted adaptation',url=path,authors=['Curriculum authors'],edition='Original repository contract; inspected 2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T12'],role='primary',reason='Required canonical-radix, chunked finish/reset and actual hosted conversion comparison.',prerequisites=['X-T04-001','X-T12-002'],assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read complete contract, core, host adapter, tests and independent oracle; inspected N1570 7.22.1.4 paragraphs1-8',scope='; '.join(LOCATORS),limits='Bounded original mechanisms and C11 draft wording; exact published C17/C23 access remains a gap.',local_sha256=sha(ROOT/path)))
 dump(ROOT/BASE/'parse_resources.json',dict(resources=[record]))
 passages={path:LOCATORS,'assessor/b02_followthrough/parse/parse.c':['Complete core'],'assessor/b02_followthrough/parse/host.c':['Complete hosted adapter'],'assessor/b02_depth/depth.c':['b2_parse'],'research/v5/expansion/b02/b02_resources.json':['R-B02-N1570-CONTRACTS; 7.22.1.4p1–8 strto conversion']}
 dump(ROOT/BASE/'parse_research_gate.json',dict(status='passage-inspected-author-only',input_sha256={p:sha(ROOT/p) for p in passages},passages=passages,resources=BASE+'/parse_resources.json',inspection=dict(date='2026-10-18',scope='Only the named actual passages and implementations',limits='No exact published C17/C23 or whole-library claim.')))
 p=ROOT/BRIEF;s=p.read_text(encoding='utf8');head,rest=s.split('```json\n',1);encoded,tail=rest.split('\n```',1);d=json.loads(encoded)
 assets=[p.relative_to(ROOT).as_posix() for p in (ROOT/PUBLIC).iterdir() if p.is_file() and p.name!='campaign.json']+['labs/b02_depth/include/depth.h','labs/b02_depth/src/depth.c']
 d['fixtures']=list(dict.fromkeys(d['fixtures']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[path]))
 d['source_records']=[r for r in d['source_records'] if r['id']!=record['id']]+[dict(id=record['id'],path=BASE+'/parse_resources.json',passages=LOCATORS)]
 d['status']['required_followthrough']=dict(contract=path,readiness=BASE+'/parse_readiness.json',report=BASE+'/parse_validation.json',target_report=BASE+'/parse_target_validation.json',export_report=BASE+'/parse_export_validation.json',scope='Original root and R1-R3 plus canonical-radix immediate variant and X-T12-002-DELAYED; author support only.')
 marker='\n## Required parser follow-through\n'
 if marker in tail:tail=tail.split(marker,1)[0]
 tail+=marker+'\nComplete the supplied [radix, streaming and hosted-adapter contract](../../../labs/b02_followthrough/parse/CONTRACTS.md), including its explicit companion build and original B02 provider snapshot. This supplies concrete scaffolds, independent finite oracles and the named delayed mechanism while retaining the original lab gate. Separate assessor analysis and implementations are excluded from learner exports. Authored solutions and executed author campaigns do not establish learner performance.\n'
 p.write_text(head+'```json\n'+json.dumps(d,ensure_ascii=False,indent=2)+'\n```'+tail,encoding='utf8')
 print(dict(attached='X-T12-002',assets=len(assets),original_lab=d['lab']['family']))
if __name__=='__main__':main()
