"""Add exact T41.08 successors without rewriting original assessment objects."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_network_scope.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_network_http';SUPPORT='research/v5/expansion/legacy_network_http/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_network_http/';PROJECTS=['PR-T41-08'];REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={
'08.a':['gcc-debug 0 execute','gcc-debug 1 execute','clang-release 0 execute','arm-none-eabi-gcc-O0 execute'],
'08.b':['gcc-debug 0 execute','gcc-debug 2 execute','clang-release 0 execute'],
'08.c':['gcc-debug 0 execute','clang-release 0 execute','arm-none-eabi-gcc-O3 resources']}
RATIONALE={
'08.a':'Actual raw-peer sequence reconstruction and independent literal HTTP byte expectations across32 binary/chunk/interim/EOF/window cases; compiling incomplete-EOF mutant rejected.',
'08.b':'Actual callback present/absent Date paths, no monotonic-to-UTC substitution and compiling stale-literal mutant rejected.',
'08.c':'Actual lower copied-response ownership at link loss, positive quiescence and new handshake with old ACK controls; physical capture and delayed observations remain separate.'}


def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def project(p):return 'curriculum/v5/expansion/legacy_assessments/'+p+'.json'
def original(p):return json.loads(subprocess.check_output(['git','show','HEAD:'+project(p)],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/network_followthrough/http').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public.extend(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/network_followthrough/driver').rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 public+=['labs/hosted_followthrough/http/flow.h','curriculum/guides/network-tcp-board-protocol.md','fixtures/networking/tcp_physical_capture.json','tools/check_network_tcp_capture.py','labs/network_followthrough/tcp/tcp.h','labs/network_followthrough/tcp/test_wire.h','labs/network_followthrough/arp/arp.h','labs/network_followthrough/ip/ip.h','labs/network_followthrough/ip/checksum_slice.py','labs/network_followthrough/scope/scope.h','labs/network_followthrough/scope/stream.h','labs/communications/include/com_http.h','labs/communications/include/com_net.h','labs/b045_systems/include/systems.h','labs/bringup_depth/bringup.h','labs/ethernet_depth/include/ethernet_depth.h']
 for base in ['labs/ethernet_followthrough/phy','labs/ethernet_followthrough/ring']:
  public.extend(p.relative_to(ROOT).as_posix() for p in (ROOT/base).iterdir() if p.suffix in ['.h','.md'] or p.name in ['model.c','variants_tests.c','tests.c'])
 public=list(dict.fromkeys(public))
 mapping=[]
 for name in PROJECTS:
  d=read(ROOT/project(name));old=original(name);assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
  put(HERE/('attachment_'+name+'.json'),dict(project=name,domain='T41',key='network_http_successor',supplied_artifacts=public,shared_contracts=['labs/network_followthrough/http/CONTRACTS.md','labs/network_followthrough/driver/CONTRACTS.md','labs/ethernet_followthrough/ring/BOARD_CAPTURE.md','curriculum/guides/network-tcp-board-protocol.md'],source_register='research/v5/expansion/legacy_network_http/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Actual binary/chunk/Date/link-recovery HTTP composition and unchanged original variants; physical/external/learner dimensions separate'))
  x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/project(name),x)
  mapping.extend(dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/network_followthrough/http/CONTRACTS.md',learner_artifacts=['http.c','criterion-indexed framing/clock/ownership/target dossier'],assessment='assessor/network_followthrough/http/ANALYSIS.md#'+c['objective'][4:].replace('.',''),checks=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_network_http/resources.json') for c in d['criteria'])
 put(HERE/'objective_mapping.json',dict(criteria=mapping,remaining='T41–49 and recursive inventory remain required; physical/source/external dimensions are separate.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md']},limitations=['Named primary passages only; no final ISO wording or physical target result inferred.','Actual T39 driver binding executed against author references; physical observations and actual learner substitution remain separate.']))
 print('PASS three original criteria preserved; additive attachments idempotent')
def package():
 from check_network_http import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_network_http_package.py',ROOT/'research/v5/expansion/legacy_network_scope/author_support.py',*[ROOT/project(p) for p in PROJECTS]}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name not in {'completion_dispositions.json','exercise_completion_dispositions.json'});rows=[]
 for resource in read(HERE/'resources.json')['resources']:
  for path,digest in resource['inspection'].get('source_sha256',{}).items():assert sha(ROOT/path)==digest,('inspected source changed',path)
 for name in PROJECTS:
  d=read(ROOT/project(name));assert d['criteria']==original(name)['criteria']
  for c in d['criteria']:
   key=c['objective'][4:];v=c['variants'][0];labels=list(dict.fromkeys(['gcc-debug 0 execute','clang-release 0 execute',*SELECTORS[key]]))
   for delayed in [False,True]:
    artifacts=['labs/network_followthrough/http/CONTRACTS.md','assessor/network_followthrough/http/ANALYSIS.md','research/v5/expansion/legacy_network_http/objective_mapping.json'];dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=name,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/network_followthrough/http/ANALYSIS.md#'+key.replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Physical qualification/capture, exact published ISO access, independent learners/delayed and external review remain separate.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T41-08 three ordinary and three unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T41–49 and recursive inventory remain required authoring work.','Physical board/clock/PHY qualification, source gaps, external review and independent/delayed learner evidence remain separate.'],summary=dict(criteria=3,delayed=3,checks=len(report['checks']))));print('PASS6 exact HTTP dispositions; external evidence remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
