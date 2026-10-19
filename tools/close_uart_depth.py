"""Canonical clock/UART attachment; preserve original criteria and delay."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_advanced.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_advanced_uart';PROJECT='curriculum/v5/expansion/legacy_assessments/PR-T28-02.json';SUPPORT='research/v5/expansion/legacy_advanced_uart/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_advanced_uart/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json','cost_analysis.json']]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+PROJECT],cwd=ROOT,text=True,encoding='utf8'))
RATIONALE={
'03.a':'Bounded source readiness, RCC gate/readback/reset sequence and implemented reset-during-DMA ownership recovery.',
'03.b':'Separate AHB/APB1/APB2/TIMPRE derivation with independently enumerated rational expectations and changed APB divider.',
'03.c':'Stale-generation rejection and quiescent reconfiguration, with implemented original-deadline wire-time feasibility after frequency changes.',
'05.a':'Actual serial provider framing/divisor reuse, independently checked nearest rounding and OVER8 encoding, preserved SR/DR status.',
'05.b':'Capacity-two binary receive/metadata, bounded TX/RX waits, drop counter and in-flight-reserve backpressure including actual software-ready GPIO mapping.',
'05.c':'Polling/IRQ/DMA service plus actual F439 stream3/channel4 adapter, explicit cancel/EN acknowledgment and independent memory/wire rights; compiled final-memory-read-before-TC cancellation.'}
def integrate():
 d=read(ROOT/PROJECT);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/peripheral_depth/uart').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public+=['labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/serial_depth/CONTRACTS.md','labs/serial_depth/README.md','labs/firmware_io/include/firmware_io.h','labs/firmware_io/CONTRACTS.md','labs/firmware_io/README.md']
 put(HERE/'attachment_PR-T28-02.json',dict(project=d['project'],domain='T28',key='clock_uart_successor',supplied_artifacts=public,shared_contracts=['labs/peripheral_depth/uart/CONTRACTS.md','labs/serial_depth/CONTRACTS.md','labs/firmware_io/CONTRACTS.md'],source_register='research/v5/expansion/legacy_advanced_uart/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Only six T28.03/.05 ordinary and six delayed tasks; actual serial/FW providers; physical, independent and remaining T28 scope separate'))
 x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/PROJECT,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/peripheral_depth/uart/CONTRACTS.md',learner_artifacts=['uart.c','target.c','criterion-indexed dossier'],assessment='assessor/peripheral_depth/uart/ANALYSIS.md#'+c['objective'][4:],source_register='research/v5/expansion/legacy_advanced_uart/resources.json') for c in d['criteria']],remaining='T28.06–.12 and T29 remain required; physical/source/external dimensions separate'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',scope='Exact T28 clock/UART passages, not external review',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'source_access.json',HERE/'audit.md']},limitations=['Published ISO and physical evidence remain separate']))
 print('PASS original six criterion objects preserved; additive attachment idempotent')
def package():
 from check_uart_depth import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_uart_package.py',ROOT/PROJECT,ROOT/'research/v5/expansion/legacy_advanced/author_support.py'}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json');rows=[];d=read(ROOT/PROJECT);assert d['criteria']==original()['criteria']
 for c in d['criteria']:
  key=c['objective'][4:];v=c['variants'][0];mutants={'03.a':[2],'03.b':[1],'03.c':[5],'05.a':[3],'05.b':[4],'05.c':[6,7]}[key]
  labels=['gcc-debug execute 0','gcc-release execute 0','clang-release execute 0','gcc-debug independent oracle',*['gcc-debug execute '+str(n) for n in mutants]]
  if key in ['03.a','05.a','05.b','05.c']:labels+=['gcc-debug target execute 0','gcc-debug target execute 1','gcc-debug target execute 2','arm-none-eabi-gcc-O3 combined audit link','clang-O3 target disassembly']
  if key=='05.c':labels+=['gcc-debug composition execute']
  for delayed in [False,True]:
   artifacts=['labs/peripheral_depth/uart/CONTRACTS.md','assessor/peripheral_depth/uart/ANALYSIS.md','research/v5/expansion/legacy_advanced_uart/objective_mapping.json','labs/peripheral_depth/uart/BOARD_CAPTURE.md']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/peripheral_depth/uart/ANALYSIS.md#'+key),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Published ISO source access remains separately registered.','Actual ISR/board integration must satisfy the T29/capture prerequisites; no observed latency or physical execution inferred.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T28-02 six ordinary and six unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T28.06–.12 and T29 remain required authoring/implementation work.','Physical board, published ISO, independent learners/delayed and external review remain separate.'],summary=dict(criteria=6,delayed=6,checks=len(report['checks']))));print('PASS twelve exact UART author dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
