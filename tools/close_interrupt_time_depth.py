"""Add exact T29.09–.10 successors without rewriting original assessment objects."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_advanced.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_advanced_time';SUPPORT='research/v5/expansion/legacy_advanced_time/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_advanced_time/';PROJECTS=['PR-T29-05'];REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={
'09.a':['gcc-debug composition reference execute','gcc-debug composition provenance execute','arm-none-eabi-gcc-O0 interval oracle'],
'09.b':['gcc-debug 1 execute','gcc-debug 2 execute','gcc-debug independent oracle','gcc-debug composition clock execute','clang tsan execute'],
'09.c':['gcc-debug 4 execute','gcc-debug 5 execute','clang-release independent oracle','clang-O3 execute'],
'10.a':['gcc-debug composition reference execute','gcc-debug composition queue execute','gcc-debug composition lease execute','target clock execute'],
'10.b':['arm-none-eabi-gcc-O0 interval oracle','clang-O3 interval oracle','physical capture structural controls','physical template unobserved rejection'],
'10.c':['clang-O3 closure','clang-O3 symbols','clang-O3 missing helper rejected','target interval execute']}
RATIONALE={
'09.a':'Separate event/entry/acquisition/service and loss ledgers, actual Xi/Xd/Dp adapters, independent-clock interval derivation in withheld analysis.',
'09.b':'Bounded coupled-carry proof, reduced-width ABA rejection, distinct actual struct cmx_cpf SC sequence protocol and retirement; native threads/TSan and actual interrupted staged writer.',
'09.c':'Checked outward calibration/event intervals, big-integer/set oracle, prescaler change and actual target large arithmetic using original T11 helper.',
'10.a':'Actual queue/lease providers, paused consumer, cancel/complete/stop and stale generation, retained-supervisor software-reset schedule and altered-reset analysis.',
'10.b':'Four actual MPS2 IRQ profiles with independent interval checks, qualified physical acquisition protocol and raw-file/uncertainty checker; no physical observation claimed.',
'10.c':'Explicit reasoning cases distinguish semantic/object/emulator/physical claims, actual original helper closure and omission rejection, board/revision/placement requalification rubric.'}


def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def project(p):return 'curriculum/v5/expansion/legacy_assessments/'+p+'.json'
def original(p):return json.loads(subprocess.check_output(['git','show','HEAD:'+project(p)],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/interrupt_depth/time').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public+=['labs/interrupt_depth/frame/target.ld']
 public+=['labs/interrupt_depth/'+folder+'/'+name for folder,name in [('frame','frame.h'),('handoff','handoff.h'),('lease','lease.h')]]
 public+=['labs/checkpoint_depth/include/'+name for name in ['checkpoint_firmware.h','checkpoint_depth.h']]
 public+=['labs/embedded_debug_depth/peripheral/peripheral.h','labs/machine_image/freestanding_depth/free.h']
 public+=['curriculum/guides/interrupt-time-board-protocol.md','fixtures/embedded/interrupt_time_capture.json','tools/check_interrupt_time_capture.py']
 mapping=[]
 for name in PROJECTS:
  d=read(ROOT/project(name));old=original(name);assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
  put(HERE/('attachment_'+name+'.json'),dict(project=name,domain='T29',key='interrupt_time_successor',supplied_artifacts=public,shared_contracts=['labs/interrupt_depth/time/CONTRACTS.md','curriculum/guides/interrupt-time-board-protocol.md'],source_register='research/v5/expansion/legacy_advanced_time/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Original coherence/event/uncertainty and controlled experiment tasks; actual author references; physical/external/learner dimensions separate'))
  x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/project(name),x)
  mapping.extend(dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/interrupt_depth/time/CONTRACTS.md',learner_artifacts=['time.c','integration.c','target.c','criterion-indexed coherence/calibration/schedule/evidence dossier'],assessment='assessor/interrupt_depth/time/ANALYSIS.md#'+c['objective'][4:],checks=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_advanced_time/resources.json') for c in d['criteria'])
 put(HERE/'objective_mapping.json',dict(criteria=mapping,remaining='T41–49 and recursive inventory remain required; physical/source/external dimensions are separate.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'source_access.json',HERE/'audit.json']},limitations=['Named primary passages only; no final ISO wording or physical target result inferred.','Actual QEMU32-bit counter is not64-bit hardware capture or physical latency; board qualification remains pending.']))
 print('PASS six original criteria preserved; additive attachments idempotent')
def package():
 from check_interrupt_time_depth import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_interrupt_time_package.py',ROOT/'research/v5/expansion/legacy_advanced/author_support.py',*[ROOT/project(p) for p in PROJECTS]}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json');rows=[]
 for resource in read(HERE/'resources.json')['resources']:
  for path,digest in resource['inspection']['source_sha256'].items():assert sha(ROOT/path)==digest,('inspected source changed',path)
 for name in PROJECTS:
  d=read(ROOT/project(name));assert d['criteria']==original(name)['criteria']
  for c in d['criteria']:
   key=c['objective'][4:];v=c['variants'][0];labels=list(dict.fromkeys(['gcc-debug 0 execute','clang-release 0 execute',*SELECTORS[key]]))
   for delayed in [False,True]:
    artifacts=['labs/interrupt_depth/time/CONTRACTS.md','assessor/interrupt_depth/time/ANALYSIS.md','research/v5/expansion/legacy_advanced_time/objective_mapping.json'];dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=name,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/interrupt_depth/time/ANALYSIS.md#'+key),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Physical qualification/capture, exact published ISO access, independent learners/delayed and external review remain separate.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T29-05 six ordinary and six unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T41–49 and recursive inventory remain required authoring work.','Physical clock/board qualification and capture, source gaps, external review and independent/delayed learner evidence remain separate.'],summary=dict(criteria=6,delayed=6,checks=len(report['checks']))));print('PASS12 exact time author dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
