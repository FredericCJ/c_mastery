"""Exact unchanged FD criterion/V1 objects with additive current author evidence."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_fd';PROJECT='PR-T38-10';REPORT='research/v5/expansion/legacy_serial_fd/validation.json';SUPPORT='research/v5/expansion/legacy_serial_fd/completion_dispositions.json'
from research.v5.expansion.legacy_serial_fd.author_support import attach
ARTIFACTS=['labs/serial_followthrough/fd/CONTRACTS.md','assessor/serial_followthrough/fd/ANALYSIS.md','assessor/serial_followthrough/fd/fd.c','labs/serial_followthrough/fd/tests.c','labs/serial_followthrough/fd/guest.c','assessor/serial_followthrough/fd/oracle.py']
REMAINING=['Required PR-T38-11/12 actual selected-controller bring-up/compatible peer/physical captures remain separate.','Remaining T38/T39/T40 and mixed LRUN parents remain required.','Independent original/delayed learner work and external review have not been performed.','Selected synthetic transmitter and discrete TDC are not a complete MAC or physical conformance claim.']
PATTERNS={'T38.12.a':['.admission','.mutation.DLC','.mutation.CRC_WIDTH','.mutation.PADDING'],'T38.12.b':['.wire','.tdc','.mutation.STUFF_COUNT','.mutation.CRC_INIT','.mutation.DOUBLE_STUFF','.mutation.TDC_END'],'T38.12.c':['.controller','.mutation.PARTIAL_WORD']}
WHY={'T38.12.a':'Actual raw flag/capability/DLC admission, every byte-length exact/rounded zero-padding case and current ESI counterexample.','T38.12.b':'Actually inspected2012/current clauses; explicit incompatible synthetic frames, protected count/fixed stuffing, independent polynomial division and delayed saved-bit/end-phase TDC trace.','T38.12.c':'Selected actual source headers, word/byte transaction boundary and complete-word partial progress; distinct TCAN and MCP supply/logic circuits.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def all_rows(r):
 rows=[]
 for c in read(canon())['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   checks=[q['id'] for q in r['commands'] if any(x in q['id'] for x in PATTERNS[c['objective']]) or q['id'] in [cc+'.O'+o+'.execute' for cc in ['arm-none-eabi-gcc','clang'] for o in ['0','3']]]
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks),closure_rationale=WHY[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 return rows
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/serial_followthrough/fd').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/serial_depth/CONTRACTS.md','labs/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld']
 d=read(canon());old=HERE/(PROJECT+'.before-fd.json')
 if not old.exists():old.write_bytes(canon().read_bytes())
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T38',supplied_artifacts=public,shared_contracts=ARTIFACTS[:1],source_register='research/v5/expansion/legacy_serial_fd/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three unchanged criterion/V1 pairs; selected current/historical synthetic frame/TDC and controller transaction boundary; physical/independent gates separate.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive FD attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_serial_fd.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 d=read(canon());old=read(HERE/(PROJECT+'.before-fd.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon())
 r,e=reports;rows=all_rows(r);assert len(rows)==6
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T38-10 three original criterion/V1 pairs, six evidence dimensions',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_serial_fd/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact FD dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
