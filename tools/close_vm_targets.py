"""Add exact T46.11 successors without rewriting original assessment objects."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_vm_representation.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_vm_targets';SUPPORT='research/v5/expansion/legacy_vm_targets/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_vm_targets/';PROJECTS=['PR-T46-11'];REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={
'11.a':['gcc-debug adapter1 compare','PPC64-O0 adapter1 ELF','PPC64-O0 adapter1 compare','PPC64-O3 adapter1 compare','exact fixture bytes'],
'11.b':['arm-none-eabi-gcc-O0 adapter1 compare','kernel frozen guard','kernel gcc-O0 compare','kernel gcc-O0 bytes','kernel clang-O2 compare','gcc-debug adapter2 compare','arm-none-eabi-gcc-O0 adapter2 compare'],
'11.c':['arm-none-eabi-gcc-O0 adapter1 resources','clang-O3 adapter1 resources','skip bss execute','implicit helper rejection','unrooted LTO link rejection','arm-none-eabi-gcc-O3-LTO adapter1 resources']}
RATIONALE={
'11.a':'Actual original VM across Linux and BE ELFv2/M4, independent exact19-byte encoding and native-layout distinction; source compiler excluded.',
'11.b':'Actual six-row cost/status comparison across Linux, M4 and pinned own-kernel executions; explicit plus2 callback successor changes results with same core/image.',
'11.c':'Nonempty data and poisoned BSS startup, actual vectors/map/object/final closure and conservative generated frames; division-import rejection and real GCC LTO helper retention/failure controls.'}


def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def project(p):return 'curriculum/v5/expansion/legacy_assessments/'+p+'.json'
def original(p):return json.loads(subprocess.check_output(['git','show','HEAD:'+project(p)],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/runtime_followthrough/targets').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public+=['labs/runtime/include/cvm.h','labs/runtime/SPEC.md','labs/runtime/src/cvm.c','labs/kernel_followthrough/runtime/probe.h','labs/kernel_followthrough/runtime/probe.c','labs/kernel_followthrough/runtime/fixture.h','labs/smp_depth/acpi/big_endian.S']
 mapping=[]
 for name in PROJECTS:
  d=read(ROOT/project(name));old=original(name);assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
  put(HERE/('attachment_'+name+'.json'),dict(project=name,domain='T46',key='vm_targets_successor',supplied_artifacts=public,shared_contracts=['labs/runtime_followthrough/targets/CONTRACTS.md','labs/runtime/SPEC.md'],source_register='research/v5/expansion/legacy_vm_targets/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Actual image/ABI/kernel/adapter/startup/LTO variants; physical/external/learner dimensions separate'))
  x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/project(name),x)
  mapping.extend(dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/runtime_followthrough/targets/CONTRACTS.md',learner_artifacts=['original learner CVM','target adapters and generated images','criterion-indexed byte/cost/closure dossier'],assessment='assessor/runtime_followthrough/targets/ANALYSIS.md#'+c['objective'][4:].replace('.',''),checks=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_vm_targets/resources.json') for c in d['criteria'])
 put(HERE/'objective_mapping.json',dict(criteria=mapping,remaining='T46–49 and recursive inventory remain required; physical/source/external dimensions are separate.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md']},limitations=['Named primary passages only; no final ISO wording or physical target result inferred.','Actual CVM1 plus selected M4 ROM256KiB/RAM64KiB/32KiB stack reserve; kernel and BE profiles separate.']))
 print('PASS three original criteria preserved; additive attachments idempotent')
def package():
 from check_vm_targets import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_vm_targets_package.py',ROOT/'research/v5/expansion/legacy_vm_representation/author_support.py',*[ROOT/project(p) for p in PROJECTS]}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name not in {'completion_dispositions.json','exercise_completion_dispositions.json'});rows=[]
 for resource in read(HERE/'resources.json')['resources']:
  for path,digest in resource['inspection'].get('source_sha256',{}).items():assert sha(ROOT/path)==digest,('inspected source changed',path)
 for name in PROJECTS:
  d=read(ROOT/project(name));assert d['criteria']==original(name)['criteria']
  for c in d['criteria']:
   key=c['objective'][4:];v=c['variants'][0];labels=list(dict.fromkeys(SELECTORS[key]))
   for delayed in [False,True]:
    artifacts=['labs/runtime_followthrough/targets/CONTRACTS.md','assessor/runtime_followthrough/targets/ANALYSIS.md','assessor/runtime_followthrough/targets/DELAYED.md','research/v5/expansion/legacy_vm_targets/objective_mapping.json'];dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=name,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/runtime_followthrough/targets/ANALYSIS.md#'+key.replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Physical target observations, exact published ISO access, independent learners/delayed and external review remain separate.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T46-11 three ordinary and three unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T46–49 and recursive inventory remain required authoring work.','Physical target observations, source gaps, external review and independent/delayed learner evidence remain separate.'],summary=dict(criteria=3,delayed=3,checks=len(report['checks']))));print('PASS6 exact VM target dispositions; external evidence remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
