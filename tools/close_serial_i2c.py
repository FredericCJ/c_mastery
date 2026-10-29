"""Preserve originals and bind the six exact I2C criterion/V1 pairs."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_i2c';PROJECTS=['PR-T38-03','PR-T38-04'];REPORT='research/v5/expansion/legacy_serial_i2c/validation.json';SUPPORT='research/v5/expansion/legacy_serial_i2c/completion_dispositions.json'
from research.v5.expansion.legacy_serial_i2c.author_support import attach
ARTIFACTS=['labs/serial_followthrough/i2c/CONTRACTS.md','labs/serial_followthrough/i2c/BOARD_CAPTURE.md','assessor/serial_followthrough/i2c/ANALYSIS.md','assessor/serial_followthrough/i2c/i2c_depth.c','assessor/serial_followthrough/i2c/clear.c','labs/serial_followthrough/i2c/transaction.c','labs/serial_followthrough/i2c/mapped.c','assessor/serial_followthrough/i2c/oracle.py','assessor/serial_followthrough/i2c/capture_investigation.py','assessor/serial_followthrough/i2c/provider_mutant.py']
REMAINING=['Physical analog timing, voltage, calibrated captures and dedicated board/peer ownership remain required.','Other T38 projects, T39–40 and mixed LRUN parent scope remain separately required.','External review and independent original/seven-day changed learner attempts remain unperformed.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon(project):return ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json')
def all_rows(r):
 patterns={'T38.04.a':['.electrical','.mutation.ROUND','.mutation.LEAK','.closure','.stack'],'T38.04.b':['.arbitration','.mutation.PHASE','.mutation.LOSER'],'T38.04.c':['capture','.electrical.oracle'],'T38.05.a':['.transaction','.mapped','.mutation.ACK','.mutation.BTF','.closure'],'T38.05.b':['.failure','.late_publication','.mutation.ARLO','.mutation.LATE'],'T38.05.c':['.recovery','.mutation.IDLE','.mapped']}
 why={'T38.04.a':'Integer nominal bounds plus conservative leakage/tolerance guarantee with64-step checked mechanism,150 envelopes,10000 capacitances and rational oracle.','T38.04.b':'65536 actual eight-bit pair traces, identical survivors, first-difference loss, initial clock synchronization and unsupported START/STOP phase counterexample.','T38.04.c':'Executed original synthetic analog admission with decoded-only mutant, capacitance/mode transfer and explicit missing physical dossier.','T38.05.a':'Actual frozen Pi one/two-byte controller composition; ACK/POS/BTF/STOP side effects plus actual F439 mapped adapter and Thumb execution.','T38.05.b':'Distinct NACK/ARLO/timeout cleanup and exact late-token publication integrity; compiled provider-copy mutant demonstrates harmless wrong-request publication.','T38.05.c':'Actual frozen struct pi_recovery with idle/no-lease adapter,1..9 release cases, held SCL, modular-time wrap and explicit competing-controller impossibility argument.'}
 rows=[]
 for project in PROJECTS:
  for c in read(canon(project))['criteria']:
   v=c['variants'][0]
   for delayed in [False,True]:
    dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:2]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=[q['id'] for q in r['commands'] if any(x in q['id'] for x in patterns[c['objective']])]),closure_rationale=why[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 return rows
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/serial_followthrough/i2c').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/peripheral_depth/i2c/i2c.h','labs/peripheral_depth/i2c/i2c.c','labs/peripheral_depth/i2c/target.c','labs/peripheral_depth/i2c/CONTRACTS.md','labs/peripheral_depth/uart/uart.h','labs/peripheral_depth/uart/uart.c','labs/peripheral_depth/uart/CONTRACTS.md','labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld']
 for project in PROJECTS:
  d=read(canon(project));old=HERE/(project+'.before-i2c.json')
  if not old.exists():old.write_bytes(canon(project).read_bytes())
  write(HERE/('attachment_'+project+'.json'),dict(project=project,domain='T38',supplied_artifacts=public,shared_contracts=ARTIFACTS[:2]+['labs/peripheral_depth/i2c/CONTRACTS.md'],source_register='research/v5/expansion/legacy_serial_i2c/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three exact original criterion/V1 pairs; author experiments do not discharge physical or independent learner gates.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(project),after)
 print('PASS additive I2C attachments')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_serial_i2c.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for k in ['source_sha256','outputs_sha256']:
   for n,h in r[k].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 for p in PROJECTS:
  d=read(canon(p));old=read(HERE/(p+'.before-i2c.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(p))
 r,e=reports;rows=all_rows(r);assert len(rows)==12
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T38-03/04 all original criterion/V1 pairs, six dimensions',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_serial_i2c/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS12 exact I2C dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
