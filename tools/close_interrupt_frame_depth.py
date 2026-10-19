"""Attach and bind exact original interrupt_frame criteria; keep external evidence separate."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_advanced.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_advanced_frame';PROJECT='curriculum/v5/expansion/legacy_assessments/PR-T29-01.json';SUPPORT='research/v5/expansion/legacy_advanced_frame/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_advanced_frame/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
RATIONALE={
'01.a':'Bounded basic/extended frame decoder with unchanged output on errors, lazy FP validity and actual M4 frame checks.',
'01.b':'Actual entry shim/architectural frame versus compiler stack; complete declared direct-call/nested hardware ledger at changed optimization and FP callee.',
'01.c':'Actual nested A/B/C and QEMU tail-chain logs plus independent explicit late-arrival phase model and two discriminating rejected traces.',
'02.a':'Independent enable/pending/active/source and masks, actual disabled-pending/masked-enable target experiment and residual-pending transfer.',
'02.b':'12288 independent bit partition cells,41472 selection cells, actual priority-bit readback and grouping-dependent preemption changes.',
'02.c':'Opposite-controller urgency conversion and selected FreeRTOS11.2 threshold/grouping admission including eight-bit special case.'}
SELECTORS={
'01.a':['gcc-debug 1 execute','gcc-debug 2 execute','gcc-debug 3 execute','arm-none-eabi-gcc-O0-fp0-v1 execute','arm-none-eabi-gcc-O0-fp0-v2 execute','arm-none-eabi-gcc-O0-fp0-v3 execute'],
'01.b':['arm-none-eabi-gcc-O0-fp0-v0 stack ledger','arm-none-eabi-gcc-O3-fp1-v0 stack ledger','clang-O3-fp1-v0 stack ledger'],
'01.c':['late arrival model','arm-none-eabi-gcc-O0-fp0-v0 architectural oracle','clang-O3-fp1-v0 architectural oracle'],
'02.a':['gcc-debug 4 execute','gcc-debug 6 execute','arm-none-eabi-gcc-O0-fp0-v0 architectural oracle'],
'02.b':['gcc-debug 5 execute','gcc-debug independent oracle','clang-O3-fp1-v0 architectural oracle'],
'02.c':['gcc-debug independent oracle','clang-release independent oracle']}

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+PROJECT],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/PROJECT);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/interrupt_depth/frame').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]

 put(HERE/'attachment_PR-T29-01.json',dict(project=d['project'],domain='T29',key='interrupt_frame_priority_successor',supplied_artifacts=public,shared_contracts=['labs/interrupt_depth/frame/CONTRACTS.md'],source_register='research/v5/expansion/legacy_advanced_frame/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria'] if c['objective'].startswith('T29.0')],scope='Six T29.01/.02 ordinary and six unchanged delayed tasks; actual M4 exception/priority execution, source-bound stack ledgers and explicit phase model; later T29 and physical/external work separate'))
 x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/PROJECT,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/interrupt_depth/frame/CONTRACTS.md',learner_artifacts=['frame.c','target.c','target.S','criterion-indexed frame/priority/stack dossier'],assessment='assessor/interrupt_depth/frame/ANALYSIS.md#'+c['objective'][4:],source_register='research/v5/expansion/legacy_advanced_frame/resources.json') for c in d['criteria'] if c['objective'].startswith('T29.0')],remaining='Remaining T29.03+ and physical/source/external dimensions remain separate'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',scope='Exact Arm frame/priority and selected FreeRTOS source passages, not external review',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'source_access.json',HERE/'audit.json']},limitations=['Published ISO and physical evidence remain separate']))
 print('PASS six original frame criterion objects preserved and attached; additive attachment idempotent')
def package():
 from check_interrupt_frame_depth import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_interrupt_frame_package.py',ROOT/PROJECT,ROOT/'research/v5/expansion/legacy_advanced/author_support.py'}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json');rows=[];d=read(ROOT/PROJECT);assert d['criteria']==original()['criteria']
 for c in [v for v in d['criteria'] if v['objective'].startswith('T29.0')]:
  key=c['objective'][4:];v=c['variants'][0]
  labels=list(dict.fromkeys(['gcc-debug 0 execute','gcc-release 0 execute','clang-release 0 execute','gcc-debug independent oracle',*SELECTORS[key]]))
  for delayed in [False,True]:
   artifacts=['labs/interrupt_depth/frame/CONTRACTS.md','assessor/interrupt_depth/frame/ANALYSIS.md','research/v5/expansion/legacy_advanced_frame/objective_mapping.json']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/interrupt_depth/frame/ANALYSIS.md#'+key),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Published ISO source access remains separately registered.','Actual board integration must satisfy capture prerequisites; no physical interrupt latency or mid-stacking arrival observation inferred.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T29-01 six ordinary and six unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T29.03+ and selected physical target integration remain required.','Physical board, published ISO, independent learners/delayed and external review remain separate.'],summary=dict(criteria=6,delayed=6,checks=len(report['checks']))));print('PASS twelve exact frame author dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
