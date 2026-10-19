"""Add exact T41.02/03 successors without rewriting original assessment objects."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_network_scope.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_network_driver';SUPPORT='research/v5/expansion/legacy_network_driver/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_network_driver/';PROJECTS=['PR-T41-02','PR-T41-03'];REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={
'02.a':['arm-none-eabi-gcc-O0 execute','arm-none-eabi-gcc-O3 selected MMIO audit'],
'02.b':['gcc-debug 0 execute','gcc-debug 3 execute','clang-O3 execute'],
'02.c':['gcc-debug changed contracts execute','clang-O3 execute'],
'03.a':['gcc-debug changed contracts execute','CCM link rejected','clang-O3 resources'],
'03.b':['gcc-debug 0 execute','gcc-debug 1 execute','gcc-debug 2 execute','gcc-debug 3 execute'],
'03.c':['gcc-debug changed contracts execute','gcc-debug 4 execute','gcc-debug 5 execute','gcc-debug 6 execute']}
RATIONALE={
'02.a':'Selected F439 actual ports and revision-specific inspected source/bench analysis; physical observations remain separate.',
'02.b':'Actual PHY/MDIO/ring bring-up bounds and logical abort versus positive device-stop ownership; queued transport incarnation isolated.',
'02.c':'Actual10/full versus100/full mode fixtures and explicit register-model/physical-clock distinction.',
'03.a':'Actual12384-byte SRAM placement and rejected CCM; emitted stack closure; actual two-line ownership/invalidation changed model.',
'03.b':'Actual copied descriptor pressure, ACK/DMA ordering, retained publication failure and held-RX wrap/restart semantics.',
'03.c':'Actual normalized frame lengths, changed CSTF capture adapter, malformed upper input release, suspended/fatal and expiry ordering.'}



def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def project(p):return 'curriculum/v5/expansion/legacy_assessments/'+p+'.json'
def original(p):return json.loads(subprocess.check_output(['git','show','HEAD:'+project(p)],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/network_followthrough/driver').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public+=['labs/network_followthrough/tcp/tcp.h','labs/network_followthrough/tcp/test_wire.h','labs/network_followthrough/arp/arp.h','labs/network_followthrough/ip/ip.h','labs/network_followthrough/ip/checksum_slice.py','labs/network_followthrough/scope/scope.h','labs/network_followthrough/scope/stream.h','labs/communications/include/com_http.h','labs/communications/include/com_net.h','labs/b045_systems/include/systems.h','labs/bringup_depth/bringup.h','labs/ethernet_depth/include/ethernet_depth.h']
 for base in ['labs/ethernet_followthrough/phy','labs/ethernet_followthrough/ring']:
  public.extend(p.relative_to(ROOT).as_posix() for p in (ROOT/base).iterdir() if p.suffix in ['.h','.md'] or p.name in ['model.c','variants_tests.c','tests.c'])
 public=list(dict.fromkeys(public))
 mapping=[]
 for name in PROJECTS:
  d=read(ROOT/project(name));old=original(name);assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
  put(HERE/('attachment_'+name+'.json'),dict(project=name,domain='T41',key='network_driver_successor',supplied_artifacts=public,shared_contracts=['labs/network_followthrough/driver/CONTRACTS.md','labs/ethernet_followthrough/ring/BOARD_CAPTURE.md'],source_register='research/v5/expansion/legacy_network_driver/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Actual selected PHY/ring/T41 ownership composition and unchanged original variants; physical/external/learner dimensions separate'))
  x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/project(name),x)
  mapping.extend(dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/network_followthrough/driver/CONTRACTS.md',learner_artifacts=['driver.c','board.c','criterion-indexed ownership/source/target dossier'],assessment='assessor/network_followthrough/driver/ANALYSIS.md#'+c['objective'][4:].replace('.',''),checks=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_network_driver/resources.json') for c in d['criteria'])
 put(HERE/'objective_mapping.json',dict(criteria=mapping,remaining='T41–49 and recursive inventory remain required; physical/source/external dimensions are separate.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md']},limitations=['Named primary passages only; no final ISO wording or physical target result inferred.','Actual T39 driver binding executed against author references; physical observations and actual learner substitution remain separate.']))
 print('PASS six original criteria preserved; additive attachments idempotent')
def package():
 from check_network_driver import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_network_driver_package.py',ROOT/'research/v5/expansion/legacy_network_scope/author_support.py',*[ROOT/project(p) for p in PROJECTS]}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 # Child reconciliation consumes this producer. Neither its generated output
 # nor its consumer bridge is an input to the driver's actual implementation.
 # Including them here creates producer -> consumer -> producer hash cycles.
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name not in {'completion_dispositions.json','exercise_completion_dispositions.json','integration_completion_dispositions.json','integration.py'});rows=[]
 for resource in read(HERE/'resources.json')['resources']:
  capture=resource['inspection'].get('capture');
  if capture:assert sha(ROOT/capture['path'])==capture['sha256'],('inspected source changed',capture['path'])
 for name in PROJECTS:
  d=read(ROOT/project(name));assert d['criteria']==original(name)['criteria']
  for c in d['criteria']:
   key=c['objective'][4:];v=c['variants'][0];labels=list(dict.fromkeys(['gcc-debug 0 execute','clang-release 0 execute',*SELECTORS[key]]))
   for delayed in [False,True]:
    artifacts=['labs/network_followthrough/driver/CONTRACTS.md','assessor/network_followthrough/driver/ANALYSIS.md','research/v5/expansion/legacy_network_driver/objective_mapping.json'];dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=name,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/network_followthrough/driver/ANALYSIS.md#'+key.replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Physical qualification/capture, exact published ISO access, independent learners/delayed and external review remain separate.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T41-02/03 six ordinary and six unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T41–49 and recursive inventory remain required authoring work.','Physical board/clock/PHY qualification, source gaps, external review and independent/delayed learner evidence remain separate.'],summary=dict(criteria=6,delayed=6,checks=len(report['checks']))));print('PASS12 exact driver dispositions; external evidence remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
