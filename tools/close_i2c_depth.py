"""Attach and bind exact original i2c criteria; keep external evidence separate."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_advanced.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_advanced_i2c';PROJECT='curriculum/v5/expansion/legacy_assessments/PR-T28-04.json';SUPPORT='research/v5/expansion/legacy_advanced_i2c/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_advanced_i2c/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json','cost_analysis.json']]
RATIONALE={'08.a':'Derive ordinary address/repeated-START and changed two-byte ACK/POS/BTF ordering from inspected register-effect passages; both wrong-order controls rejected.', '08.b':'Address/data NACK, ARLO, stretching and quarantined late-START cleanup; exclusive nine-clock recovery and second-master refusal with actual provider comparison.', '08.c':'Original device lost-acknowledgement histories distinguish immutable reads from possibly applied increments; replay counterexample and explicit uncertainty.'}

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+PROJECT],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/PROJECT);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/peripheral_depth/i2c').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public+=['labs/peripheral_depth/uart/'+s for s in ['uart.h','uart.c','CONTRACTS.md']]+['labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/serial_depth/CONTRACTS.md','labs/firmware_io/include/firmware_io.h','labs/firmware_io/CONTRACTS.md']
 put(HERE/'attachment_PR-T28-04.json',dict(project=d['project'],domain='T28',key='i2c_sequence_successor',supplied_artifacts=public,shared_contracts=['labs/peripheral_depth/i2c/CONTRACTS.md','labs/peripheral_depth/uart/CONTRACTS.md','labs/serial_depth/CONTRACTS.md','labs/firmware_io/CONTRACTS.md'],source_register='research/v5/expansion/legacy_advanced_i2c/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria'] if c['objective'].startswith('T28.08')],scope='Three T28.08 ordinary and three unchanged delayed tasks; actual pu_rates/sd_i2c_begin/fw_i2c_recover providers; physical, independent and remaining T28 scope separate'))
 x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/PROJECT,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/peripheral_depth/i2c/CONTRACTS.md',learner_artifacts=['i2c.c','target.c','criterion-indexed dossier','recovery/uncertain-effect record'],assessment='assessor/peripheral_depth/i2c/ANALYSIS.md#'+c['objective'][4:],source_register='research/v5/expansion/legacy_advanced_i2c/resources.json') for c in d['criteria'] if c['objective'].startswith('T28.08')],remaining='T28.09–.12 and T29 remain required; physical/source/external dimensions separate'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',scope='Exact T28 i2c/capture passages, not external review',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'source_access.json',HERE/'audit.md']},limitations=['Published ISO and physical evidence remain separate']))
 print('PASS original six original criterion objects; only three I2C criteria attached preserved; additive attachment idempotent')
def package():
 from check_i2c_depth import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_i2c_package.py',ROOT/PROJECT,ROOT/'research/v5/expansion/legacy_advanced/author_support.py'}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json');rows=[];d=read(ROOT/PROJECT);assert d['criteria']==original()['criteria']
 for c in [v for v in d['criteria'] if v['objective'].startswith('T28.08')]:
  key=c['objective'][4:];v=c['variants'][0];mutants={'08.a':[1,2],'08.b':[3,4,6],'08.c':[5]}[key]
  labels=['gcc-debug execute 0','gcc-release execute 0','clang-release execute 0','gcc-debug independent oracle',*['gcc-debug execute '+str(n) for n in mutants]]
  if key=='08.a':labels+=['gcc-debug target execute 0','arm-none-eabi-gcc-O3 combined audit link']
  if key=='08.b':labels+=['gcc-debug composition execute','gcc-debug target execute 1','gcc-debug target execute 2','arm-none-eabi-gcc-O3 combined audit link','clang-O3 target disassembly']
  for delayed in [False,True]:
   artifacts=['labs/peripheral_depth/i2c/CONTRACTS.md','assessor/peripheral_depth/i2c/ANALYSIS.md','research/v5/expansion/legacy_advanced_i2c/objective_mapping.json','labs/peripheral_depth/i2c/BOARD_CAPTURE.md']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/peripheral_depth/i2c/ANALYSIS.md#'+key),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Published ISO source access remains separately registered.','Actual ISR/board integration must satisfy T29 and capture prerequisites; no physical I2C waveform, electrical recovery or slave response inferred.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T28-04 three ordinary and three unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T28.09–.12 and T29 remain required authoring/implementation work.','Physical board, published ISO, independent learners/delayed and external review remain separate.'],summary=dict(criteria=3,delayed=3,checks=len(report['checks']))));print('PASS six exact i2c author dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
