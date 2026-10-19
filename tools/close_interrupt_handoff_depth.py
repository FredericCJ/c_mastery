"""Add exact T29.03–.06 successors without rewriting original assessment objects."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_advanced.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_advanced_handoff';SUPPORT='research/v5/expansion/legacy_advanced_handoff/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_advanced_handoff/';PROJECTS=['PR-T29-02','PR-T29-03'];REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','supplemental_validation.json','exports.json']]
SELECTORS={
'03.a':['gcc-debug 0 execute','arm-none-eabi-gcc-O0-i1-v0 object audit','clang-O3-i1-v0 object audit'],
'03.b':['arm-none-eabi-gcc-O0-i1-v0 execute','gcc tsan threads execute','interference model'],
'03.c':['gcc-debug 3 execute','gcc-debug independent oracle','clang-release independent oracle'],
'04.a':['gcc-debug locks execute','clang tsan locks execute','interference model'],
'04.b':['arm-none-eabi-gcc-O0-i1-v1 execute','arm-none-eabi-gcc-O0-i1-v2 execute','clang-O3-i1-v0 execute','gcc-debug independent oracle'],
'04.c':['arm-none-eabi-gcc-O0-i1-v0 execute','interference model'],
'05.a':['GenMC 0','GenMC 7','GenMC 8','gcc tsan threads execute','interference model'],
'05.b':['arm-none-eabi-gcc widened imports','clang widened store imports','arm-none-eabi-gcc widened link rejection','clang widened link rejection','clang-release wide host execute','clang-O3-i1-v0 object audit'],
'05.c':['gcc-debug 1 execute','GenMC 1','gcc-debug DMA 0 execute','gcc-debug DMA 6 execute'],
'06.a':['gcc-debug independent oracle','gcc-debug full capacity execute','clang-release full capacity execute'],
'06.b':['gcc-debug 2 execute','gcc-asan 0 execute','clang-release 0 execute','GenMC 8'],
'06.c':['arm-none-eabi-gcc-O0-i1-v0 interval oracle','clang-O3-i1-v0 interval oracle','instrumentation comparison']}
RATIONALE={
'03.a':'Bounded acknowledged capture, owned variable-length prefix and actual complete generated call graph/runtime closure.',
'03.b':'Actual controlled nested busy rejection with serialized begin; actual two-source coherent hosted composition and explicit second-core limitations.',
'03.c':'N-1/drop-newest exact burst and saturation, separately serialized retain-newest implementation and independent deque oracle.',
'04.a':'Defined participant/mask/scalar-pair counterexamples, actual cooperating mutex and dual-core/cache ownership analysis.',
'04.b':'Actual PRIMASK save/restore and BASEPRI_MAX monotonic nesting,4096triples and compiled semantic controls.',
'04.c':'Actual higher-urgency partial-pair observation, explicit DMA/core interference model and ownership repair.',
'05.a':'Actual phase-split queue publication/reuse, bounded RC11 exploration and per-source queue transfer.',
'05.b':'Actual word-index target closure, real widened-index host execution and forbidden target helper rejection; distinct serialized original wide counter.',
'05.c':'Premature publication and missing release controls, actual Pd finalization/post-invalidation composition and discriminating stale-data control.',
'06.a':'Exact ordinary queue history and independent deque histories; unchanged actual CMX full-capacity provider across modular wrap.',
'06.b':'Multiword unpublished/held slot pauses, changed compiler/copy lowering, reuse race control and object evidence.',
'06.c':'Actual virtual-timer publication intervals,64survivors/16drops, equal-average burst transfer and four instrumentation comparisons.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def project(p):return 'curriculum/v5/expansion/legacy_assessments/'+p+'.json'
def original(p):return json.loads(subprocess.check_output(['git','show','HEAD:'+project(p)],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/interrupt_depth/handoff').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public+=['labs/interrupt_depth/frame/'+n for n in ['frame.h','frame.c','target.ld','CONTRACTS.md']]
 public+=['labs/execution/include/cmx.h','labs/execution/src/concurrent.c','labs/peripheral_depth/dma/dma.h','labs/peripheral_depth/dma/CONTRACTS.md','curriculum/guides/interrupt-handoff-board-protocol.md','fixtures/embedded/interrupt_handoff_capture.json','tools/check_interrupt_handoff_capture.py']
 mapping=[]
 for name in PROJECTS:
  d=read(ROOT/project(name));old=original(name);assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
  put(HERE/('attachment_'+name+'.json'),dict(project=name,domain='T29',key='interrupt_handoff_successor',supplied_artifacts=public,shared_contracts=['labs/interrupt_depth/handoff/CONTRACTS.md','curriculum/guides/interrupt-handoff-board-protocol.md'],source_register='research/v5/expansion/legacy_advanced_handoff/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Original ordinary and unchanged delayed capture/mask/publication/capacity tasks, actual author references; physical/external/learner dimensions remain separate'))
  x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/project(name),x)
  mapping.extend(dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/interrupt_depth/handoff/CONTRACTS.md',learner_artifacts=['handoff.c','port.c','wide.c','target.c','target.S','criterion-indexed ownership/mask/dependency/latency dossier'],assessment='assessor/interrupt_depth/handoff/ANALYSIS.md#'+c['objective'][4:],checks=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_advanced_handoff/resources.json') for c in d['criteria'])
 put(HERE/'objective_mapping.json',dict(criteria=mapping,remaining='T29.07–.10 remain required; physical/source/external dimensions are separate.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'source_access.json',HERE/'audit.json']},limitations=['Named passages only; N1570 is a C11 draft, not final C17.','QEMU time/source is not a physical board specification.']))
 print('PASS twelve original criteria preserved; additive attachments idempotent')
def package():
 from check_interrupt_handoff_depth import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_interrupt_handoff_package.py',ROOT/'research/v5/expansion/legacy_advanced/author_support.py',*[ROOT/project(p) for p in PROJECTS]}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json');rows=[]
 for name in PROJECTS:
  d=read(ROOT/project(name));assert d['criteria']==original(name)['criteria']
  for c in d['criteria']:
   key=c['objective'][4:];v=c['variants'][0];labels=list(dict.fromkeys(['gcc-debug 0 execute','clang-release 0 execute',*SELECTORS[key]]))
   for delayed in [False,True]:
    artifacts=['labs/interrupt_depth/handoff/CONTRACTS.md','assessor/interrupt_depth/handoff/ANALYSIS.md','research/v5/expansion/legacy_advanced_handoff/objective_mapping.json'];dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=name,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/interrupt_depth/handoff/ANALYSIS.md#'+key),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Physical qualification/capture, exact published ISO access, independent learners/delayed and external review remain separate.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T29-02/.03 twelve ordinary and twelve unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T29.07–.10 remain required authoring work.','Physical board qualification/capture, source gaps, external review and independent/delayed learner evidence remain separate.'],summary=dict(criteria=12,delayed=12,checks=len(report['checks']))));print('PASS24 exact handoff author dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
