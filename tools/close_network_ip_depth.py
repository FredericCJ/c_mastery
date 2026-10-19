"""Add exact T41.05 successors without rewriting original assessment objects."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_network_scope.author_support import attach
from research.v5.expansion.legacy_network_driver.integration import apply as apply_driver
HERE=ROOT/'research/v5/expansion/legacy_network_ip';SUPPORT='research/v5/expansion/legacy_network_ip/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_network_ip/';PROJECTS=['PR-T41-05'];REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={
'05.a':['gcc-debug validation execute','gcc-debug independent oracle','gcc-debug mutant1 execute','gcc-debug mutant4 execute','gcc-debug mutant5 execute','gcc-debug mutant6 execute','gcc-debug mutant7 execute','gcc-debug mutant8 execute','clang-O3 execute'],
'05.b':['gcc-debug core execute','gcc-debug checksum-mutant execute','gcc-debug composition execute','target checksum execute'],
'05.c':['gcc-debug composition execute','gcc-debug mutant3 execute','clang-release validation execute','unobserved capture rejection']}
RATIONALE={
'05.a':'Checked IPv4 field/extent matrix, full coverage and identity, strict/identical overlaps, absolute expiry and actual timeout reports; independent22751event oracle and target tests.',
'05.b':'Actual unchanged SChecksum body with original source identity,33411allFFsplits,131076-byte accumulation bound, pseudoheader composition and compiled odd-padding defect.',
'05.c':'All65Echo data lengths,520unaligned inputs, actual original ARP/IP/Echo composition and reassembly, preserved bytes/padding and separately required physical56/64-byte protocol.'}



def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def project(p):return 'curriculum/v5/expansion/legacy_assessments/'+p+'.json'
def original(p):return json.loads(subprocess.check_output(['git','show','HEAD:'+project(p)],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/network_followthrough/ip').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public.extend(p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/network_followthrough/congestion').rglob('*')) if p.is_file() and '__pycache__' not in p.parts)
 public.append('labs/network_followthrough/src/congestion.c')
 public+=['labs/communications/include/com_net.h','labs/communications/include/com_http.h','labs/b045_systems/include/systems.h','labs/network_followthrough/include/systems.h','labs/network_followthrough/src/checksum.c','labs/network_followthrough/src/ip.c','labs/network_followthrough/CMakeLists.txt','labs/network_followthrough/family.json','curriculum/guides/network-ip-board-protocol.md','fixtures/networking/ip_physical_capture.json','tools/check_network_ip_capture.py']
 mapping=[]
 for name in PROJECTS:
  d=read(ROOT/project(name));old=original(name);assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
  put(HERE/('attachment_'+name+'.json'),dict(project=name,domain='T41',key='network_ip_successor',supplied_artifacts=public,shared_contracts=['labs/network_followthrough/ip/CONTRACTS.md','curriculum/guides/network-ip-board-protocol.md'],source_register='research/v5/expansion/legacy_network_ip/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Original scope/length/lifetime/stream and changed-condition tasks; actual author references; physical/external/learner dimensions separate'))
  x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/project(name),x)
  mapping.extend(dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/network_followthrough/ip/CONTRACTS.md',learner_artifacts=['ip.c','copy-port adapter','target driver and criterion-indexed field/parity/reassembly/ownership dossier'],assessment='assessor/network_followthrough/ip/ANALYSIS.md#'+c['objective'][4:].replace('.',''),checks=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_network_ip/resources.json') for c in d['criteria'])
 put(HERE/'objective_mapping.json',dict(criteria=mapping,remaining='T41–49 and recursive inventory remain required; physical/source/external dimensions are separate.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'source_access.json',HERE/'audit.json']},limitations=['Named primary passages only; no final ISO wording or physical target result inferred.','Actual MPS2 packet arrays are not physical Ethernet observations; actual T39 driver binding remains separately required.']))
 print('PASS three original criteria preserved; additive attachments idempotent')
def package():
 from check_network_ip_depth import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_network_ip_package.py',ROOT/'research/v5/expansion/legacy_network_scope/author_support.py',*[ROOT/project(p) for p in PROJECTS]}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name not in {'completion_dispositions.json','exercise_completion_dispositions.json'});rows=[]
 for resource in read(HERE/'resources.json')['resources']:
  for path,digest in resource['inspection']['source_sha256'].items():assert sha(ROOT/path)==digest,('inspected source changed',path)
 for name in PROJECTS:
  d=read(ROOT/project(name));assert d['criteria']==original(name)['criteria']
  for c in d['criteria']:
   key=c['objective'][4:];v=c['variants'][0];labels=list(dict.fromkeys(['gcc-debug core execute','clang-release core execute',*SELECTORS[key]]))
   for delayed in [False,True]:
    artifacts=['labs/network_followthrough/ip/CONTRACTS.md','assessor/network_followthrough/ip/ANALYSIS.md','research/v5/expansion/legacy_network_ip/objective_mapping.json'];dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','partial',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=name,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/network_followthrough/ip/ANALYSIS.md#'+key.replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=['NW-IP-DRIVER'],evidence_limits=report['limits']+['Physical qualification/capture, exact published ISO access, independent learners/delayed and external review remain separate.']))
 apply_driver(rows,files,'NW-IP-DRIVER')
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T41-05 three ordinary and three unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['Actual driver integration is verified by the linked successor; physical observations remain separate.','T41–49 and recursive inventory remain required authoring work.','Physical board/clock/PHY qualification, source gaps, external review and independent/delayed learner evidence remain separate.'],summary=dict(criteria=3,delayed=3,checks=len(report['checks']))));print('PASS6 exact IPv4 criterion dispositions with verified actual driver integration')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
