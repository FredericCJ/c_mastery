"""Close implemented children, retaining RM0440 and physical evidence explicitly."""
from pathlib import Path
import json
from run_g474_uart_followthrough import ROOT,sha,REPORT
from check_g474_uart_followthrough import validate
HERE=ROOT/'research/v5/expansion/legacy_advanced_g474_uart';REVIEW='reviews/v5/expansion/legacy_g474_uart/'
def main():
 reports=[REPORT,REVIEW+'support_validation.json',REVIEW+'exports.json'];d=json.loads((ROOT/REPORT).read_text());assert not validate(d)
 files={Path(__file__),ROOT/'tools/check_g474_uart_package.py',ROOT/'research/v5/expansion/legacy_advanced/author_support.py'}
 for name in reports:
  r=json.loads((ROOT/name).read_text());assert r['passed'];files.add(ROOT/name)
  for p,h in r.get('input_sha256',{}).items():assert sha(ROOT/p)==h,(name,p);files.add(ROOT/p)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json')
 for r in json.loads((HERE/'resources.json').read_text())['resources']:
  for p,h in r['inspection'].get('source_sha256',{}).items():assert sha(ROOT/p)==h;files.add(ROOT/p)
 artifacts=['labs/g474_uart_followthrough/CONTRACTS.md','assessor/g474_uart_followthrough/ANALYSIS.md','assessor/g474_uart_followthrough/DELAYED.md','labs/g474_uart_followthrough/BOARD_CAPTURE.md'];rows=[]
 def dims(implementation='complete',author='passed'):
  return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation',implementation,artifacts),('author_validation',author,reports),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
 for suffix in ['-IMPLEMENTATION','-IMPLEMENTATION-DELAYED']:
  rows.append(dict(id='G-PL-G474-FULL-PORT'+suffix,parent_ids=['G-PL-G474-FULL-PORT'],objective='T28.11.c',parent_project='PR-T28-06',criterion='AX-PR-T28-06-T28.11.c',dimensions=dims(),artifacts=artifacts,closure_rationale='Original initializer, actual imported byte provider,212 bounded cases/profile, independent divisor oracle, compiling semantic controls,12 strict image/object audits and complete capture/delayed package.',author_validation=dict(report=REPORT,command_groups=['gcc-debug:model','clang-asan:model','gcc-debug:mutant-1','gcc-debug:mutant-2','gcc-debug:mutant-3','gcc-debug:mutant-4','baud','capture','board:clang-O3-115200']),minimum_days=7 if suffix.endswith('DELAYED') else 0,remaining_author_work=[],external_blockers=[dict(dimension='independent_learner',missing='Independent attempt and at least seven-day delayed transfer'),dict(dimension='external_review',missing='Non-author technical review')],evidence_limits=d['limits']))
 rows.append(dict(id='G-PL-G474-FULL-PORT',parent_ids=['G-PA-RM0440-FULL'],dimensions=dims('partial','partial'),artifacts=artifacts,closure_rationale='Split mixed obligation: source-supported selected original implementation completed; full-manual inspection and actual board evidence remain open.',completed_children=[r['id'] for r in rows],remaining_author_work=['Inspect newly accessible authorized RM0440 passages and reconcile selected contract when available'],external_blockers=[dict(dimension='source_access',missing='Full RM0440 PDF readable by available tools; exact failed attempts in source_access.json'),dict(dimension='physical_observation',missing='Identified dedicated board, qualified instrument/receiver and actual route/clock/wire captures')],evidence_limits=d['limits']))
 out=dict(schema_version=1,scope='Selected G474 original initializer and capture implementation; no full RM0440 or physical acceptance',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=reports,updates=rows,remaining_parents=['G-PA-RM0440-FULL','G-PL-G474-FULL-PORT-MANUAL','G-PL-G474-FULL-PORT-PHYSICAL']);(HERE/'completion_dispositions.json').write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n',encoding='utf8');print('PASS two implemented children; mixed parent remains partial')
if __name__=='__main__':main()
