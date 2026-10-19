"""Add exact T29.07–.08 successors without rewriting original assessment objects."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_advanced.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_advanced_lease';SUPPORT='research/v5/expansion/legacy_advanced_lease/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_advanced_lease/';PROJECTS=['PR-T29-04'];REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={
'07.a':['gcc-debug 0 execute','gcc-debug independent oracle','gcc-debug composition cancel-policy execute'],
'07.b':['gcc-debug composition reference execute','gcc-debug composition stale-cache execute','gcc-debug composition dirty-neighbour execute','visibility counterexamples'],
'07.c':['gcc-debug 2 execute','gcc-debug 5 execute','gcc-debug composition fabricated-fence execute','clang-release independent oracle'],
'08.a':['visibility counterexamples','arm-none-eabi-gcc-O0 object and interval audit','clang-O3 object and interval audit'],
'08.b':['arm-none-eabi-gcc-O0 F439 build','clang-O3 H745 build','wrong cache register object rejection'],
'08.c':['gcc-debug composition peer-generation execute','gcc-debug composition reference execute','gcc-debug composition dirty-neighbour execute']}
RATIONALE={
'07.a':'Exactly-once terminal selection and stop-first transfer, actual Pd cancellation-policy adapter and independent event-word oracle.',
'07.b':'Actual Pd/cache providers, source/destination cleaning, pending FIFO writes, speculative refill and post-invalidation; independent visibility counterexamples.',
'07.c':'Generation7/8 and epoch separation, finite exhaustion and separate retained engine/supervisor software-reset handshake; fabricated-fence control.',
'08.a':'Distinct compiler/ISA/cache/mask counterexamples, actual selected F439 port composition, complete instruction definitions and four-profile paired duplicate-DSB investigation.',
'08.b':'Actual F439 and H745 generated port closures; cache-register effective-address audits and compiled wrong-register rejection; precise inspected architecture/SoC passages.',
'08.c':'Actual cache-range and peer providers, retained failed-peer outstanding work, cacheless/WB/shared comparisons and dirty-neighbour isolation restriction.'}

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def project(p):return 'curriculum/v5/expansion/legacy_assessments/'+p+'.json'
def original(p):return json.loads(subprocess.check_output(['git','show','HEAD:'+project(p)],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/interrupt_depth/lease').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public+=['labs/interrupt_depth/frame/target.ld','labs/interrupt_depth/handoff/target.S']
 public+=['labs/peripheral_depth/dma/'+n for n in ['dma.h','dma.c','target.c','CONTRACTS.md']]
 public+=['labs/firmware_lifecycle/targets/'+n for n in ['h745_cache.h','h745_cache.c','h745_port.c','H745_CACHE.md','H745_ERRATA.md']]
 public+=['curriculum/guides/interrupt-lease-board-protocol.md','fixtures/embedded/interrupt_lease_capture.json','tools/check_interrupt_lease_capture.py']
 mapping=[]
 for name in PROJECTS:
  d=read(ROOT/project(name));old=original(name);assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
  put(HERE/('attachment_'+name+'.json'),dict(project=name,domain='T29',key='interrupt_lease_successor',supplied_artifacts=public,shared_contracts=['labs/interrupt_depth/lease/CONTRACTS.md','curriculum/guides/interrupt-lease-board-protocol.md'],source_register='research/v5/expansion/legacy_advanced_lease/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Original ordinary and unchanged delayed cancellation/visibility/reset/target tasks, actual author references; physical/external/learner dimensions remain separate'))
  x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/project(name),x)
  mapping.extend(dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/interrupt_depth/lease/CONTRACTS.md',learner_artifacts=['lease.c','bridge.c','order.c','target.c','criterion-indexed ownership/visibility/reset/target dossier'],assessment='assessor/interrupt_depth/lease/ANALYSIS.md#'+c['objective'][4:],checks=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_advanced_lease/resources.json') for c in d['criteria'])
 put(HERE/'objective_mapping.json',dict(criteria=mapping,remaining='T29.09–.10 remain required; physical/source/external dimensions are separate.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'source_access.json',HERE/'audit.json']},limitations=['Named primary passages only; no final ISO wording or physical target result inferred.','QEMU time is not physical latency; selected board port closures are not complete boot images.']))
 print('PASS six original criteria preserved; additive attachments idempotent')
def package():
 from check_interrupt_lease_depth import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_interrupt_lease_package.py',ROOT/'research/v5/expansion/legacy_advanced/author_support.py',*[ROOT/project(p) for p in PROJECTS]}
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
    artifacts=['labs/interrupt_depth/lease/CONTRACTS.md','assessor/interrupt_depth/lease/ANALYSIS.md','research/v5/expansion/legacy_advanced_lease/objective_mapping.json'];dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=name,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/interrupt_depth/lease/ANALYSIS.md#'+key),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Physical qualification/capture, exact published ISO access, independent learners/delayed and external review remain separate.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T29-04 six ordinary and six unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T29.09–.10 remain required authoring work.','Physical board qualification/capture, source gaps, external review and independent/delayed learner evidence remain separate.'],summary=dict(criteria=6,delayed=6,checks=len(report['checks']))));print('PASS12 exact lease author dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
