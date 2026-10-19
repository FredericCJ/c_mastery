"""Preserve original PR-T38-09 objects and attach exact guarded author evidence."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_can';PROJECT='PR-T38-09';REPORT='research/v5/expansion/legacy_serial_can/validation.json';SUPPORT='research/v5/expansion/legacy_serial_can/completion_dispositions.json'
from research.v5.expansion.legacy_serial_can.author_support import attach
ARTIFACTS=['labs/serial_followthrough/can/CONTRACTS.md','labs/serial_followthrough/can/BOARD_CAPTURE.md','assessor/serial_followthrough/can/ANALYSIS.md',*[f'assessor/serial_followthrough/can/{n}.c' for n in ['codec','fault','controller','port']],'labs/serial_followthrough/can/tests.c','labs/serial_followthrough/can/controller_tests.c','labs/serial_followthrough/can/guest.c','assessor/serial_followthrough/can/oracle.py']
REMAINING=['Physical two-node selected-board bus/clock/pin/transceiver evidence, including real ACK/arbitration/bus-off and recovery captures.','PR-T38-10 FD/current-preISO/controller work, remaining T38/T39/T40 and mixed LRUN parent obligations remain required.','Independent original/delayed learner work and external review have not been performed.','The scoped classified-event FCE and local strict codec are not a full physical/conforming CAN MAC implementation.']
PATTERNS={'T38.10.a':['.arbitration','.mutation.NUMERIC'],'T38.10.b':['.codec','.stream','.mutation.CRC','.mutation.STREAM'],'T38.10.c':['.fault','.mutation.ARB_COUNTER','.mutation.EXCEPTION','.mutation.REC_RESET'],'T38.11.a':['.timing','.mutation.PROPAGATION'],'T38.11.b':['.fifo','.port','.mutation.W1C'],'T38.11.c':['.mailbox','.recovery','.port','.mutation.EARLY_ARB','.mutation.ABORT_LEASE']}
WHY={'T38.10.a':'Actual retained arbitration producer, bit-prefix/numeric-ID counterexamples and a rejected numeric-order mutant.','T38.10.b':'Actual incremental decoder with independent CRC/fixed-form faults, all three-chunk splits and cross-boundary stuffing state.','T38.10.c':'Actually inspected current complete counter clause; classified-event implementation exercises all rules/exceptions and bounded restart history.','T38.11.a':'Exact encoded fields and independent changed-clock/sample/propagation search, actual target arithmetic closure.','T38.11.b':'Actual selected filter/FIFO0 register adapter, exactly-once copy/release, explicit sticky-W1C acknowledgement and changed client queue policy.','T38.11.c':'Actual selected TX mailbox lease/token adapter, auto-retry/NART, cancel-success race, retained bus-off ownership and bounded observed recovery.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def all_rows(r):
 rows=[]
 for c in read(canon())['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:2]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   checks=[q['id'] for q in r['commands'] if any(x in q['id'] for x in PATTERNS[c['objective']]) or q['id'] in [cc+'.O'+o+'.execute' for cc in ['arm-none-eabi-gcc','clang'] for o in ['0','3']]]
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks),closure_rationale=WHY[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 return rows
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/serial_followthrough/can').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/serial_depth/CONTRACTS.md','labs/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld']
 d=read(canon());old=HERE/(PROJECT+'.before-can.json')
 if not old.exists():old.write_bytes(canon().read_bytes())
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T38',supplied_artifacts=public,shared_contracts=ARTIFACTS[:2],source_register='research/v5/expansion/legacy_serial_can/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Six unchanged criterion/V1 pairs; actual codec/FCE and selected CAN1 register adapter; physical/independent gates separate.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive CAN attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_serial_can.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 d=read(canon());old=read(HERE/(PROJECT+'.before-can.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon())
 r,e=reports;rows=all_rows(r);assert len(rows)==12
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T38-09 six original criterion/V1 pairs, six evidence dimensions',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_serial_can/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS12 exact CAN dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
